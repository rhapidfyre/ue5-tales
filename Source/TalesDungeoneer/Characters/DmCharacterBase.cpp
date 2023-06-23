// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "DmCharacterBase.h"


// Sets default values
ADmCharacterBase::ADmCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADmCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}
