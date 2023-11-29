

#include "WeaponComponent.h"

#include "Components/AudioComponent.h"
#include "TalesDungeoneer/Weapons/WeaponSystem.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Weapons/WeaponBase.h"

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
	AWeaponBase* selectedWeapon = isPrimaryWeapon ? _PrimaryWeapon : _SecondaryWeapon;
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
		selectedWeapon->setWeaponIsArmed(MakeReady);
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
	//if (!GetOwner()->HasAuthority()) return;
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): AdjustWeaponAttachment()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	const ACharacter* ownerActor = Cast<ACharacter>(GetOwner());
	if (!IsValid(ownerActor))
		return;

	AWeaponBase* tempWeapon = _PrimaryWeapon;
	switch (weaponSlot)
	{
	case EWeaponSlots::SECONDARY:
		tempWeapon = _SecondaryWeapon;
		break;
	default:
		break;
	}
	
	if (!IsValid(tempWeapon))
		return; // prevent invalid memory access
	
	const bool isWeaponReady = tempWeapon->getIsWeaponArmed();
	
	const FStWeaponData weaponData = tempWeapon->getWeaponData();
	const FVector locationOffset = isWeaponReady ? weaponData.ActorHeldOffset	: weaponData.ActorHolsterOffset;
	const FRotator rotateOffset	 = isWeaponReady ? weaponData.ActorHeldRotation	: weaponData.ActorHolsterRotation;
	const FName socketName		 = isWeaponReady ? weaponData.ActorHeldBone		: weaponData.ActorHolsterBone;
	
	tempWeapon->AttachToComponent(ownerActor->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale, socketName);
	tempWeapon->AddActorLocalOffset(locationOffset, false);
	tempWeapon->AddActorLocalRotation(rotateOffset, false);
}

void UWeaponComponent::Server_ToggleWeapon_Implementation(EWeaponSlots weaponType, bool makeReady)
{
	if (!GetOwner()->HasAuthority()) return;
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Server_ToggleWeapon_Implementation()"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	AWeaponBase* tempWeapon = _PrimaryWeapon;
	switch (weaponType)
	{
	case EWeaponSlots::SECONDARY:
		tempWeapon = _SecondaryWeapon;
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
	const FDateTime nowTime = FDateTime::UtcNow();
	if (_NextAttackTime > nowTime)
	{
		// Prevent DDOSing the server - Still needs to be verified server-side
		return false;
	}
	
	const ACharacterBase* ownerActor = Cast<ACharacterBase>(GetOwner());
	if (IsValid(ownerActor))
	{

		AWeaponBase* hitWeapon = _PrimaryWeapon;
		switch (weaponType)
		{
		case EWeaponSlots::SECONDARY:
			hitWeapon = _SecondaryWeapon;
			break;
		default:
			break;
		}
		
		if (!IsValid(hitWeapon))
			return false;
		
		// If the attacker is a game mode character, we need to do some more stuff
		ACharacterBase* ownerCharacter = Cast<ACharacterBase>(GetOwner());
		if (IsValid(ownerCharacter))
		{
			const EWeaponTypes animWeaponType = hitWeapon->getWeaponData().WeaponType;
			if (hitWeapon->getIsWeaponArmed())
			{
				if (AttackAnimations.Contains(animWeaponType))
				{
					UAnimMontage* animMontage = AttackAnimations[animWeaponType];
					if (IsValid(animMontage))
					{
						LocalPlayAnimMontage(ownerCharacter, animMontage);
					}
				}
			}
			else
			{
				if (DrawAnimations.Contains(animWeaponType))
				{
					UAnimMontage* animMontage = DrawAnimations[animWeaponType];
					if (IsValid(animMontage))
					{
						LocalPlayAnimMontage(ownerCharacter, animMontage);
					}
				}
				
			}
		}

		// Check if the weapon is ready for an attack action
		const FStWeaponData HitWeaponData = hitWeapon->getWeaponData();
		if (hitWeapon->getIsWeaponArmed())
		{
			// Let the attacker animate
			if (hitWeapon->doAttack())
			{
				const EWeaponTypes animWeaponType = HitWeaponData.WeaponType;
				if (AttackAnimations.Contains(animWeaponType))
				{
					UAnimMontage* animMontage = AttackAnimations[animWeaponType];
					if (IsValid(animMontage))
					{
						LocalPlayAnimMontage(
							Cast<ACharacterBase>(GetOwner()), animMontage);
					}
				}
				
				if (AttackSounds.Num() > 0)
				{
					TArray<USoundBase*> SoundKeys;
					AttackSounds.GetKeys(SoundKeys);
					float getSoundNumber = FMath::RandRange(0, AttackSounds.Num() - 1);
					USoundBase* animSound = SoundKeys[getSoundNumber];
					float* soundDelay = AttackSounds.Find(animSound);
					if (IsValid(animSound))
					{
						DelaySoundEffect(animSound, soundDelay == nullptr? 0.f : *soundDelay);
					}
				}
				
				// Delay the next attack
				const int mSeconds = static_cast<int>(HitWeaponData.AttackDelay * 1000);
				_NextAttackTime = nowTime + FTimespan::FromMilliseconds(mSeconds);
			}
			
			// Whether or not it worked client side, always send a request to
			//    server for attack authorization, and let it handle the request.
			Server_RequestAttack(weaponType);
			
		}
		else
		{
			const EWeaponTypes animWeaponType = HitWeaponData.WeaponType;
			if (DrawAnimations.Contains(animWeaponType))
			{
				UAnimMontage* animMontage = DrawAnimations[animWeaponType];
				if (IsValid(animMontage))
				{
					LocalPlayAnimMontage(
						Cast<ACharacterBase>(GetOwner()), animMontage);
				}
			}

			if (DrawSounds.Num() > 0)
			{
				USoundBase* animSound = DrawSounds[ FMath::RandRange(0, DrawSounds.Num() - 1) ];
				if (IsValid(animSound))
				{
					DelaySoundEffect(animSound);
				}
			}
			
			// Delay the next attack
			const int mSeconds = static_cast<int>(HitWeaponData.AttackDelay * 1000);
			_NextAttackTime = nowTime + FTimespan::FromMilliseconds(mSeconds);
			
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
		_TargetActor = targetActor;
	else
		_TargetActor = nullptr;
}

void UWeaponComponent::ClearTargetedActor()
{
	if (!GetOwner()->HasAuthority()) return;
	_TargetActor = nullptr;
}

void UWeaponComponent::UnsetWeapon(EWeaponSlots weaponSlot)
{
	if (!GetOwner()->HasAuthority())
		return;
	
	// Block clients
	if (!GetOwner()->HasAuthority())
		return;

	AWeaponBase* tempWeapon = nullptr;
	switch (weaponSlot)
	{
	case EWeaponSlots::PRIMARY:
		tempWeapon = _PrimaryWeapon;
		break;
	case EWeaponSlots::SECONDARY:
		tempWeapon = _SecondaryWeapon;
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
		if (IsValid(_PrimaryWeapon))
			return _PrimaryWeapon->getIsWeaponArmed();
		break;
	case EWeaponSlots::SECONDARY:
		if (IsValid(_SecondaryWeapon))
			return _SecondaryWeapon->getIsWeaponArmed();
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

	ACharacter* ownerActor = Cast<ACharacter>(GetOwner());
	if (!IsValid(ownerActor))
		return;

	UnsetWeapon(weaponSlot);
	
	// Add the next weapon
	const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName(weaponName);
	if (UWeaponSystem::GetWeaponIsValid(weaponData))
	{
		if (UItemSystem::getItemNameIsValid(weaponName))
		{
			const TSubclassOf<AWeaponBase> weaponClass = weaponData.SpawnClass;
			
			FActorSpawnParameters spawnParams;
			spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// Spawn the actor and wait for us to set up it's data
			AWeaponBase* tempWeapon = GetWorld()->SpawnActorDeferred<AWeaponBase>(
						weaponClass, ownerActor->GetActorTransform(), ownerActor,
						nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
			
			if (IsValid(tempWeapon))
			{
				tempWeapon->setWeaponName(weaponName);
				
				FTransform newTransform;
				tempWeapon->FinishSpawning(newTransform);
				tempWeapon->SetOwner(ownerActor);

				switch(weaponSlot)
				{
				case EWeaponSlots::PRIMARY:
					_PrimaryWeapon = tempWeapon;
					break;
				case EWeaponSlots::SECONDARY:
					_SecondaryWeapon = tempWeapon;
					break;
				default:
					tempWeapon->Destroy();
					break;
				}
				
				AdjustWeaponAttachment(weaponSlot);
				
			}
			
		}
	}
	
}

// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	_NextAttackTime = FDateTime::UtcNow();
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicated via Multicast to everyone, for things like mesh sync
	DOREPLIFETIME_CONDITION(UWeaponComponent, _TargetActor,	 		COND_None);
	DOREPLIFETIME_CONDITION(UWeaponComponent, _PrimaryWeapon,	 	COND_None);
	DOREPLIFETIME_CONDITION(UWeaponComponent, _SecondaryWeapon,	 	COND_None);
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
	if (IsValid(_PrimaryWeapon))
		_PrimaryWeapon->Destroy();
	if (IsValid(_SecondaryWeapon))
		_SecondaryWeapon->Destroy();
	Super::OnUnregister();
}

float UWeaponComponent::WeaponReadyChanged(EWeaponSlots weaponSlot)
{
	AWeaponBase* hitWeapon = nullptr;
	switch(weaponSlot)
	{
	case EWeaponSlots::PRIMARY:
		if (IsValid(_PrimaryWeapon))
		{
			hitWeapon = _PrimaryWeapon;
		}
		break;
	case EWeaponSlots::SECONDARY:
		if (IsValid(_SecondaryWeapon))
		{
			hitWeapon = _SecondaryWeapon;
		}
		break;
	default:
		break;
	}
	
	if (IsValid(GetOwner()))
	{
		// If the montage is invalid
		const EWeaponTypes WeaponType = hitWeapon->getWeaponData().WeaponType;
		if (StowAnimations.Contains(WeaponType))
		{
			UAnimMontage* animMontage = StowAnimations[WeaponType];
			if (hitWeapon->getIsWeaponArmed())
			{
				if (DrawAnimations.Contains(WeaponType))
					animMontage = DrawAnimations[WeaponType];
				else
					animMontage = nullptr;
			}
			
			if (IsValid(animMontage))
			{
				if (GetOwner()->HasAuthority())
				{
					Multicast_SendAnimation(Cast<ACharacter>(GetOwner()), animMontage);
				}
				else
				{
					const float animLength = LocalPlayAnimMontage(Cast<ACharacter>(
												GetOwner()), animMontage);
					if (animLength <= 0.f)
					{
						UE_LOG(LogTemp, Warning, TEXT("Failed to play Anim Montage '%s'"), *animMontage->GetPathName());
					}
				}
			}
		}
		if (IsValid(hitWeapon))
		{
			return hitWeapon->getIsWeaponArmed() ?
				     hitWeapon->getWeaponData().DelayDrawTime
				   : hitWeapon->getWeaponData().DelayStowTime;
		}
	}
	return 0.f;
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
		SendSoundEffect(Cast<ACharacter>(GetOwner()), soundToPlay);
	else
	{
		FTimerDelegate attackArgs;
		attackArgs.BindUObject(this, &UWeaponComponent::SendSoundEffect,
			Cast<ACharacter>(GetOwner()), soundToPlay);
	
		FTimerHandle doSoundEffectTimer;
		GetWorld()->GetTimerManager().SetTimer(
			doSoundEffectTimer, attackArgs, soundDelay, false);
	}
}

void UWeaponComponent::Server_RequestAttack_Implementation(EWeaponSlots weaponType)
{
	const FDateTime nowTime = FDateTime::UtcNow();
	if (_NextAttackTime < nowTime)
	{
		// Determine the weapon being used
		AWeaponBase* weaponInUse = _PrimaryWeapon;
		bool isOperating = bPrimaryOperating;
		switch (weaponType)
		{
		case EWeaponSlots::SECONDARY:
			weaponInUse = _SecondaryWeapon;
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
		
		const FStWeaponData weaponData = weaponInUse->getWeaponData();

		// Authorize Attack
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Server_RequestAttack - doAttack()"), *GetName(), GetOwner()->HasAuthority()?TEXT("SRV"):TEXT("CLI"));

		// Is the weapon able to attack?
		if (weaponInUse->doAttack())
		{
			const EWeaponTypes animWeaponType = weaponInUse->getWeaponData().WeaponType;
			if (!AttackAnimations.Contains(animWeaponType))
			{
				return;
			}
				
			UAnimMontage* animMontage = AttackAnimations[animWeaponType];
			if (IsValid(animMontage))
			{
				Multicast_SendAnimation(
					Cast<ACharacterBase>(GetOwner()), animMontage);
			}
			
			// Add cooldown to the next attack
			const int mSeconds = static_cast<int>(weaponData.AttackDelay * 1000);
			_NextAttackTime = nowTime + FTimespan::FromMilliseconds(mSeconds);
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
	if (IsValid(_TargetActor)) newTgt = _TargetActor->GetName();
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Target has Changed. New Target = %s"), *GetName(), GetOwner()->HasAuthority()?TEXT("SRV"):TEXT("CLI"), *newTgt);
}
