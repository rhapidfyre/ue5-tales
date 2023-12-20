// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "Saves/SavedCharacters.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "../TalesDungeoneer.h"
#include "Characters/CharacterBase.h"
#include "Characters/PlayerCharacterBase.h"
#include "Gamemode/AdventureMode/TalesHudBase.h"
#include "lib/datastructures/GlobalData.h"


// Returns TRUE if the game state is any type of server
bool ATalesGameStateBase::CheckIsServer() const
{
	return GetNetMode() < NM_Client;
}

// Returns TRUE if the game state is server, but also a client
bool ATalesGameStateBase::CheckIsPlayableClient() const
{
	const ENetMode NetMode = GetNetMode();
	return NetMode == NM_ListenServer || NetMode == NM_Standalone;
}

FString ATalesGameStateBase::GenerateAlphanumeric(FString OptionalPath) const
{
	FString NewSaveSlotName = "";
	for (int i = 0; i < 18; i++)
	{
		TArray<int> RandValues = {
			FMath::RandRange(48,57), // Numbers 0-9
			FMath::RandRange(65,90) // Uppercase A-Z
		};
		const char RandChar = static_cast<char>(RandValues[FMath::RandRange(0,RandValues.Num()-1)]);
		NewSaveSlotName.AppendChar(RandChar);
	}
	return OptionalPath + NewSaveSlotName;
}

void ATalesGameStateBase::SetIsCreatingCharacter(bool isCreating)
{
	bIsCreating = isCreating;
}

ATalesGameStateBase::ATalesGameStateBase() {}

void ATalesGameStateBase::Server_NewNotification_Implementation(
	const FString& NewTitle, const FString& NewMessage, int NewPriority)
{
	Multicast_SendNotification(NewTitle, NewMessage, NewPriority);
}

void ATalesGameStateBase::LocalNotification(
			FString NewTitle, FString NewMessage, int NewPriority)
{
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (IsValid(LocalPlayer))
	{
		const APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld());
		if (IsValid(PlayerController))
		{
			ATalesHudBase* HudBase = Cast<ATalesHudBase>(PlayerController->GetHUD());
			if (IsValid(HudBase))
			{
				HudBase->NotifyHud(NewTitle, NewMessage, NewPriority);
			}
		}
	}
}

bool ATalesGameStateBase::SaveMetaData()
{
	if (!GetIsSaveMetaReady())
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
	
	return UGameplayStatics::SaveGameToSlot(SaveMeta, GetMetaDataSaveName(), 0);
}

void ATalesGameStateBase::SaveMetaDataAsync() const
{
	UGlobalSaveData* SavedMeta = Cast<UGlobalSaveData>(GetSaveGameMeta());
	if (!IsValid(SavedMeta))
	{
		if (!UGameplayStatics::DoesSaveGameExist(GetMetaDataSaveName(), 0))
		{
			SavedMeta = Cast<UGlobalSaveData>(
				UGameplayStatics::CreateSaveGameObject(UGlobalSaveData::StaticClass()));
			if (!IsValid(SavedMeta))
			{
				return;
			}
		}
	}
	if (IsValid(SavedMeta))
	{
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameDelegate);
		Helper_SetSaveValues(SavedMeta);
		UGameplayStatics::AsyncSaveGameToSlot(SavedMeta, GetMetaDataSaveName(), 0, SaveDelegate);
	}
}

void ATalesGameStateBase::RemoveSelectedCharacter()
{
	if (!CheckIsPlayableClient())
	{
		return;
	}

	if (SavedCharacters_.IsValidIndex( GetSelectedCharacter() ))
	{
		const FString SaveSlotName = GetCharacterSlotName();
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			const int DeleteIndex = SelectedCharacter_;
			SavedCharacters_.RemoveAt(DeleteIndex);
			SelectedCharacter_ = SavedCharacters_.Num() - 1;
			UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
			SaveMetaDataAsync(); // Update the save file
			OnCharacterDeleted.Broadcast(SaveSlotName, DeleteIndex);
		}
	}
}

// Performs save of the currently active character - Updates the save meta.
// Calls 'OnCharacterSaved' when finished
bool ATalesGameStateBase::SaveCharacterSync()
{
	
	APlayerCharacterBase* CharacterBase = Cast<APlayerCharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	if (!IsValid(CharacterBase))
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacter({sv}) FAILED: Could not retrieve CharacterBase.", HasAuthority()?"S":"C");
		return false;
	}
	
	const USavedCharacter* SaveObject = Cast<USavedCharacter>( CharacterBase->SaveCharacter() );
	if (IsValid(SaveObject))
	{
		if (GetIsCreatingCharacter())
		{
			// Create the new character
			const FString SaveSlotName = SaveObject->SaveSlotName;
			const int32 SaveUserIndex  = SaveObject->UserIndex;
			const int NewIndex = SavedCharacters_.Add( FSaveMeta(SaveSlotName,SaveUserIndex) );
			SetIsCreatingCharacter(false);
			SetSelectedCharacter(NewIndex);
		}
		return true;
	}
	return false;
}

void ATalesGameStateBase::ResetCharacter()
{
	// TODO
}

void ATalesGameStateBase::SaveCharacterAsync()
{
	SaveCharacterSync();
}

void ATalesGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ATalesGameStateBase, bIsCreating, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ATalesGameStateBase, CheatMode_, COND_OwnerOnly);
	
	DOREPLIFETIME(ATalesGameStateBase, DungeonLevel);
}

void ATalesGameStateBase::Multicast_SendNotification_Implementation(
	const FString& NewTitle, const FString& NewMessage, int NewPriority)
{
	
}

USaveGame* ATalesGameStateBase::GetSaveGameMeta() const
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		return nullptr;
	}
	USaveGame* SavedMeta = Cast<USaveGame>(
			UGameplayStatics::LoadGameFromSlot(GetMetaDataSaveName(),0));
	if (IsValid(SavedMeta))
	{
		return SavedMeta;
	}
	return nullptr;
}

TArray<FSaveMeta> ATalesGameStateBase::GetSavedCharacterSlotNames() const
{
	return SavedCharacters_;
}

int ATalesGameStateBase::GetIndexOfSavedCharacter(const FString& SlotName, const int32& UserIndex) const
{
	if (!SlotName.IsEmpty())
	{
		for (int i = 0; i < SavedCharacters_.Num(); i++)
		{
			if (   SavedCharacters_[i].SaveName  == SlotName
				&& SavedCharacters_[i].SaveIndex == UserIndex )
			{
				return i;
			}
		}
	}
	return -1;
}

USavedCharacter* ATalesGameStateBase::GetSavedCharacterData(FString SaveSlotName)
{
	if (!CheckIsPlayableClient())
	{
		return nullptr;
	}
	if (!SaveSlotName.IsEmpty())
	{
		USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
		USavedCharacter* SaveData = Cast<USavedCharacter>(SaveGame);
		return SaveData;
	}
	return nullptr;
}

FString ATalesGameStateBase::GetCharacterSlotName() const
{
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_))
	{
		return SavedCharacters_[SelectedCharacter_].SaveName;
	}
	return "";
}

int ATalesGameStateBase::GetCharacterUserIndex() const
{
	if (GetIsCreatingCharacter())
	{
		return 0;
	}
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_))
	{
		return SavedCharacters_[SelectedCharacter_].SaveIndex;
	}
	return 0;
}

bool ATalesGameStateBase::GetIsValidCharacterSelected() const
{
	if (GetIsCreatingCharacter()) { return false; }
	return SavedCharacters_.IsValidIndex(GetSelectedCharacter());
}

bool ATalesGameStateBase::GetDoesCharacterSaveExist() const
{
	if ( GetIsValidCharacterSelected() )
	{
		if ( !GetCharacterSlotName().IsEmpty() )
		{
			return UGameplayStatics::DoesSaveGameExist(
				GetCharacterSlotName(), GetCharacterUserIndex());
		}
	}
	return false;
}

void ATalesGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// Create the save if it doesn't exist
	SaveMetaName_ = UGlobalData::GetAppVersion(true, true);
	if ( GetMetaDataSaveName().IsEmpty() ) {SaveMetaName_ = "SaveMeta"; }
	CreateSaveGameIfNotExists();
	
	// Load the save game data from the previous session
	LoadSaveGameMeta( HasAuthority() );
	OnSaveGameObjectReady.Broadcast();
}

bool ATalesGameStateBase::SaveCurrentCharacter(FString& SaveResponse, bool RunAsync)
{
	if ((HasAuthority() && !bSavesOnServer) || (!HasAuthority() && bSavesOnServer))
	{
		const ENetMode netMode = GetNetMode();
		if (netMode != NM_Standalone && netMode != NM_ListenServer)
		{
			UE_LOGFMT(LogTales, Warning, "SaveTheSaveCurrentCharacterGame(S) FAILED: Authority Violation");
			SaveResponse = "Authority Violation";
			return false;
		}
	}
	
	if (!GetIsSaveMetaReady())
	{
		SaveResponse = "SaveMeta isn't ready yet.";
		UE_LOGFMT(LogTales, Warning, "SaveTheSaveCurrentCharacterGame(S) FAILED: SaveMeta is not ready yet.");
		return false;
	}
	
	FString SaveSlotName;
	const APlayerCharacterBase* CharacterBase = Cast<APlayerCharacterBase>(
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
	}
	else
	{
		SaveResponse = "Player Character not found / invalid";
		return false;
	}

	// Save asynchronously
	if (RunAsync)
	{
		SaveCharacterAsync();
		SaveResponse = "Saving Character Asynchronously";
		return true;
	}
	
	if (SaveCharacterSync())
	{
		if (SaveMetaData())
		{
			SaveResponse = "Synchronous Save Successful";
			return true;
		}
		SaveResponse = "Synchronous Save FAILED";
	}
	else
	{
		SaveResponse = "SaveCharacterSync returned FALSE";
	}

	return false;
}

/**
 * @brief Loads the save game meta file into memory
 * @param LoadAsync True if loading can be run asynchronously
 * @return Returns true if ran async or file was loaded successfully.
 */
bool ATalesGameStateBase::LoadSaveGameMeta(bool LoadAsync)
{
	if (GetMetaDataSaveName().IsEmpty())
	{
		return false;
	}

	if (LoadAsync)
	{
		LoadSaveGameMetaAsync();
		return true;
	}
	return LoadSaveGameMetaSync();
}

void ATalesGameStateBase::CharacterSaveLoaded(
		const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	if (IsValid(CharacterBase))
	{
		CharacterBase->LoadCharacter(
			GetCharacterSlotName(), GetCharacterUserIndex(), LoadedGameData);
	}
}

void ATalesGameStateBase::SaveGameMetaLoaded(
		const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	const UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>( LoadedGameData );
	if (!IsValid(SaveMeta))
	{
		CreateSaveGameIfNotExists();
		SaveMeta = Cast<UGlobalSaveData>( GetSaveGameMeta() );
	}
		
	bSaveMetaIsReady = IsValid(SaveMeta);
	if (GetIsSaveMetaReady())
	{
		SelectedCharacter_	= SaveMeta->GetSelectedCharacterIndex();
		SavedCharacters_	= SaveMeta->GetAllCharacterSaves();
		
		OnSaveGameObjectReady.Broadcast();

		// If a valid character is selected, attempt to load it
		if (GetIsValidCharacterSelected() && GetNetMode() != NM_DedicatedServer)
		{
			UE_LOGFMT(LogGameState, Display, "Attempting to restore Character "
				"from save game '{SaveSlot} ({SlotIndex})", GetCharacterSlotName(), GetCharacterUserIndex());
			
			APlayerCharacterBase* PlayerCharacter = Cast<APlayerCharacterBase>
					( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
			
			if (IsValid(PlayerCharacter))
				{ PlayerCharacter->LoadCharacter(); }
		}
		
		else
		{
			if (GetIsValidCharacterSelected())
			{
				UE_LOGFMT(LogGameState, Display, "Executable is a Dedicated Server.");
			}
			else
			{
				UE_LOGFMT(LogGameState, Display, "No Character Exists to Restore");
			}
		}
	}
}

void ATalesGameStateBase::SetSelectedCharacter(int CharacterIndex)
{
	if (GetIsCreatingCharacter())
	{
		return;
	}
	
	if (SavedCharacters_.IsValidIndex(CharacterIndex))
	{
		SelectedCharacter_ = CharacterIndex;
		if (!LoadCharacterSync( GetCharacterSlotName() ))
		{
			SelectedCharacter_ = -1;
		}
	}
	else
	{
		SelectedCharacter_ = -1;
		ResetCharacter();
	}
	
	SaveMetaData();
	OnCharacterSelected.Broadcast(SelectedCharacter_);
}

int ATalesGameStateBase::GetSelectedCharacter() const
{
	return SelectedCharacter_;
}

int ATalesGameStateBase::GetNextCharacterIndex()
{
	if (GetIsCreatingCharacter()) { return -1; }
	
	int nextIndex = SelectedCharacter_ + 1;
	if (!SavedCharacters_.IsValidIndex(nextIndex))
	{
		// If the first character is valid, set index to it
		if (SavedCharacters_.IsValidIndex(0)) { nextIndex = 0; }
		
		// Otherwise, there are no characters to select
		else { nextIndex = -1; }
	}
	
	if (nextIndex != GetSelectedCharacter())
	{
		SetSelectedCharacter(nextIndex);
	}
	return GetSelectedCharacter();
}

int ATalesGameStateBase::GetPrevCharacterIndex()
{
	if (GetIsCreatingCharacter())
	{
		return -1;
	}
	int lastIndex = SelectedCharacter_ - 1;
	if (!SavedCharacters_.IsValidIndex(lastIndex))
	{
		lastIndex = SavedCharacters_.Num() - 1;
		// If the last index isn't valid, there are no characters to select
		if (!SavedCharacters_.IsValidIndex(lastIndex)) { lastIndex = -1; }
	}
	
	if (lastIndex != GetSelectedCharacter())
	{
		SetSelectedCharacter(lastIndex);
	}
	return GetSelectedCharacter();
}

int ATalesGameStateBase::GetLastCharacterIndex() const
{
	return SavedCharacters_.Num() - 1;
}

void ATalesGameStateBase::Helper_SetSaveValues(UGlobalSaveData* SaveMeta) const
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("Helper_SetSaveValues() FAILED: Saves are clientside."));
		return;
	}
	if (IsValid(SaveMeta))
	{
		SaveMeta->SetSavedCharacterNameList(SavedCharacters_);
		SaveMeta->SetSelectedCharacter(SelectedCharacter_);
	}
}

void ATalesGameStateBase::Helper_LoadSavedValues(const UGlobalSaveData* SaveMeta)
{
	if (IsValid(SaveMeta))
	{
		SavedCharacters_	= SaveMeta->GetAllCharacterSaves();
		SelectedCharacter_	= SaveMeta->GetSelectedCharacterIndex();
		OnCharacterSelected.Broadcast(SelectedCharacter_);
	}
}

void ATalesGameStateBase::SaveGameDelegate(
		const FString& SlotName, const int32 UserIndex, bool bSuccess) const
{
	OnGameSaved.Broadcast(bSuccess);
}

void ATalesGameStateBase::OnRep_SelectedCharacter_Implementation(const int OldSelection)
{
	OnCharacterSelected.Broadcast(SelectedCharacter_);
}

void ATalesGameStateBase::OnRep_CheatMode_Implementation(bool OldState)
{
	if (CheatMode_ != OldState)
	{
		LocalNotification("Cheat Mode",
			GetIsCheatModeEnabled() ?
				"Cheat Mode has been ENABLED"
				: "Cheat Mode has been DISABLED", 1);
	}
}

bool ATalesGameStateBase::CreateSaveGameIfNotExists()
{
	if (SaveMetaName_.IsEmpty()) { SaveMetaName_ = "SaveMeta"; }
	if (!UGameplayStatics::DoesSaveGameExist(GetMetaDataSaveName(), GetMetaDataSaveIndex()))
	{
		UGlobalSaveData* NewSave = Cast<UGlobalSaveData>(
			UGameplayStatics::CreateSaveGameObject(UGlobalSaveData::StaticClass()));
		
		if (!IsValid(NewSave))
		{
			UE_LOGFMT(LogGameState, Fatal, "Failed to create metadata");
			return false;
		}
		
		bSaveMetaIsReady = UGameplayStatics::SaveGameToSlot(
			NewSave, GetMetaDataSaveName(), GetMetaDataSaveIndex());
		
		UE_LOGFMT(LogGameState, Display, "Created MetaData");
		return GetIsSaveMetaReady();
	}
	// Already Exists
	return false;
}

bool ATalesGameStateBase::LoadCharacterSync(const FString& SaveSlotName)
{
	if (GetIsCreatingCharacter())
	{
		return false;
	}
	
	USavedCharacter* SavedCharacter = GetSavedCharacterData(SaveSlotName);
	if (IsValid(SavedCharacter))
	{
		// Get the player character entity for *this* player
		ACharacterBase* CharacterBase = Cast<ACharacterBase>
				( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
		
		if (IsValid(CharacterBase))
		{
			return CharacterBase->LoadCharacter(GetCharacterSlotName(),
												GetCharacterUserIndex(),
												SavedCharacter);
		}
	}
	return false;
}

void ATalesGameStateBase::LoadCharacterAsync(const FString& SaveSlotName)
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::CharacterSaveLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadedDelegate);
}

bool ATalesGameStateBase::LoadSaveGameMetaSync()
{
	USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(GetMetaDataSaveName(), 0);
	if (IsValid(SaveGame))
	{
		const UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>(SaveGame);
		if (IsValid(SaveMeta))
		{
			SaveGameMetaLoaded(GetMetaDataSaveName(), 0, SaveGame);
			return true;
		}
	}
	return false;
}

void ATalesGameStateBase::LoadSaveGameMetaAsync()
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameMetaLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(GetMetaDataSaveName(), 0, LoadedDelegate);
}
