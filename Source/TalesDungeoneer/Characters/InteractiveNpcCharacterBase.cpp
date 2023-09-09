// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "InteractiveNpcCharacterBase.h"


// Sets default values
AInteractiveNpcCharacterBase::AInteractiveNpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AInteractiveNpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractiveNpcCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

