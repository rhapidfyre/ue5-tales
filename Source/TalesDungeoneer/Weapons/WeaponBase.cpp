// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"

#include "WeaponSystem.h"
#include "lib/ItemData.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"
#include "Engine/DamageEvents.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bNetLoadOnClient = true;
	bNetUseOwnerRelevancy = true;
	
	mWeaponName = UItemSystem::getInvalidName();

	mSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	mSkeletalMesh->SetupAttachment(GetRootComponent());
	
	mSkeletalMesh->SetSimulatePhysics(false);
	mSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (IsValid(mSkeletalMesh)) mSkeletalMesh->SetAutoActivate(true);
	mSkeletalMesh->SetIsReplicated(true);
	mSkeletalMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	SetActorTickEnabled(true);
}




void AWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IsValid(mSkeletalMesh))
	{
		if (IsValid(mSkeletalMesh)) mSkeletalMesh->RegisterComponent();
		
		// Setup the Skeletal Mesh. Attachment will be handled by whoever spawned this actor.
		if (mWeaponName != UItemSystem::getInvalidName())
		{
			const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName(mWeaponName);
			if (UWeaponSystem::GetWeaponIsValid(weaponData))
			{
				// Setup the skeletal mesh
				mSkeletalMesh->SetSkeletalMesh(weaponData.Mesh);
				mSkeletalMesh->AddRelativeLocation(weaponData.MeshOffset);
				mSkeletalMesh->AddRelativeRotation(weaponData.MeshRotOffset);
			
			}//weapon found in data table
		}//weapon name is valid
	}//skeletal mesh valid
	SetReplicates(true);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		if (!IsValid(mSkeletalMesh))
		{
			UE_LOG(LogTemp, Error, TEXT("%s(%s): Skeletal Mesh FAILED to initialize. Cannot continue."),
				*GetName(), HasAuthority()?TEXT("SRV"):TEXT("CLI"));
			Destroy();
			return;
		}
	}
	updateWeapon();
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
	if (HasAuthority()) mWeaponName = weaponName;
}

void AWeaponBase::setWeaponIsArmed(bool setArmed)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): setWeaponIsArmed(%s)"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), setArmed?TEXT("TRUE"):TEXT("FALSE"));
	}
	if (!HasAuthority()) return;
	bIsWeaponArmed	= setArmed;
	bIsOperating	= true;
	if (bIsWeaponArmed)	startDrawEffects();
	else				startStowEffects();
	bIsOperating = false;
}

void AWeaponBase::Server_RequestWeaponHit_Implementation(AActor* hitActor)
{
	if (!IsValid(hitActor))
	{
		UE_LOG(LogTemp, Error, TEXT("%s(%s): Server_RequestWeaponHit() - hitActor INVALID. Cannot proceed."),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return;
	}
	
	const FDateTime nowTime = FDateTime::UtcNow();
	if (mNextAttackTime > nowTime)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Server_RequestWeaponHit() - Attacks are occuring too quickly"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return;
	}

	// If the hit distance is greater than the weapon's max reach, then the hit is invalid.
	const FStWeaponData weaponData = getWeaponData();

	const float mSeconds = weaponData.AttackDelay * 1000;
	mNextAttackTime = nowTime + FTimespan::FromMilliseconds(mSeconds);
	
	const float hitDistance = GetDistanceTo(hitActor);
	const float maxHitRange = weaponData.MaxReachDistance + 32.f;
	if (hitDistance > maxHitRange)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Server_RequestWeaponHit() - Invalid Attack! Too far away! (%f > %f)"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), hitDistance, maxHitRange);
		return;
	}

	float totalDamage = 0.f;
	float dmgMultiplier = 1.f;
	
	for (int i = 0; i < weaponData.DamageData.Num(); i++)
	{
		FStWeaponDamageData damageData = weaponData.DamageData[i];
		const float temp = damageData.BaseDamage * dmgMultiplier;
		float dmgMod = 0.f;
		
		if (damageData.DamageVariance)
		{
			const float dmgVariance = damageData.DamageVariance * temp;
			dmgMod = FMath::RandRange(0-dmgVariance, dmgVariance);
		}
		totalDamage += (temp + dmgMod);

		FDamageEvent pointDamage(damageData.DamageType);
		AController* ownerController = nullptr;
		if (IsValid(GetOwner()))
		{
			const ACharacterBase* charBase = Cast<ACharacterBase>(GetOwner());
			if (IsValid(GetOwner()))
				ownerController = charBase->GetController();
		}
		hitActor->TakeDamage(totalDamage, pointDamage, ownerController, this);
		
	}
	
}

void AWeaponBase::PerformWeaponHit(AActor* hitActor)
{
	OnHit.Broadcast(hitActor);
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
	updateWeapon();
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

void AWeaponBase::soundEffectWithDelay(USoundBase* soundEffect, float soundDelay)
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): soundEffectWithDelay()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	if (soundDelay < 0.01)
	{
		sendSoundEffect(soundEffect);
		return;
	}
	FTimerDelegate soundArgs; FTimerHandle soundTimer;
	soundArgs.BindUObject(this, &AWeaponBase::PrepSoundEffect, soundEffect);
	GetWorld()->GetTimerManager().SetTimer(soundTimer, soundArgs, soundDelay, false);
}

void AWeaponBase::niagaraEffectWithDelay(UNiagaraSystem* niagaraEffect, float effectDelay, float isLooped)
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
	if (IsValid(tempAudio) && IsValid(mSkeletalMesh))
	{
		tempAudio->RegisterComponent();
		tempAudio->AutoAttachParent = mSkeletalMesh;
		tempAudio->bAutoManageAttachment = true;
		tempAudio->SetSound(soundEffect);
		tempAudio->Activate(true);
		tempAudio->Play();
		tempAudio->bAutoDestroy = true;
	}
}

void AWeaponBase::updateWeapon()
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): updateWeapon()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	if (mWeaponName.IsValid())
	{
		if (UWeaponSystem::GetWeaponNameIsValid(mWeaponName))
		{
			// Setup the Skeletal Mesh. Attachment will be handled by whoever called this function.
			if (mWeaponName != UItemSystem::getInvalidName())
			{
				const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName(mWeaponName);
				if (UWeaponSystem::GetWeaponIsValid(weaponData))
				{
					mSkeletalMesh->SetSkeletalMesh(weaponData.Mesh);
					mSkeletalMesh->SetSimulatePhysics(false);
					mSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
		}
	}
}

bool AWeaponBase::checkForHit()
{
	return false; // Implemented in child classes
}

void AWeaponBase::startAttackTimer()
{
	// Invalidate the old attack timer, then recreate it.
	GetWorld()->GetTimerManager().ClearTimer(mAttackTimer);
	
	// Cancel the attack timer after attack completes
	const FStWeaponData weaponData = getWeaponData();
	const float killTime = weaponData.HitDetectStop      > weaponData.AttackDelay
						 ? weaponData.AttackDelay - 0.05 : weaponData.HitDetectStop;
	
	GetWorld()->GetTimerManager().SetTimer(mAttackTimer, this,
			&AWeaponBase::cancelAttackTimer, killTime, false);
	
}

void AWeaponBase::cancelAttackTimer()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(mAttackTimer))
		(GetWorld()->GetTimerManager().ClearTimer(mAttackTimer));
}

void AWeaponBase::stopAttack_Implementation()
{
	
}


bool AWeaponBase::doAttack()
{
	if (!bIsWeaponArmed) return false;
	// Make sure this actor is a valid actor before running attack logic and accessing bad memory
	const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName(mWeaponName);
	if (UWeaponSystem::GetWeaponIsValid(weaponData))
	{
		// Weapon does damage?
		for (const FStWeaponDamageData iterDamageData : getWeaponData().DamageData)
		{
			if (iterDamageData.BaseDamage > 0) 
			{
				// If the weapon is valid and does damage, the attack passes.
				// We don't check delay/timers; That's the job of the UWeaponComponent
				return true;
			}
		}
	}
	return false;
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
	if (doAttack())
		startAttackTimer();
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