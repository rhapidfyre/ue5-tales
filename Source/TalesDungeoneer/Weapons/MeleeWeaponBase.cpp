// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponBase.h"

#include "WeaponSystem.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"


// Sets default values
AMeleeWeaponBase::AMeleeWeaponBase()
{
	// Replication, tick etc is all set by parent "AWeaponBase"
	
	mHitDetector = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HitDetector"));
	if (IsValid(mHitDetector)) mHitDetector->SetAutoActivate(true);

	// MeleeWeapon Detection
	mHitDetector->SetGenerateOverlapEvents(true);

	// Set type as MeleeWeapon
	mHitDetector->SetCollisionObjectType(ECC_GameTraceChannel4);
	
	// Overlap Resource Nodes
	mHitDetector->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Overlap);

	mHitDetector->SetCollisionResponseToChannel(ECC_Pawn,			ECR_Overlap);
	mHitDetector->SetCollisionResponseToChannel(ECC_Visibility,		ECR_Ignore);
	mHitDetector->SetCollisionResponseToChannel(ECC_Camera,			ECR_Ignore);
	mHitDetector->SetCollisionResponseToChannel(ECC_PhysicsBody,	ECR_Overlap);
	
	mHitDetector->SetupAttachment(GetRootComponent());
}

void AMeleeWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IsValid(mHitDetector))
	{
		mHitDetector->RegisterComponent();
	}
}

void AMeleeWeaponBase::BeginDestroy()
{
	mHitDetector->OnComponentBeginOverlap.RemoveDynamic(this, &AMeleeWeaponBase::onMeleeWeaponHit);
	Super::BeginDestroy();
}


void AMeleeWeaponBase::startHitDetection()
{
	_HitTargets.Empty();
	startAttackTimer();
}

// Called when the game starts or when spawned
void AMeleeWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (!mHitDetector->OnComponentBeginOverlap.IsAlreadyBound(this, &AMeleeWeaponBase::onMeleeWeaponHit))
		 mHitDetector->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeaponBase::onMeleeWeaponHit);
}

TArray<AActor*> AMeleeWeaponBase::getOverlappingResources()
{
	if (!bCanCollide) return {};
	TArray<AActor*> hitActors;
	mHitDetector->GetOverlappingActors(hitActors);
	for (AActor* tempActor : hitActors)
	{
		if (IsValid(tempActor))
		{
			hitActors.Add(tempActor);
			_HitTargets.Add(tempActor);
		}
	}
	return hitActors;
}

void AMeleeWeaponBase::onMeleeWeaponHit(UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (getIsAttacking())
	{
 		if (IsValid(OtherActor))
			_HitTargets.Add(OtherActor);
	}
}

void AMeleeWeaponBase::startAttackTimer()
{
	// If anything is targeted, hit that first, if able
	ACharacterBase* WeaponOwner = Cast<ACharacterBase>(GetOwner());
	if (IsValid(WeaponOwner))
	{
		ACharacterBase* WeaponTarget = WeaponOwner->AbilityComponent->GetTargetedActor();
		if (IsValid(WeaponTarget))
		{
			// TODO - Line of Sight check

			// If the target is within distance, hit that target.
			if (WeaponOwner->GetDistanceTo(WeaponTarget) <= getWeaponData().MaxReachDistance)
			{
				Server_RequestWeaponHit({WeaponTarget});
				cancelAttackTimer();

				// End attack if only one target can be hit
				if (getWeaponData().MaxTargetsHitAtOnce <= 1)
					return;
			}
		}
	}
	
	// Hit anything already within the collision area
	TArray<AActor*> HitActors;
	if (checkForHit(HitActors))
	{
		// Only one hit
		if (getWeaponData().MaxTargetsHitAtOnce <= 1)
		{
			_HitTargets.Add(HitActors[0]);
			cancelAttackTimer();
			return;
		}

		// Weapon can hit multiple targets
		for (AActor* HitActor : HitActors)
		{
			if (_HitTargets.Num() < getWeaponData().MaxTargetsHitAtOnce)
			{
				_HitTargets.Add(HitActor);
			}
			else
			{
				cancelAttackTimer();
				return;
			}
		}
	}
	Super::startAttackTimer();
}

void AMeleeWeaponBase::cancelAttackTimer()
{
	Super::cancelAttackTimer();
	
	if (!_HitTargets.IsEmpty())
		Server_RequestWeaponHit(_HitTargets.Array());
	
	_HitTargets.Empty();
}

void AMeleeWeaponBase::updateWeapon()
{
	Super::updateWeapon();
}

bool AMeleeWeaponBase::doAttack()
{
	// Verifies that the weapon is able to be used
	if (Super::doAttack())
	{
		// Cancels the attack timer after attack completes
		const FStWeaponData weaponData = getWeaponData();
		const float KillTime = weaponData.HitDetectDelay > weaponData.HitDetectStop
		                     ? weaponData.HitDetectStop - 0.1 : weaponData.HitDetectDelay;

		// Activates the hit detector, optionally with a delay
		if (KillTime > 0.f)
		{
			GetWorld()->GetTimerManager().SetTimer(_DelayTimer, this,
					&AMeleeWeaponBase::startHitDetection, KillTime, false);
		}
		else
			startHitDetection();
		
		return true;
	}
	return false;
}

bool AMeleeWeaponBase::checkForHit(TArray<AActor*>& HitActors)
{
	if (!getIsAttacking()) return false;
	// Checks if the melee weapon's collision is overlapping anything
	const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName( getWeaponName() );
	if (!UWeaponSystem::GetWeaponIsValid(weaponData)) return false;
	HitActors = getOverlappingResources();
	return HitActors.Num() > 0;
}

