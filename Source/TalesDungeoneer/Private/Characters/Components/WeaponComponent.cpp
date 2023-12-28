

#include "Characters/Components/WeaponComponent.h"

#include "Components/AudioComponent.h"
#include "Weapons/WeaponSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "GameFramework/Character.h"
#include "Characters/CharacterBase.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/WeaponBase.h"


float LocalPlayAnimMontage(ACharacter* characterReference, UAnimMontage* animMontage)
{
	if (IsValid(characterReference))
	{
		USkeletalMeshComponent* skMesh = characterReference->GetMesh();
		if (IsValid(skMesh))
		{
			UAnimInstance* animInstance = skMesh->GetAnimInstance();
			if (IsValid(animInstance))
			{
				return animInstance->Montage_Play(animMontage, 1.0f);
			}
		}
	}
	return 0.f;
}

float LocalPlayAnimSound(ACharacter* CharacterReference, USoundBase* SoundBase)
{
	if (IsValid(CharacterReference) && IsValid(SoundBase))
	{
		UAudioComponent* tempAudio = NewObject<UAudioComponent>(CharacterReference);
		if (IsValid(tempAudio))
		{
			tempAudio->RegisterComponent();
			tempAudio->AutoAttachParent = CharacterReference->GetRootComponent();
			tempAudio->bAutoManageAttachment = true;
			tempAudio->SetSound(SoundBase);
			tempAudio->Activate(true);
			tempAudio->Play();
			tempAudio->bAutoDestroy = true;
		}
		return tempAudio->GetSound()->Duration;
	}
	return 0.f;
}

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	
}

void UWeaponComponent::SetToggleWeapon(EWeaponSlots WeaponSlot, bool MakeReady)
{
	if (!GetOwner()->HasAuthority()) return;
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): SetToggleWeapon()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}

	// Determine which weapon we're affecting
	const bool isPrimaryWeapon = WeaponSlot == EWeaponSlots::PRIMARY;
	AWeaponBase* selectedWeapon = isPrimaryWeapon ? PrimaryWeapon_ : SecondaryWeapon_;
	if (isPrimaryWeapon ? bPrimaryOperating : bSecondaryOperating)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): Weapon is Operating. SetToggleWeapon() Ignored."),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return;
	}
	
	if (isPrimaryWeapon)
	{
		bPrimaryOperating = true;
	}
	else
	{
		bSecondaryOperating = true;
	}
	
	if (IsValid(selectedWeapon))
	{
		if (!selectedWeapon->setWeaponIsArmed(MakeReady))
			return;
	}
	const float changeTime = WeaponReadyChanged(WeaponSlot);
	
	// Use timer
	if (changeTime > 0.f)
	{
		FTimerHandle TempTimer; // Will self destruct after firing if loop is false
		FTimerDelegate TempArgs;

		// Sets operating to false after the delay time has been reached
		// This prevents the weapons from being drawn/sheathed prematurely
		TempArgs.BindUObject(this, &UWeaponComponent::WeaponSlotReady, WeaponSlot);
		GetWorld()->GetTimerManager().SetTimer(TempTimer, TempArgs, changeTime, false);
	}
	// Otherwise just make the weapon ready
	else
		WeaponSlotReady(WeaponSlot);
	
}

void UWeaponComponent::AdjustWeaponAttachment(EWeaponSlots weaponSlot)
{
}

void UWeaponComponent::Server_ToggleWeapon_Implementation(EWeaponSlots weaponType, bool makeReady)
{
	if (!GetOwner()->HasAuthority()) return;
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Server_ToggleWeapon_Implementation()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	AWeaponBase* tempWeapon = PrimaryWeapon_;
	switch (weaponType)
	{
	case EWeaponSlots::SECONDARY:
		tempWeapon = SecondaryWeapon_;
		break;
	default:
		break;
	}
	
	if (IsValid(tempWeapon))
	{
		// Do nothing if the status of the weapon isn't changing
		if (tempWeapon->getIsWeaponArmed() != makeReady)
			SetToggleWeapon(weaponType, makeReady);
	}
}

bool UWeaponComponent::PerformAttack(EWeaponSlots weaponType)
{
	if (GetWorld()->GetTimerManager().IsTimerActive(AttackCooldown_))
	{
		return false;
	}
	
	if (IsValid(CharacterBase_))
	{

		AWeaponBase* hitWeapon = PrimaryWeapon_;
		switch (weaponType)
		{
		case EWeaponSlots::SECONDARY:
			hitWeapon = SecondaryWeapon_;
			break;
		default:
			break;
		}
		
		if (!IsValid(hitWeapon))
			return false;

		// Check if the weapon is ready for an attack action
		const ENetMode NetMode = GetNetMode();
		if (hitWeapon->getIsWeaponArmed())
		{
			Server_RequestAttack(weaponType);
		}
		else
		{
			// Whether or not it worked client side, always send a request to
			//    server for weapon toggle, and let it handle the request.
			Server_ToggleWeapon(weaponType, true);
		}
		
		return true;
	}
	
	return false;
}

void UWeaponComponent::SetIsBlocking(bool IsBlocking)
{
	if (IsBlocking)
	{
		// Make shield ready
		SetToggleWeapon(EWeaponSlots::SECONDARY, true);
	}
	// If the shield item was already equipped, block immediately.
	else
	{
		bBlocking = true;
	}
}

bool UWeaponComponent::GetIsBlocking()
{
	return bBlocking;
}

void UWeaponComponent::SetTargetedActor(AActor* targetActor)
{
	if (!GetOwner()->HasAuthority()) return;
	if (IsValid(targetActor))
		TargetActor_ = targetActor;
	else
		TargetActor_ = nullptr;
}

void UWeaponComponent::ClearTargetedActor()
{
	if (!GetOwner()->HasAuthority()) return;
	TargetActor_ = nullptr;
}

void UWeaponComponent::UnsetWeapon(EWeaponSlots weaponSlot)
{

	AWeaponBase* tempWeapon = nullptr;
	switch (weaponSlot)
	{
	case EWeaponSlots::PRIMARY:
		tempWeapon = PrimaryWeapon_;
		break;
	case EWeaponSlots::SECONDARY:
		tempWeapon = SecondaryWeapon_;
		break;
	default:
		break;
	}
	
	// If a valid weapon is in this slot, we need to handle it.
	if (IsValid(tempWeapon))
	{
		tempWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		tempWeapon->SetOwner(nullptr);
		tempWeapon->Destroy();
	}
	
}

bool UWeaponComponent::GetIsWeaponReady(EWeaponSlots WeaponSlot) const
{
	switch(WeaponSlot)
	{
	case EWeaponSlots::PRIMARY:
		if (IsValid(PrimaryWeapon_))
			return PrimaryWeapon_->getIsWeaponArmed();
		break;
	case EWeaponSlots::SECONDARY:
		if (IsValid(SecondaryWeapon_))
			return SecondaryWeapon_->getIsWeaponArmed();
		break;
	default:
		break;
	}
	return false;
}

void UWeaponComponent::SetWeapon(FName weaponName, EWeaponSlots weaponSlot)
{
	if (!GetOwner()->HasAuthority())
		return;

	if (!IsValid(CharacterBase_))
		CharacterBase_ = Cast<ACharacterBase>( GetOwner() );
	
	if (!IsValid(CharacterBase_))
		return;

	// Delete any already existing weapon
	UnsetWeapon(weaponSlot);
	
	// Add the next weapon
	const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName(weaponName);
	if (UWeaponSystem::GetWeaponIsValid(weaponData))
	{
		
	}
	
}

// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterBase_ = Cast<ACharacterBase>( GetOwner() );
	GetWorld()->GetTimerManager().ClearTimer(AttackCooldown_);
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicated via Multicast to everyone, for things like mesh sync
	DOREPLIFETIME(UWeaponComponent, TargetActor_);
	DOREPLIFETIME(UWeaponComponent, PrimaryWeapon_);
	DOREPLIFETIME(UWeaponComponent, SecondaryWeapon_);
}

void UWeaponComponent::OnComponentCreated()
{
	if (bVerboseOutput)
		bShowDebug = true;
	Super::OnComponentCreated();
	SetAutoActivate(true);
	SetIsReplicated(true);
	RegisterComponent();
}

void UWeaponComponent::WeaponSlotReady(EWeaponSlots WeaponSlot)
{
	AdjustWeaponAttachment(WeaponSlot);
	
	switch(WeaponSlot)
	{
	case EWeaponSlots::PRIMARY:
		bPrimaryOperating = false;
		break;
		
	case EWeaponSlots::SECONDARY:
		bSecondaryOperating = false;
		break;
		
	// Set both to false as an "oh crap" case
	default:
		bPrimaryOperating	= false;
		bSecondaryOperating = false;
		break;
	}
}

void UWeaponComponent::OnUnregister()
{
	if (IsValid(PrimaryWeapon_))
		PrimaryWeapon_->Destroy();
	if (IsValid(SecondaryWeapon_))
		SecondaryWeapon_->Destroy();
	Super::OnUnregister();
}

void UWeaponComponent::ResetAttackCooldown()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(AttackCooldown_))
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Clearing Cooldown Timer"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("S"):TEXT("C"));

		GetWorld()->GetTimerManager().ClearTimer(AttackCooldown_);
	}
}

float UWeaponComponent::WeaponReadyChanged(EWeaponSlots weaponSlot)
{
	AWeaponBase* hitWeapon = nullptr;
	switch(weaponSlot)
	{
	case EWeaponSlots::PRIMARY:
		if (IsValid(PrimaryWeapon_))
		{
			hitWeapon = PrimaryWeapon_;
		}
		break;
	case EWeaponSlots::SECONDARY:
		if (IsValid(SecondaryWeapon_))
		{
			hitWeapon = SecondaryWeapon_;
		}
		break;
	default:
		break;
	}
	
	if (IsValid(GetOwner()))
	{
	}
	return 0.f;
}

void UWeaponComponent::OnRep_PrimaryChanged()
{
	AdjustWeaponAttachment(EWeaponSlots::PRIMARY);
}

void UWeaponComponent::OnRep_SecondaryChanged()
{
	AdjustWeaponAttachment(EWeaponSlots::SECONDARY);
}

void UWeaponComponent::Multicast_WeaponSoundEffect_Implementation(ACharacter* characterRef, USoundBase* soundEffect)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Multicast_WeaponSoundEffect_Implementation()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	if (IsValid(characterRef))
	{
		ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (IsValid(LocalPlayer))
		{
			// Only play the sound if the instigating character is NOT this player
			// (they already played it when they instigated the action)
			if (characterRef->GetInstigatorController() != LocalPlayer->GetPlayerController(GetWorld()))
			{
				const FVector soundLocation = characterRef->GetActorLocation();
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), soundEffect, soundLocation, FRotator());
			}
		}
	}
}

void UWeaponComponent::Multicast_SendAnimation_Implementation(ACharacter* characterRef, UAnimMontage* animToPlay)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Multicast_SendAnimation_Implementation()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	
	// Dont play on the activating client, they played it when they ran the command
	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if ( IsValid(localPlayer) )
	{
		if (GetOwner()->GetInstigatorController() == localPlayer->PlayerController)
		{
			UE_LOG(LogTemp, Display, TEXT("%s(%s): Animation was self; Ignoring."),
				*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
			return;
		}
	}
	const float animLength = LocalPlayAnimMontage(characterRef, animToPlay);
	if (animLength <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to play Anim Montage '%s'"), *animToPlay->GetPathName());
	}
}

void UWeaponComponent::SendSoundEffect(ACharacter* characterRef, USoundBase* soundToPlay)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): SendSoundEffect()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	if (IsValid(characterRef))
	{
		// If client, simply play the sound
		if (GetNetMode() == NM_Client)
		{
			const FVector soundLocation = characterRef->GetActorLocation();
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), soundToPlay, soundLocation, FRotator());
		}
		// If server, send sound to everyone
		else
			Multicast_WeaponSoundEffect(characterRef, soundToPlay);
	}
}

void UWeaponComponent::DelaySoundEffect(USoundBase* soundToPlay, float soundDelay)
{
	if (soundDelay <= 0.0f)
		SendSoundEffect(CharacterBase_, soundToPlay);
	else
	{
		FTimerDelegate attackArgs;
		attackArgs.BindUObject(this, &UWeaponComponent::SendSoundEffect,
			Cast<ACharacter>(CharacterBase_), soundToPlay);
	
		FTimerHandle doSoundEffectTimer;
		GetWorld()->GetTimerManager().SetTimer(
			doSoundEffectTimer, attackArgs, soundDelay, false);
	}
}

void UWeaponComponent::Server_RequestAttack_Implementation(EWeaponSlots weaponType)
{
	// If this weapon isn't on cooldown
	if (!GetWorld()->GetTimerManager().IsTimerActive(AttackCooldown_))
	{
		// Determine the weapon being used
		AWeaponBase* weaponInUse = PrimaryWeapon_;
		bool isOperating = bPrimaryOperating;
		switch (weaponType)
		{
		case EWeaponSlots::SECONDARY:
			weaponInUse = SecondaryWeapon_;
			isOperating = bSecondaryOperating;
			break;
		default:
			break;
		}

		if (isOperating)
		{
			UE_LOG(LogTemp, Warning, TEXT("%s(%s): Server_RequestAttack - Weapon is operating!"), *GetName(), GetOwner()->HasAuthority()?TEXT("SRV"):TEXT("CLI"));
			return;
		}

		if (!weaponInUse->getIsWeaponArmed())
		{
			UE_LOG(LogTemp, Error, TEXT("%s(%s): Server_RequestAttack - Weapon not ready... Drawing!"), *GetName(), GetOwner()->HasAuthority()?TEXT("SRV"):TEXT("CLI"));
			SetToggleWeapon(weaponType, true);
			return;
		}

		if (!IsValid(weaponInUse))
		{
			if (bShowDebug)
				UE_LOG(LogTemp, Error, TEXT("%s(%s): Server_RequestAttack - No weapon in use!"), *GetName(), GetOwner()->HasAuthority()?TEXT("SRV"):TEXT("CLI"));
			return;
		}
		
		
		// Is the weapon able to attack?
		bool attackSuccess = weaponInUse->doAttack();
		if (attackSuccess)
		{
			
		}
	}
}

void UWeaponComponent::OnRep_TargetChanged_Implementation()
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): OnRep_TargetChanged_Implementation()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	FString newTgt = "-no target-";
	if (IsValid(TargetActor_)) newTgt = TargetActor_->GetName();
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Target has Changed. New Target = %s"), *GetName(), GetOwner()->HasAuthority()?TEXT("SRV"):TEXT("CLI"), *newTgt);
}
