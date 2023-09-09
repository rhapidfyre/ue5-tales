// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "CombatNpcCharacterBase.h"


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

// Called every frame
void ACombatNpcCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

