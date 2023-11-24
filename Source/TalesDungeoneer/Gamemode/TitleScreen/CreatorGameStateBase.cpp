// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "CreatorGameStateBase.h"

ACreatorGameStateBase::ACreatorGameStateBase() {}

void ACreatorGameStateBase::BeginPlay()
{
	Super::BeginPlay();
}


/**
 * @brief Turns the current player character into a new saved character slot
 * @param SaveResponse The return string stating the action of the outcome
 * @param RunAsync True for async, false for sync
 * @return True on success, false on failure
 */
bool ACreatorGameStateBase::CreateNewCharacter(FString& SaveResponse, bool RunAsync)
{
	
	if (CreateCharacterSaveIfNotExists())
	{
		if (SaveCurrentCharacter(SaveResponse, RunAsync))
		{
			return true;
		}
	}
	else
	{
		SaveResponse = "Name Already in Use";
	}
	return false;
}
