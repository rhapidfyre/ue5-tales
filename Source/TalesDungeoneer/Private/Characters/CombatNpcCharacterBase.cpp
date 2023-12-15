// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Characters/CombatNpcCharacterBase.h"

// Sets default values
ACombatNpcCharacterBase::ACombatNpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ACombatNpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACombatNpcCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	if (TimeBetweenAttacks[0] < 0.f)
		{ TimeBetweenAttacks[0] = 0.f; }
	if (TimeBetweenAbilities[0] < 0.f)
		{ TimeBetweenAbilities[0] = 0.f; }
	
	if (TimeBetweenAttacks[1] < TimeBetweenAttacks[0])
		{ TimeBetweenAttacks[1] = TimeBetweenAttacks[0]+0.01; }
	if (TimeBetweenAbilities[1] < TimeBetweenAbilities[0])
		{ TimeBetweenAbilities[1] =TimeBetweenAbilities[0]+0.01; }
}

void ACombatNpcCharacterBase::ProcessPrimaryAttack()
{
	if (_AttackTimer.IsValid())
	{
		_AttackTimer.Invalidate();
	}
}

void ACombatNpcCharacterBase::ProcessSecondaryAttack()
{
	if (_AttackTimer.IsValid())
	{
		_AttackTimer.Invalidate();
	}
}

// Called every frame
void ACombatNpcCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ACombatNpcCharacterBase::NpcCanAttemptAttack()
{
	return true;
}

bool ACombatNpcCharacterBase::NpcCanActivateAbility()
{
	return false;
}

