// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "CreatorCharacterBase.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACreatorCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACreatorCharacterBase::CharacterRestoredFromSave(const FString SaveSlotName)
{
	Super::CharacterRestoredFromSave(SaveSlotName);
}	
