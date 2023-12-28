// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponBase.h"

#include "Weapons/WeaponSystem.h"
#include "Components/AudioComponent.h"
#include "Net/UnrealNetwork.h"
#include "Logging/StructuredLog.h"
#include "Characters/CombatNpcCharacterBase.h"

AWeaponBase::AWeaponBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bNetLoadOnClient = true;
	bNetUseOwnerRelevancy = true;

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

/**
 * Set to true when the weapon should be drawn. False to stow.
 * @param setArmed True for draw, false for stow.
 * @return Whether the weapon is now armed (true) or unarmed (false)
 */
bool AWeaponBase::setWeaponIsArmed(bool setArmed)
{
	bIsWeaponArmed = setArmed;
	return bIsWeaponArmed;
}

bool AWeaponBase::getIsMeleeWeapon()
{
	return true;
}

bool AWeaponBase::getIsRangedWeapon()
{
	return !getIsMeleeWeapon();
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


void AWeaponBase::UpdateWeapon()
{
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
		/*
		WeaponMeshSkeleton->AddRelativeLocation(WeaponData.MeshOffset);
		WeaponMeshSkeleton->AddRelativeRotation(WeaponData.MeshRotOffset);
		*/
	}
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


bool AWeaponBase::doAttack()
{
	if (!bIsWeaponArmed) { return false; }
	return true;
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
	DOREPLIFETIME_CONDITION(AWeaponBase, bIsWeaponArmed, COND_None);
    
}