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

}

void AMeleeWeaponBase::TargetHitByWeapon(AActor* HitActor)
{

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
		return true;
	}
	return false;
}
