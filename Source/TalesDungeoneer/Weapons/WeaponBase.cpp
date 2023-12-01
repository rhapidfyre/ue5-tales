// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "WeaponSystem.h"
#include "lib/ItemData.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"
#include "Engine/DamageEvents.h"
#include "Logging/StructuredLog.h"
#include "TalesDungeoneer/Characters/CombatNpcCharacterBase.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bNetLoadOnClient = true;
	bNetUseOwnerRelevancy = true;
	
	mWeaponName = UItemSystem::getInvalidName();

	WeaponRoot = CreateDefaultSubobject<USceneComponent>("WeaponRoot");
	SetRootComponent(WeaponRoot);

	// Allow use of static meshes
	WeaponMeshStatic = CreateDefaultSubobject<UStaticMeshComponent>("WeaponMeshStatic");
	WeaponMeshStatic->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshStatic->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMeshStatic->SetupAttachment(WeaponRoot, "root");

	// Allow use of skeletal meshes
	WeaponMeshSkeleton = CreateDefaultSubobject<USkeletalMeshComponent>("WeaponMeshSkeleton");
	WeaponMeshSkeleton->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshSkeleton->SetCollisionResponseToAllChannels(ECR_Ignore);
	WeaponMeshSkeleton->SetupAttachment(WeaponRoot, "root");

	// Optional weapon grip positions
	// If not used, the weapon will be gripped at root
	WeaponGripLeft = CreateDefaultSubobject<USceneComponent>("WeaponGripLeft");
	WeaponGripLeft->SetupAttachment(WeaponRoot);
	WeaponGripRight = CreateDefaultSubobject<USceneComponent>("WeaponGripRight");
	WeaponGripRight->SetupAttachment(WeaponRoot);
	
	
}

void AWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetReplicates(true);
	UpdateWeapon();
}

void AWeaponBase::BeginPlay()
{
	SetActorTickEnabled(true);
	Super::BeginPlay();
	UpdateWeapon();
}

void AWeaponBase::PostActorCreated()
{
	Super::PostActorCreated();
}

void AWeaponBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): EndPlay()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
}

void AWeaponBase::setWeaponName(FName weaponName)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): setWeaponName(%s)"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *weaponName.ToString());
	}
	// NetMulticast should run on both server and client, updating the mesh and everything
	if (HasAuthority())
		mWeaponName = weaponName;
}

/**
 * Set to true when the weapon should be drawn. False to stow.
 * @param setArmed True for draw, false for stow.
 * @return True if the change was successful, false otherwise.
 */
bool AWeaponBase::setWeaponIsArmed(bool setArmed)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): setWeaponIsArmed(%s)"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), setArmed?TEXT("TRUE"):TEXT("FALSE"));
	}
	
	if (!HasAuthority())
		return false;
	
	if (GetWorldTimerManager().IsTimerActive(mAttackTimer))
		return false;

	UE_LOGFMT(LogTemp, Display, "{WeaponName}({Sv}): Cooldown Started (Readiness Changed)",
		*GetName(), HasAuthority()?"S":"C");
	FTimerDelegate AttackDelegate;
	AttackDelegate.BindUObject(this, &AWeaponBase::cancelAttackTimer);
	GetWorldTimerManager().SetTimer(mAttackTimer, AttackDelegate,
		setArmed ?
		getWeaponData().DelayDrawTime : getWeaponData().DelayStowTime, false);
	
	bIsWeaponArmed	= setArmed;
	if (bIsWeaponArmed)	startDrawEffects();
	else				startStowEffects();
	return true;
}

/** Called by the client or server to validate the hit & calculate damage
 * * Calls 'TakeDamage' on the actor after verification.
 * @param HitActor The actor taking the hit
 */
void AWeaponBase::Server_RequestWeaponHit_Implementation(AActor* HitActor)
{
	if (!IsValid(HitActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): Server_RequestWeaponHit() - Hit Actor is INVALID"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return;
	}

	// If the hit distance is greater than the weapon's max reach, then the hit is invalid.
	const FStWeaponData weaponData = getWeaponData();

	// Verify the weapon owner's controller is valid
	AController* OwnerController = GetOwner()->GetInstigatorController();
	if (!IsValid(OwnerController))
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Server_RequestWeaponHit() - Owner Controller INVALID"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return ;
	}

	// Allows child classes to implement a validation failure
	if (!GetIsAttackValid(HitActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): Server_RequestWeaponHit() - GetIsAttackValid returned FALSE!"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return;
	}

	// Allows child blueprints to implement validation failure
	if (!IsAttackValid(HitActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): Server_RequestWeaponHit() - IsAttackValid(BP) returned FALSE!"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return;
	}
	
	float totalDamage = 0.f;
	float dmgMultiplier = 1.f;

	// Apply each damage type on the weapon to the hit actor individually
	// Allows for resistances, vulnerabilities, etc. to be checked properly
	for (const FStWeaponDamageData damageData : weaponData.DamageData)
	{
		const float temp = damageData.BaseDamage * dmgMultiplier;
		float dmgMod = 0.f;

		// Modify damage based on the variance of the weapon
		if (damageData.DamageVariance)
		{
			const float dmgVariance = damageData.DamageVariance * temp;
			dmgMod = FMath::RandRange(0-dmgVariance, dmgVariance);
		}
		totalDamage += (temp + dmgMod);

		FDamageEvent pointDamage(damageData.DamageType);
		
		// Each damage needs to be applied individually to allow for resist/bonus
		// Resistance and vulnerability will be calculated by the actor taking the damage
		HitActor->TakeDamage(totalDamage, pointDamage, OwnerController, GetOwner());
	}

	// Dispatch Hit Effects
	// Always play the hit noise & particle effects, even if all the damage was resisted
	if (IsValid(weaponData.WeaponSounds.UseSoundWeaponHit))
	{
		soundEffectWithDelay(weaponData.WeaponSounds.UseSoundWeaponHit,
			weaponData.WeaponSounds.DelaySoundWeaponHit);
	}
	
	// Play the "I got hit" animation if any damage was actually taken
	const ACharacterBase* HitCharacterBase = Cast<ACharacterBase>( HitActor );
	if ( IsValid(HitCharacterBase) && (totalDamage > 0.f) )
	{
		// Ask the Vitality System to play hit effects
		HitCharacterBase->VitalityWelfare->HitByWeapon();
	}
}

/** Used on the client making the attack. Performs sounds, animations and
 * all hit related effects prior to the server event, to ensure everything
 * appears seamless to the client making the attack.
 * @param HitActor The actor that was hit by the attack
 */
void AWeaponBase::PerformWeaponHit(AActor* HitActor)
{
	OnHit.Broadcast(HitActor);
}

bool AWeaponBase::getIsMeleeWeapon()
{
	switch (getWeaponData().WeaponType)
	{
	case EWeaponTypes::NONE:	return true;
	case EWeaponTypes::PICKAXE: return true;
	case EWeaponTypes::SPEAR:	return true;
	case EWeaponTypes::SWORD:	return true;
	default: break;
	}
	return false;
}

bool AWeaponBase::getIsRangedWeapon()
{
	return !getIsMeleeWeapon();
}

FStWeaponData AWeaponBase::getWeaponData()
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): getWeaponData()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	return UWeaponSystem::GetWeaponDataFromName(mWeaponName);
}

void AWeaponBase::OnRep_IsWeaponArmed_Implementation()
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): OnRep_IsWeaponArmed_Implementation()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	// Called when the weapon is toggled
}

void AWeaponBase::OnRep_WeaponName_Implementation()
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): OnRep_WeaponName_Implementation()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	UpdateWeapon();
}


void AWeaponBase::startStowEffects(float delayTime)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): startStowEffects()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	const FStWeaponData weaponData = getWeaponData();
		
	// Play draw sound after given delay
	const FStWeaponSoundData soundData = weaponData.WeaponSounds;
	if (IsValid(soundData.UseSoundStowWeapon))
		soundEffectWithDelay(soundData.UseSoundStowWeapon, soundData.DelaySoundStowWeapon);

	// Stop associated particle effects
	
}

void AWeaponBase::startDrawEffects(float delayTime)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): startDrawEffects()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	const FStWeaponData weaponData = getWeaponData();
		
	// Play draw sound after given delay
	const FStWeaponSoundData soundData = weaponData.WeaponSounds;
	if (IsValid(soundData.UseSoundDrawWeapon))
	{
		soundEffectWithDelay(soundData.UseSoundDrawWeapon, soundData.DelaySoundDrawWeapon);
	}

}

/** Plays the given effect based on the enum requested
 * This should only be called by the client performing the action
 * @param WeaponEffect The enum specifying which action is being taken
 */
void AWeaponBase::Server_PlayWeaponEffect_Implementation(const EWeaponEffectType WeaponEffect)
{
	const FStWeaponData WeaponData = getWeaponData();
	float WeaponSoundDelay;
	USoundBase* WeaponSoundBase;
	
	switch (WeaponEffect)
	{
	case (EWeaponEffectType::ATTACK):
		WeaponSoundBase  = WeaponData.WeaponSounds.UseSoundWeaponAttack;
		WeaponSoundDelay = WeaponData.WeaponSounds.DelaySoundWeaponAttack;
		break;
		
	case (EWeaponEffectType::HIT):
		WeaponSoundBase  = WeaponData.WeaponSounds.UseSoundWeaponHit;
		WeaponSoundDelay = WeaponData.WeaponSounds.DelaySoundWeaponHit;
		break;
		
	default:
		return;
	}
	
	if (!IsValid(WeaponSoundBase))
	{
		return;
	}
	soundEffectWithDelay(WeaponSoundBase, WeaponSoundDelay);
}


/** Processes the requested sound and delay timer, sending it to all clients.
 * @param SoundBase The sound to be played
 * @param TimerRate The delay. Defaults to zero.
 */
void AWeaponBase::soundEffectWithDelay(USoundBase* SoundBase, float TimerRate)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): soundEffectWithDelay()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	if (TimerRate < 0.01)
	{
		sendSoundEffect(SoundBase);
		return;
	}
	FTimerDelegate soundArgs; FTimerHandle soundTimer;
	soundArgs.BindUObject(this, &AWeaponBase::PrepSoundEffect, SoundBase);
	GetWorld()->GetTimerManager().SetTimer(soundTimer, soundArgs, TimerRate, false);
}

void AWeaponBase::niagaraEffectWithDelay(
	UNiagaraSystem* niagaraEffect, float effectDelay, float isLooped)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): niagaraEffectWithDelay()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	//TODO
}

void AWeaponBase::particleEffectWithDelay(UParticleSystem* particleEffect, float effectDelay, float isLooped)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): particleEffectWithDelay()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	//TODO
}

void AWeaponBase::sendSoundEffect_Implementation(USoundBase* soundEffect)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): sendSoundEffect_Implementation()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	if (!IsValid(soundEffect)) return;
	UAudioComponent* tempAudio = NewObject<UAudioComponent>(this);
	if (IsValid(tempAudio))
	{
		tempAudio->RegisterComponent();
		tempAudio->AutoAttachParent = GetRootComponent();
		tempAudio->bAutoManageAttachment = true;
		tempAudio->SetSound(soundEffect);
		tempAudio->Activate(true);
		tempAudio->Play();
		tempAudio->bAutoDestroy = true;
	}
}

void AWeaponBase::UpdateWeapon()
{
	const FStWeaponData WeaponData = getWeaponData();
	MaxTargetsHitAtOnce_ = WeaponData.MaxTargetsHitAtOnce;
	
	if (UWeaponSystem::GetWeaponNameIsValid(getWeaponName()))
	{
		if (IsValid(WeaponData.MeshStatic))
			UsingStaticMesh = WeaponData.MeshStatic;
		if (IsValid(WeaponData.MeshSkeletal))
			UsingSkeletalMesh = WeaponData.MeshSkeletal;
	}

	// Validate Mesh and realign it
	if (IsValid(UsingStaticMesh) || IsValid(UsingSkeletalMesh))
	{
		
		if (IsValid(UsingSkeletalMesh))
		{
			WeaponMeshSkeleton->SetSkeletalMesh(UsingSkeletalMesh);
		}
		else
		{
			if (IsValid(UsingStaticMesh))
			{
				WeaponMeshStatic->SetStaticMesh(UsingStaticMesh);
			}
		}
		WeaponMeshSkeleton->ResetRelativeTransform();
		WeaponMeshSkeleton->AddRelativeLocation(WeaponData.MeshOffset);
		WeaponMeshSkeleton->AddRelativeRotation(WeaponData.MeshRotOffset);
	}
}

bool AWeaponBase::CheckForHit(TArray<AActor*>& HitActors)
{
	return false; // Implemented in child classes
}

void AWeaponBase::startAttackTimer()
{
	if (getIsAttacking())
		return;
		
	// Cancel the attack timer after attack completes
	const FStWeaponData weaponData = getWeaponData();
	float killTime = weaponData.HitDetectStop > weaponData.AttackDelay
	               ? weaponData.HitDetectStop : weaponData.AttackDelay;

	// Add some randomness to NPC attack timers
	const ACombatNpcCharacterBase* NpcAttacker = Cast<ACombatNpcCharacterBase>( GetOwner() );
	if (IsValid(NpcAttacker))
	{
		killTime += FMath::RandRange(
			NpcAttacker->TimeBetweenAttacks[0],
			NpcAttacker->TimeBetweenAttacks[1]);
	}

	UE_LOGFMT(LogTemp, Display, "{WeaponName}({Sv}): Cooldown Started (Attacking)",
		*GetName(), HasAuthority()?"S":"C");
	FTimerDelegate AttackDelegate;
	AttackDelegate.BindUObject(this, &AWeaponBase::cancelAttackTimer);
	GetWorld()->GetTimerManager().SetTimer(mAttackTimer,
					AttackDelegate, killTime, false);
	
}

void AWeaponBase::cancelAttackTimer()
{
	UE_LOGFMT(LogTemp, Display, "{WeaponName}({Sv}): Ended Cooldown",
		*GetName(), HasAuthority()?"S":"C");
	if (GetWorld()->GetTimerManager().IsTimerActive(mAttackTimer))
		(GetWorld()->GetTimerManager().ClearTimer(mAttackTimer));
}

/** Allows child classes to deny or allow the attack.
 * ex: Melee weapon checks distance - Invalid distance is found, then
 * the attack check could return false and indicate failure.
 * @param HitActor The actor to run the validation against (hit target)
 * @return True by default. Children should set true or false accordingly.
 */
bool AWeaponBase::GetIsAttackValid(AActor* HitActor)
{
	return true;
}

/** Allows BLUEPRINT child classes to deny or allow the attack.
 * ex: Melee weapon checks distance - Invalid distance is found, then
 * the attack check could return false and indicate failure.
 * @param HitActor The actor to run the validation against (hit target)
 * @return True by default. Children should set true or false accordingly.
 */
bool AWeaponBase::IsAttackValid_Implementation(AActor* HitActor)
{
	return true;
}

void AWeaponBase::Multicast_PlayWeaponAttack_Implementation()
{
	if (UWeaponSystem::GetWeaponNameIsValid(getWeaponName()))
	{
		const FStWeaponData WeaponData = getWeaponData();
		
	}
}

void AWeaponBase::Multicast_PlayWeaponHit_Implementation()
{
}

void AWeaponBase::Multicast_PlayWeaponStow_Implementation()
{
}

void AWeaponBase::Multicast_PlayWeaponDraw_Implementation()
{
}

void AWeaponBase::stopAttack_Implementation()
{
	cancelAttack();
}


bool AWeaponBase::doAttack()
{
	if (!bIsWeaponArmed)
		return false;

	if (GetWorldTimerManager().IsTimerActive(mAttackTimer))
		return false;
	
	bool WeaponCanDoAttack = false;
	
	// Make sure this actor is a valid actor before running attack logic and accessing bad memory
	const FName weaponName = getWeaponName();
	const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName(weaponName);
	if (UWeaponSystem::GetWeaponIsValid(weaponData))
	{
		// Weapon does damage?
		for (const FStWeaponDamageData iterDamageData : getWeaponData().DamageData)
		{
			if (iterDamageData.BaseDamage > 0) 
			{
				// If the weapon is valid and does damage, the attack passes.
				// We don't check delay/timers; That's the job of the UWeaponComponent
				WeaponCanDoAttack = true;
			}
		}
	}

	// Request weapon attack effect from server for multicast
	if (WeaponCanDoAttack)
		Server_PlayWeaponEffect(EWeaponEffectType::ATTACK);
	
	startAttackTimer();
	return WeaponCanDoAttack;
}

void AWeaponBase::cancelAttack()
{
	cancelAttackTimer();
}

bool AWeaponBase::getIsAttacking()
{
	return GetWorld()->GetTimerManager().IsTimerActive(mAttackTimer);
}

void AWeaponBase::startAttack_Implementation()
{
	doAttack();
}

/**
 * -------------------------------------------------------------------------------------------------------------------
 *				REPLICATION
 * -------------------------------------------------------------------------------------------------------------------
 */

// base Unreal Engine AActor* override for replication
void AWeaponBase::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): GetLifetimeReplicatedProps()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	DOREPLIFETIME_CONDITION(AWeaponBase, bIsWeaponArmed, COND_None);
	DOREPLIFETIME_CONDITION(AWeaponBase, mWeaponName, COND_None);
    
}