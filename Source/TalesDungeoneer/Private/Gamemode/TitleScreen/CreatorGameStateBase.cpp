// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gamemode/TitleScreen/CreatorGameStateBase.h"

#include "Characters/CreatorCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ACreatorGameStateBase::ACreatorGameStateBase() {}

void ACreatorGameStateBase::BeginPlay()
{
	Super::BeginPlay();
}

void ACreatorGameStateBase::SetIsCreatingCharacter(bool isCreating)
{
	bIsCreating = isCreating;

	ACharacter* PlayerCharacter			 = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	ACreatorCharacterBase* CharacterBase = Cast<ACreatorCharacterBase>(PlayerCharacter);
	if (IsValid(CharacterBase))
	{
		CharacterBase->SetIsBeingCreated(bIsCreating);
	}
	OnCharacterSelected.Broadcast(bIsCreating ? -1 : GetSelectedCharacter());
}


/**
 * @brief Turns the current player character into a new saved character slot
 * @param SaveResponse The return string stating the action of the outcome
 * @param RunAsync True for async, false for sync
 * @return True on success, false on failure
 */
bool ACreatorGameStateBase::CreateNewCharacter(FString& SaveResponse, bool RunAsync)
{
	ACharacter* PlayerCharacter		= UGameplayStatics::GetPlayerCharacter(GetWorld(), GetCharacterUserIndex());
	ACharacterBase* CharacterBase	= Cast<ACharacterBase>(PlayerCharacter);
	if (IsValid(CharacterBase))
	{
		// Run the actual save
		const FString SaveSlotName = CharacterBase->GetCharacterSafeName();
		if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			return SaveCurrentCharacter(SaveResponse, RunAsync);
		}
		SaveResponse = "Name Already in Use";
		return false;
	}
	SaveResponse = "Invalid Character Entity";
	return false;
}

void ACreatorGameStateBase::OnRep_IsCreating_Implementation()
{
	OnCharacterSelected.Broadcast(bIsCreating ? -1 : GetSelectedCharacter());
}

bool ACreatorGameStateBase::SaveCharacterSync(USaveGame*& SaveGame)
{
	if (Super::SaveCharacterSync(SaveGame))
	{
		USavedCharacter* savedCharacter = Cast<USavedCharacter>(SaveGame);
		if (IsValid(savedCharacter) && GetIsCreatingCharacter())
		{
			SetIsCreatingCharacter(false);
			return true;
		}
	}
	return false;
}


void ACreatorGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ACreatorGameStateBase, bIsCreating, COND_OwnerOnly);
}