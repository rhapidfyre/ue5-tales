// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gamemode/TitleScreen/CreatorGameStateBase.h"

#include "Characters/CharacterBase.h"
#include "Kismet/GameplayStatics.h"

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
	ACharacterBase* CharacterBase = Cast<ACharacterBase>(
			UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if (IsValid(CharacterBase))
	{
		const FString SaveSlotName = CharacterBase->GetSafeCharacterName();
		if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			CharacterBase->SaveCharacterData();
		}
		else
		{
			SaveResponse = "Name Already in Use";
			return false;
		}
	}
	SaveResponse = "Invalid Character Entity";
	return false;
}
