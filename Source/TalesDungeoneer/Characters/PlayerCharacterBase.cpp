// Copyright Take Five Games, LLC 2023 - All rights reserved


#include "PlayerCharacterBase.h"

#include "Kismet/GameplayStatics.h"
#include "TalesDungeoneer/Gamemode/BaseFiles/TalesGameStateBase.h"
#include "TalesDungeoneer/Saves/SavedCharacters.h"


// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterTeam(ECharacterTeam::PLAYER);
	const AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GameStateBase);
	if (IsValid(TalesGameState))
	{
		const FString SaveSlotName = TalesGameState->GetSelectedCharacterSaveSlotName();
		USavedCharacter* SavedCharacter = Cast<USavedCharacter>(
			UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		
		if (IsValid(SavedCharacter))
			LoadSaveData(SaveSlotName, 0, SavedCharacter);
		
	}
	OnPlayerJoined.Broadcast();
}
