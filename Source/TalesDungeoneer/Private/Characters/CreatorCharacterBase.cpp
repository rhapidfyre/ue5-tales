// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "Characters/CreatorCharacterBase.h"

#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "Gamemode/TitleScreen/CreatorGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

bool ACreatorCharacterBase::CreateCharacter()
{
	ACreatorGameStateBase* TalesGameState =
			Cast<ACreatorGameStateBase>(GetWorld()->GetGameState());
	
	if (IsValid(TalesGameState))
	{
		if (!TalesGameState->GetIsCreatingCharacter())
		{
			UE_LOGFMT(LogTemp, Warning, "Creation of Character {NewName} Rejected - Not in Character Creator", GetCharacterName());
			return false;
		}
		
		NewSaveSlotName_	= GetCharacterSafeName();
		NewSaveUserIndex_	= 0; //TalesGameState->GetCurrentSaveUserIndex();
	
		// Allow creation as long as this character doesn't already exist
		if (UGameplayStatics::DoesSaveGameExist(NewSaveSlotName_, NewSaveUserIndex_))
		{
			UE_LOGFMT(LogTemp, Warning, "Creation of Character {NewName} Rejected - Already Exists", NewSaveSlotName_);
			NewSaveSlotName_	= "";
			NewSaveUserIndex_	= 0;
			return false;
		}

		// Do Initial Character Setup
		UE_LOGFMT(LogTemp, Display, "Attempting to create new character "
			"'{CharacterName}' in Save Slot: '{SaveName}, {UserIndex}'",
			GetCharacterName(), NewSaveSlotName_, NewSaveUserIndex_);

		FString SaveResponse = "";
		const bool saveSuccess = TalesGameState->CreateNewCharacter(SaveResponse, false);
		UE_LOGFMT(LogTemp, Display, "Creation of Character {NewName} Response: {SaveResponse}", GetCharacterName(), SaveResponse);
		return saveSuccess;
	}
	return false;
}

void ACreatorCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}
