// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "CreatorCharacterBase.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACreatorCharacterBase::LoadSaveData(const FString& SaveName, const int32 UserIndex, USaveGame* SaveData)
{
	Super::LoadSaveData(SaveName, UserIndex, SaveData);
	const USavedCharacter* CharacterData = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterData))
	{
		if (IsValid(MeshMergeComponent))
		{
			MeshMergeComponent->InitializeMeshMerge(CharacterData);
		}
		UE_LOG(LogTemp, Display, TEXT("LoadSaveData(): Successfully restored character from Save Slot '%s'"),
			*SaveName);
		CharacterRestoredFromSave(SaveName);
	}
}

void ACreatorCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(MeshMergeComponent))
		MeshMergeComponent->InitializeMeshMerge();
}
