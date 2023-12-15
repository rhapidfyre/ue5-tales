// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MeleeWeaponBase.h"

#include "Characters/CharacterBase.h"


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

void AMeleeWeaponBase::startHitDetection()
{
	startAttackTimer();
}

// Called when the game starts or when spawned
void AMeleeWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
	mHitDetector->IgnoreActorWhenMoving(GetOwner(), true);

	if (!mHitDetector->OnComponentBeginOverlap.IsAlreadyBound(this, &AMeleeWeaponBase::onMeleeWeaponHit))
		 mHitDetector->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeaponBase::onMeleeWeaponHit);
}

TArray<AActor*> AMeleeWeaponBase::getOverlappingResources()
{
	TArray<AActor*> hitActors;
	mHitDetector->GetOverlappingActors(hitActors);
	TArray<AActor*> ArrayCopy = hitActors;
	for (AActor* tempActor : hitActors)
	{
		if (IsValid(tempActor))
		{
			if (tempActor != GetOwner())
			{
				hitActors.Add(tempActor);
				TargetHitByWeapon(tempActor);
			}
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
			TargetHitByWeapon(OtherActor);
	}
}

void AMeleeWeaponBase::startAttackTimer()
{
	_HitTargets.Empty();
	
	// If anything is targeted, hit that first, if able
	ACharacterBase* WeaponOwner = Cast<ACharacterBase>(GetOwner());
	if (IsValid(WeaponOwner))
	{

	}
	
	// Hit anything already within the collision area
	TArray<AActor*> HitActors;
	if (CheckForHit(HitActors))
	{
		// Only one hit
		if (getWeaponData().MaxTargetsHitAtOnce <= 1)
		{
			TargetHitByWeapon(HitActors[0]);
			cancelAttackTimer();
			return;
		}

		// Weapon can hit multiple targets
		for (AActor* HitActor : HitActors)
		{
			if (_HitTargets.Num() < getWeaponData().MaxTargetsHitAtOnce)
			{
				TargetHitByWeapon(HitActor);
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
	_HitTargets.Empty();
}

void AMeleeWeaponBase::TargetHitByWeapon(AActor* HitActor)
{
	// Do not allow the hit to be the wielder of the weapon
	if (HitActor == GetOwner()) return;

	// If the target is a valid target, add them to the hit targets list
	// and dispatch the server event
	if (IsValid(Cast<ACharacterBase>(HitActor)))
	{
		_HitTargets.Add(HitActor);
		Server_RequestWeaponHit(HitActor);
	}
	
	// Cancel further attacks if max targets has been reached
	if (_HitTargets.Num() >= getWeaponData().MaxTargetsHitAtOnce)
	{
		cancelAttackTimer();
	}
}

void AMeleeWeaponBase::UpdateWeapon()
{
	Super::UpdateWeapon();
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
		
		// Melee weapons always play their miss sound (i.e: "WHOOSH!")
		Server_PlayWeaponEffect(EWeaponEffectType::MISS);
		return true;
	}
	return false;
}

bool AMeleeWeaponBase::CheckForHit(TArray<AActor*>& HitActors)
{
	if (!getIsWeaponArmed()) return false;
	/*
	// Checks if the melee weapon's collision is overlapping anything
	const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName( getWeaponName() );
	if (!UWeaponSystem::GetWeaponIsValid(weaponData)) return false;
	HitActors = getOverlappingResources();
	return HitActors.Num() > 0;
	*/
	return false;
}

