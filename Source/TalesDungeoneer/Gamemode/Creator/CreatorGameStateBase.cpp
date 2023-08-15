// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "CreatorGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "TalesDungeoneer/Saves/StaticSaveData.h"


ACreatorGameState::ACreatorGameState()
{
	
}

bool ACreatorGameState::SaveTheGame()
{
	CreateSaveGameIfNotExists();
	UGlobalSaveData* SavedMeta = Cast<UGlobalSaveData>(GetSaveGameMeta());
	if (!IsValid(SavedMeta))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveTheGame() FAILED: Could not retrieve save game meta object."));
		return false;
	}
	return UGameplayStatics::SaveGameToSlot(SavedMeta, _SaveMetaName, 0);
}

void ACreatorGameState::SetSaveGameMetaName(FString SaveSlotName)
{
	_SaveMetaName = SaveSlotName;
	SaveTheGameAsync();
}

void ACreatorGameState::SaveTheGameAsync()
{
	UGlobalSaveData* SavedMeta = Cast<UGlobalSaveData>(GetSaveGameMeta());
	if (!IsValid(SavedMeta))
	{
		if (!UGameplayStatics::DoesSaveGameExist(_SaveMetaName, 0))
		{
			SavedMeta = Cast<UGlobalSaveData>(
				UGameplayStatics::CreateSaveGameObject(UGlobalSaveData::StaticClass()));
			if (!IsValid(SavedMeta))
			{
				UE_LOG(LogTemp, Fatal, TEXT("Could not create new save game meta object."));
				return;
			}
		}
	}
	if (IsValid(SavedMeta))
	{
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &ACreatorGameState::SaveGameDelegate);

		// Do save game stuff here
		//
		//////////////////////////
		
		UE_LOG(LogTemp, Error, TEXT("Calling AsyncSaveGameToSlot()"));	
		UGameplayStatics::AsyncSaveGameToSlot(SavedMeta, _SaveMetaName, 0, SaveDelegate);
	}
}

USaveGame* ACreatorGameState::GetSaveGameMeta()
{
	USaveGame* SavedMeta = Cast<USaveGame>(
			UGameplayStatics::LoadGameFromSlot(_SaveMetaName,0));
	if (IsValid(SavedMeta))
		return SavedMeta;
	return nullptr;
}

void ACreatorGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ACreatorGameState::SaveGameMetaLoaded(const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	if (IsValid(LoadedGameData))
	{
		OnSaveGameObjectReady.Broadcast(LoadedGameData);
	}
}

void ACreatorGameState::SaveGameDelegate(const FString& SlotName,
	const int32 UserIndex, bool bSuccess)
{
	UE_LOG(LogTemp, Display, TEXT("SaveGameResponse()"));
}

bool ACreatorGameState::CreateSaveGameIfNotExists()
{
	if (!UGameplayStatics::DoesSaveGameExist(_SaveMetaName, 0))
	{
		UGlobalSaveData* NewSave = Cast<UGlobalSaveData>(
			UGameplayStatics::CreateSaveGameObject(UGlobalSaveData::StaticClass()));
		if (!IsValid(NewSave))
		{
			UE_LOG(LogTemp, Fatal, TEXT("Could not create new save game meta object."));
			return false;
		}
		return UGameplayStatics::SaveGameToSlot(NewSave, _SaveMetaName, 0);
	}
	// Already Exists
	return true;
}

void ACreatorGameState::LoadSaveGameMeta(FString SaveSlotName)
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ACreatorGameState::SaveGameMetaLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadedDelegate);
}
