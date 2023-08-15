// Copyright Take Five Games, LLC 2023 - All rights reserved


#include "PlayerCharacterBase.h"

#include "TalesDungeoneer/Saves/SavedCharacters.h"


// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

void APlayerCharacterBase::LoadSaveData(const FString& SaveName,
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

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterTeam(ECharacterTeam::PLAYER);
	OnPlayerJoined.Broadcast();
}
