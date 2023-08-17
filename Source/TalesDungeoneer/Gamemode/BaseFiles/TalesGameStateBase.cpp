// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "TalesGameStateBase.h"
#include "TalesDungeoneer/Saves/SavedCharacters.h"

#include "Kismet/GameplayStatics.h"


FString GetCleanedSaveSlotString(FString UncleanedString)
{
	return UncleanedString.Replace(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase);	
}


ATalesGameStateBase::ATalesGameStateBase() {}

/**
 * @brief Sets the new save game meta file name,
 *			creating it if it does not exist.
 * @param SaveSlotName The name of the new save meta file
 */
void ATalesGameStateBase::SetSaveGameMetaName(FString SaveSlotName)
{
	_SaveMetaName = SaveSlotName;
	CreateSaveGameIfNotExists();
}


bool ATalesGameStateBase::SaveMetaData()
{
	if (!bSaveMetaIsReady)
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveTheGame() FAILED: SaveMeta isn't ready yet."));
		return false;
	}
	CreateSaveGameIfNotExists();
	
	// Previous Copy of the data without the new changes
	UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>(GetSaveGameMeta());
	if (!IsValid(SaveMeta))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveTheGame() FAILED: Could not retrieve save game meta object."));
		return false;
	}
	
	// Update save file with new data
	Helper_SetSaveValues(SaveMeta);
	
	return UGameplayStatics::SaveGameToSlot(SaveMeta, _SaveMetaName, 0);
}

void ATalesGameStateBase::SaveMetaDataAsync()
{
	UGlobalSaveData* SavedMeta = Cast<UGlobalSaveData>(GetSaveGameMeta());
	if (!IsValid(SavedMeta))
	{
		if (!UGameplayStatics::DoesSaveGameExist(_SaveMetaName, 0))
		{
			SavedMeta = Cast<UGlobalSaveData>(
				UGameplayStatics::CreateSaveGameObject(UGlobalSaveData::StaticClass()));
			if (!IsValid(SavedMeta))
				return;
		}
	}
	if (IsValid(SavedMeta))
	{
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameDelegate);
		Helper_SetSaveValues(SavedMeta);
		UGameplayStatics::AsyncSaveGameToSlot(SavedMeta, _SaveMetaName, 0, SaveDelegate);
	}
}

bool ATalesGameStateBase::SaveCharacter(const FString SaveSlotName)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	if (!IsValid(CharacterBase))
		return false;
	
	if (SaveSlotName.IsEmpty())
		return false;

	CreateCharacterSaveIfNotExists();
	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(GetSavedCharacterData(SaveSlotName));
	if (!IsValid(SavedCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveCharacter() FAILED: Could not retrieve save game object."));
		return false;
	}

	Helper_SetCharacterValues(CharacterBase, SavedCharacter);
	return UGameplayStatics::SaveGameToSlot(SavedCharacter, SaveSlotName, 0);
}

void ATalesGameStateBase::SaveCharacterAsync(const FString SaveSlotName)
{
	ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	if (!IsValid(CharacterBase))
		return;
	
	if (SaveSlotName.IsEmpty())
		return;

	CreateCharacterSaveIfNotExists();

	USavedCharacter* SavedCharacter = GetSavedCharacterData(SaveSlotName);
	if (IsValid(SavedCharacter))
	{
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &ATalesGameStateBase::SaveCharacterDelegate);
		Helper_SetCharacterValues(CharacterBase, SavedCharacter);
		UGameplayStatics::AsyncSaveGameToSlot(SavedCharacter, SaveSlotName, 0, SaveDelegate);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SaveCharacter() FAILED: Could not retrieve save game object."));	
	}
}

USaveGame* ATalesGameStateBase::GetSaveGameMeta() const
{
	USaveGame* SavedMeta = Cast<USaveGame>(
			UGameplayStatics::LoadGameFromSlot(_SaveMetaName,0));
	if (IsValid(SavedMeta))
		return SavedMeta;
	return nullptr;
}

void ATalesGameStateBase::LoadSaveGameMetaAsync()
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameMetaLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(_SaveMetaName, 0, LoadedDelegate);
}

TArray<FString> ATalesGameStateBase::GetSavedCharacterSlotNames() const
{
	return _SavedCharacters;
}

void ATalesGameStateBase::GetSavedCharacterDataAsync(FString SaveSlotName, APlayerCharacterBase* PlayerCharacter)
{
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(PlayerCharacter, &APlayerCharacterBase::LoadSaveData);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadDelegate);
}

USavedCharacter* ATalesGameStateBase::GetSavedCharacterData(FString SaveSlotName)
{
	if (!SaveSlotName.IsEmpty())
	{
		return Cast<USavedCharacter>(
			UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)
			);
	}
	return nullptr;
}

FString ATalesGameStateBase::GetSelectedCharacterSlotSaveName() const
{
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter))
		return _SavedCharacters[_SelectedCharacter];
	return "";
}

int ATalesGameStateBase::GetSelectedCharacterIndex() const
{
	return _SelectedCharacter;
}

void ATalesGameStateBase::SetSavedCharacterNameList(TArray<FString> RestoredCharacters)
{
	_SavedCharacters = RestoredCharacters;
}

void ATalesGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// Create the save if it doesn't exist
	if (!_SaveMetaName.IsEmpty())
		CreateSaveGameIfNotExists();

	// Load the save game data from the previous session
	LoadSaveGameMetaAsync();
	
}

bool ATalesGameStateBase::SaveCurrentCharacter(FString& SaveResponse, bool RunAsync)
{
	SaveResponse = "Saving Character Failed - Reason Unknown"; 
	if (!bSaveMetaIsReady)
	{
		SaveResponse = "SaveMeta isn't ready yet.";
		UE_LOG(LogTemp, Warning, TEXT("SaveTheGame() FAILED: SaveMeta isn't ready yet."));
		return false;
	}
	
	FString SaveSlotName;
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>(
			UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Determine the name of the save slot
	if (IsValid(CharacterBase))
	{
		const FString TempSlotName = CharacterBase->GetCharacterName();
		if (TempSlotName.IsEmpty())
		{
			SaveResponse = "Character Name Invalid";
			return false;
		}
		SaveSlotName = GetCleanedSaveSlotString(TempSlotName);
	}
	else
	{
		SaveResponse = "Player Character not found / invalid";
		return false;
	}

	// Save asynchronously
	if (RunAsync)
	{
		SaveCharacterAsync(SaveSlotName);
		SaveResponse = "Saving Character Asynchronously";
		return true;
	}

	// If not running async, or async fails, run sync
	if (SaveCharacter(SaveSlotName))
	{
		SaveResponse = "Synchronous Save Successful";
		return true;
	}
	
	return false;
}

void ATalesGameStateBase::SaveGameMetaLoaded(
		const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	if (IsValid(LoadedGameData))
	{
		const UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>( LoadedGameData );
		bSaveMetaIsReady = IsValid(SaveMeta);
		if (bSaveMetaIsReady)
		{
			Helper_LoadSavedValues(SaveMeta);
			OnSaveGameObjectReady.Broadcast();
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("SaveGameMetaLoaded(): Save Slot '%s' Not found."),
								*SlotName);
}

void ATalesGameStateBase::Helper_SetSaveValues(UGlobalSaveData* SaveMeta)
{
	if (IsValid(SaveMeta))
	{
		SaveMeta->SetSavedCharacterNameList(_SavedCharacters);
		SaveMeta->SetSelectedCharacter(_SelectedCharacter);
	}
}

void ATalesGameStateBase::Helper_LoadSavedValues(const UGlobalSaveData* SaveMeta)
{
	if (IsValid(SaveMeta))
	{
		_SavedCharacters	= SaveMeta->GetAllCharacterSaves();
		_SelectedCharacter	= SaveMeta->GetSelectedCharacterIndex();
	}
}

void ATalesGameStateBase::Helper_SetCharacterValues(
	const ACharacterBase* CharacterBase, USaveGame* SaveData)
{
	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(SaveData);
	if (IsValid(SavedCharacter))
	{
		SavedCharacter->CharacterName  = CharacterBase->GetCharacterName();
		SavedCharacter->CharacterLevel = CharacterBase->GetCharacterLevel();
		SavedCharacter->CharacterClass = CharacterBase->GetCharacterClass();
		SavedCharacter->CharacterRace  = CharacterBase->GetCharacterRace();

		// Save the version of the game when this character was saved
		SavedCharacter->SaveVersion    = UGlobalData::GetAppVersion();
	}
}

void ATalesGameStateBase::Helper_LoadCharacterValues(const FString SaveSlotName)
{
	ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	USavedCharacter* SavedCharacter = GetSavedCharacterData(SaveSlotName);
	
	if (IsValid(SavedCharacter))
	{
		CharacterBase->SetCharacterName(  SavedCharacter->CharacterName  );
		CharacterBase->SetCharacterLevel( SavedCharacter->CharacterLevel );
		CharacterBase->SetCharacterClass( SavedCharacter->CharacterClass );
		CharacterBase->SetCharacterRace(  SavedCharacter->CharacterRace  );

		// Save the version of the game when this character was saved
		SavedCharacter->SaveVersion    = UGlobalData::GetAppVersion();
	}
}

void ATalesGameStateBase::SaveGameDelegate(
		const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	OnGameSaved.Broadcast(bSuccess);
}

void ATalesGameStateBase::SaveCharacterDelegate(
		const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	OnCharacterSaved.Broadcast(bSuccess);
}

bool ATalesGameStateBase::CreateSaveGameIfNotExists()
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
		bSaveMetaIsReady = UGameplayStatics::SaveGameToSlot(NewSave, _SaveMetaName, 0);
		return bSaveMetaIsReady;
	}
	// Already Exists
	return true;
}

bool ATalesGameStateBase::CreateCharacterSaveIfNotExists()
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	if (IsValid(CharacterBase))
	{
		const FString SaveSlotName = GetCleanedSaveSlotString(CharacterBase->GetCharacterName());
		if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			UGlobalSaveData* NewSave = Cast<UGlobalSaveData>(
				UGameplayStatics::CreateSaveGameObject(UGlobalSaveData::StaticClass()));
			if (!IsValid(NewSave))
			{
				UE_LOG(LogTemp, Fatal, TEXT("Could not create new save game meta object."));
				return false;
			}
			return UGameplayStatics::SaveGameToSlot(NewSave, SaveSlotName, 0);
		}
		return true;
	}
	return false;
}
