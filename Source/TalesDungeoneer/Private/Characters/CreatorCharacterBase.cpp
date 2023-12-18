// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "Characters/CreatorCharacterBase.h"

#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

bool ACreatorCharacterBase::CreateCharacter()
{
	ATalesGameStateBase* TalesGameState =
			Cast<ATalesGameStateBase>(GetWorld()->GetGameState());
	
	if (IsValid(TalesGameState))
	{
		if (!TalesGameState->GetIsCreatingCharacter())
		{
			UE_LOGFMT(LogTemp, Warning, "Creation of Character {NewName} Rejected - Not in Character Creator", GetCharacterName());
			return false;
		}
		
		NewSaveSlotName_	= GetSafeCharacterName();
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
		UE_LOGFMT(LogTemp, Warning, "Attempting to create new character "
			"'{CharacterName}' in Save Slot: '{SaveName}, {UserIndex}'",
			GetCharacterName(), NewSaveSlotName_, NewSaveUserIndex_);

		// If the new save is successful, set it as the selected character
		FString SaveResponse = "";
		if (TalesGameState->SaveCurrentCharacter(SaveResponse, false))
		{
			const int saveIndex = TalesGameState->GetIndexOfSavedCharacter(NewSaveSlotName_, NewSaveUserIndex_);
			if (saveIndex >= 0)
			{
				TalesGameState->SetSelectedCharacter(saveIndex);
			}
			return true;
		}
		UE_LOGFMT(LogTemp, Error, "Creation of Character {NewName} FAILED - Reason: {SaveResponse}", SaveResponse);
	}
	return false;
}