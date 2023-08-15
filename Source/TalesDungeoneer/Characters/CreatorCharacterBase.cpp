// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "CreatorCharacterBase.h"

#include "TalesDungeoneer/Saves/SavedCharacters.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACreatorCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}	

void ACreatorCharacterBase::LoadSaveData(const FString& SaveName,
		const int32 UserIndex, USaveGame* SaveData)
{
	USavedCharacter* CharacterData = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterData))
	{
		SetCharacterName(CharacterData->CharacterName);
		SetCharacterLevel(CharacterData->CharacterLevel);
		SetCharacterRace(CharacterData->CharacterRace);
		SetCharacterClass(CharacterData->CharacterClass);
		SetExperiencePoints(CharacterData->ExperiencePoints);
	}
}