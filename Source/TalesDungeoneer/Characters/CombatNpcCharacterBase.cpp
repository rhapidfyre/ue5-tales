// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "CombatNpcCharacterBase.h"
#include "TalesDungeoneer/Weapons/WeaponBase.h"

// Sets default values
ACombatNpcCharacterBase::ACombatNpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ACombatNpcCharacterBase::PerformAttack(EWeaponSlots WeaponSlot)
{
	if (NpcCanAttemptAttack())
	{
		Super::PerformAttack(WeaponSlot);
	}
}

// Called when the game starts or when spawned
void ACombatNpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACombatNpcCharacterBase::ProcessPrimaryAttack()
{
	if (_AttackTimer.IsValid())
		_AttackTimer.Invalidate();
	PrimaryAttack();
}

void ACombatNpcCharacterBase::ProcessSecondaryAttack()
{
	if (_AttackTimer.IsValid())
		_AttackTimer.Invalidate();
	SecondaryAttack();
}

// Called every frame
void ACombatNpcCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ACombatNpcCharacterBase::NpcCanAttemptAttack()
{
	if (!_AttackTimer.IsValid())
	{
		
		if (IsValid(WeaponComponent->GetWeaponInSlot(EWeaponSlots::PRIMARY)))
			GetWorld()->GetTimerManager().SetTimer(_AttackTimer, this,
				&ACombatNpcCharacterBase::ProcessPrimaryAttack,
				FMath::RandRange(TimeBetweenAttacks[0], TimeBetweenAttacks[1]), false);
		
		else if (IsValid(WeaponComponent->GetWeaponInSlot(EWeaponSlots::SECONDARY)))
			GetWorld()->GetTimerManager().SetTimer(_AttackTimer, this,
				&ACombatNpcCharacterBase::ProcessSecondaryAttack,
				FMath::RandRange(TimeBetweenAttacks[0], TimeBetweenAttacks[1]), false);
		
	}
	return false;
}

bool ACombatNpcCharacterBase::NpcCanActivateAbility()
{
	return false;
}

