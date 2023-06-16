// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeWeaponBase.h"

#include "WeaponSystem.h"


// Sets default values
AMeleeWeaponBase::AMeleeWeaponBase()
{
	// Replication, tick etc is all set by parent "AWeaponBase"
	
	mHitDetector = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HitDetector"));
	if (IsValid(mHitDetector)) mHitDetector->SetAutoActivate(true);
	
	mHitDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	mHitDetector->SetCollisionObjectType(ECC_GameTraceChannel4); // MeleeWeapon Detection

	mHitDetector->SetCollisionResponseToChannel(ECC_Pawn,			ECR_Overlap);
	mHitDetector->SetCollisionResponseToChannel(ECC_Visibility,		ECR_Block);
	mHitDetector->SetCollisionResponseToChannel(ECC_Camera,			ECR_Ignore);
	mHitDetector->SetCollisionResponseToChannel(ECC_PhysicsBody,	ECR_Block);
	
	// Overlap Resource Nodes
	mHitDetector->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Overlap);

	// Overlap Pawns/Characters
	mHitDetector->SetupAttachment(mSkeletalMesh);
}

void AMeleeWeaponBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (IsValid(mHitDetector))
	{
		mHitDetector->RegisterComponent();
		mHitDetector->Deactivate();
		mHitDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

// Called when the game starts or when spawned
void AMeleeWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		if ( IsValid(mHitDetector) )
		{
			if (!mHitDetector->OnComponentBeginOverlap.IsAlreadyBound(this, &AMeleeWeaponBase::onMeleeWeaponHit))
				 mHitDetector->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeaponBase::onMeleeWeaponHit);
		}
	}
}

TArray<AActor*> AMeleeWeaponBase::getOverlappingResources() const
{
	TArray<AActor*> hitActors;
	mHitDetector->GetOverlappingActors(hitActors);
	for (AActor* tempActor : hitActors)
	{
		if (IsValid(tempActor))
			hitActors.Add(tempActor);
	}
	return hitActors;
}

void AMeleeWeaponBase::onMeleeWeaponHit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
 	if (IsValid(OtherActor))
	{
		if (getIsAttacking())
		{
			PerformWeaponHit(OtherActor);
			Server_RequestWeaponHit(OtherActor);
		}
	}
}

void AMeleeWeaponBase::startAttackTimer()
{
	mHitDetector->Activate();
	mHitDetector->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Super::startAttackTimer();
}

void AMeleeWeaponBase::cancelAttackTimer()
{
	Super::cancelAttackTimer();
	mHitDetector->Deactivate();
	mHitDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMeleeWeaponBase::updateWeapon()
{
	Super::updateWeapon();
	if (IsValid(mSkeletalMesh))
	{
		const FStWeaponData weaponData = getWeaponData();
		const FVector hitStart = mSkeletalMesh->GetSocketTransform("HitStart", RTS_World).GetLocation();
		const FVector hitStop  = mSkeletalMesh->GetSocketTransform("HitStop",  RTS_World).GetLocation();
		const float halfHeight = FVector::Dist(hitStart, hitStop)/2;

		mHitDetector->ResetRelativeTransform();
		mHitDetector->AttachToComponent(mSkeletalMesh,
			FAttachmentTransformRules::SnapToTargetIncludingScale, "HitStart");

		// Rotate, then Set center to midpoint
		mHitDetector->SetCapsuleSize(weaponData.MaxHitRadius, halfHeight, false);

		FRotator relativeRot(mSkeletalMesh->GetSocketTransform("HitStart", RTS_ParentBoneSpace).GetRotation());
		relativeRot.Add(0.f,0.f,0.f);

		// Sets the rotation and location to be accurate to the weapon's sockets
		mHitDetector->AddRelativeRotation(relativeRot);
	}
}

bool AMeleeWeaponBase::doAttack()
{
	if (Super::doAttack())
	{
		startAttackTimer();
		return true;
	}
	return false;
}

bool AMeleeWeaponBase::checkForHit()
{
	// Checks if the melee weapon's collision is overlapping anything
	const FStWeaponData weaponData = UWeaponSystem::GetWeaponDataFromName( getWeaponName() );
	if (!UWeaponSystem::GetWeaponIsValid(weaponData)) return false;
	return getOverlappingResources().Num() > 0;
}

