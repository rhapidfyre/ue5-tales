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

void ATalesGameStateBase::SetIsCreatingCharacter(bool isCreating)
{
	bIsCreating = isCreating;
}

ATalesGameStateBase::ATalesGameStateBase() {}

UDataTable* ATalesGameStateBase::GetNpcDataTable()
{
	UDataTable* dataTable = NpcDataTable;
	if (IsValid(dataTable))
		return dataTable;
	return nullptr;
}

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

/**
 * @brief Sets the new save game meta file name,
 *			creating it if it does not exist.
 * @param SaveSlotName The name of the new save meta file
 */
void ATalesGameStateBase::SetSaveGameMetaName(FString SaveSlotName)
{
	SaveMetaName_ = SaveSlotName;
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
	
	return UGameplayStatics::SaveGameToSlot(SaveMeta, SaveMetaName_, 0);
}

void ATalesGameStateBase::SaveMetaDataAsync() const
{
	UGlobalSaveData* SavedMeta = Cast<UGlobalSaveData>(GetSaveGameMeta());
	if (!IsValid(SavedMeta))
	{
		if (!UGameplayStatics::DoesSaveGameExist(SaveMetaName_, 0))
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
		UGameplayStatics::AsyncSaveGameToSlot(SavedMeta, SaveMetaName_, 0, SaveDelegate);
	}
}

void ATalesGameStateBase::RemoveSelectedCharacter()
{
	if (!CheckIsPlayableClient()) return;
	
	if (SavedCharacters_.IsValidIndex( GetSelectedCharacterIndex() ))
	{
		const FString SaveSlotName = GetSelectedCharacterSaveSlotName();
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

bool ATalesGameStateBase::SaveCharacterSync(const FString& SaveSlotName)
{	
	if (SaveSlotName.IsEmpty())
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacter({sv}) FAILED: Invalid SaveSlotName (EMPTY)", HasAuthority()?"S":"C");
		return false;
	}
	
	ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	if (!IsValid(CharacterBase))
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacter({sv}) FAILED: Could not retrieve CharacterBase.", HasAuthority()?"S":"C");
		return false;
	}

	UE_LOGFMT(LogTales, Log, "SaveCharacter({sv}): Saving Character '{CharacterName}'",
		HasAuthority()?"S":"C", CharacterBase->GetCharacterName());
	
	if (!SavedCharacters_.Contains(SaveSlotName))
	{
		SavedCharacters_.Add(SaveSlotName);
	}
	
	return CharacterBase->SaveCharacterData();
}

void ATalesGameStateBase::SaveCharacterAsync(const FString& SaveSlotName)
{
	SaveCharacterSync(SaveSlotName);
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
			UGameplayStatics::LoadGameFromSlot(SaveMetaName_,0));
	if (IsValid(SavedMeta))
		return SavedMeta;
	return nullptr;
}

TArray<FString> ATalesGameStateBase::GetSavedCharacterSlotNames() const
{
	return SavedCharacters_;
}

int ATalesGameStateBase::GetIndexOfSavedCharacter(const FString& SaveSlotName, int32 UserSaveIndex) const
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserSaveIndex))
	{
		for (int i = 0; i < SavedCharacters_.Num(); i++)
		{
			if (SavedCharacters_[i] == SaveSlotName)
			{
				return i;
			}
		}
	}
	return -1;
}

void ATalesGameStateBase::GetSavedCharacterDataAsync(FString SaveSlotName, ACharacterBase* PlayerCharacter)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		return;
	}
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(PlayerCharacter, &ACharacterBase::LoadCharacterData);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadDelegate);
}

USavedCharacter* ATalesGameStateBase::GetSavedCharacterData(FString SaveSlotName)
{
	if (!CheckIsPlayableClient()) return nullptr;
	if (!SaveSlotName.IsEmpty())
	{
		USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
		USavedCharacter* SaveData = Cast<USavedCharacter>(SaveGame);
		return SaveData;
	}
	return nullptr;
}

FString ATalesGameStateBase::GetSelectedCharacterSaveSlotName() const
{
	if (GetIsCreatingCharacter())
	{
		return "";
	}
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_))
	{
		return SavedCharacters_[SelectedCharacter_];
	}
	return "";
}

int ATalesGameStateBase::GetSelectedCharacterIndex() const
{
	if (GetIsCreatingCharacter())
	{
		return -1;
	}
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_))
	{
		return SelectedCharacter_;
	}
	return -1;
}

bool ATalesGameStateBase::GetDoesCharacterSaveExist() const
{
	if (GetIsCreatingCharacter())
	{
		return false;
	}
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_))
	{
		if (SavedCharacters_[SelectedCharacter_] != "")
		{
			return UGameplayStatics::DoesSaveGameExist(SavedCharacters_[SelectedCharacter_], 0);
		}
	}
	return false;
}

void ATalesGameStateBase::SetSavedCharacterNameList(TArray<FString> RestoredCharacters)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		return;
	}
	SavedCharacters_ = RestoredCharacters;
}

void ATalesGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// Create the save if it doesn't exist
	if ( !SaveMetaName_.IsEmpty() )
	{
		CreateSaveGameIfNotExists();
	}

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
	
	if (!bSaveMetaIsReady)
	{
		SaveResponse = "SaveMeta isn't ready yet.";
		UE_LOGFMT(LogTales, Warning, "SaveTheSaveCurrentCharacterGame(S) FAILED: SaveMeta is not ready yet.");
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
		SaveSlotName = CharacterBase->GetSafeCharacterName();
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
	if (SaveCharacterSync(SaveSlotName))
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
	if (SaveMetaName_.IsEmpty())
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
		CharacterBase->LoadCharacterData(SlotName, UserIndex, LoadedGameData);
	}
}

void ATalesGameStateBase::SaveGameMetaLoaded(
		const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	const UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>( LoadedGameData );
	if (!IsValid(LoadedGameData))
	{
		UE_LOG(LogTemp, Display, TEXT("SaveGameMetaLoaded(): Save Slot '%s' Not found. Creating..."),
									*SlotName);
		CreateSaveGameIfNotExists();
		SaveMeta = Cast<UGlobalSaveData>( GetSaveGameMeta() );
	}
		
	bSaveMetaIsReady = IsValid(SaveMeta);
	if (bSaveMetaIsReady)
	{
		OnSaveGameObjectReady.Broadcast();
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("SaveGameMetaLoaded(): Save Slot '%s' Not found."),
								*SlotName);
}

void ATalesGameStateBase::SetSelectedCharacter(int CharacterIndex)
{
	if (GetIsCreatingCharacter())
	{
		return;
	}
	
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter( GetWorld(), 0);
	
	if (SavedCharacters_.IsValidIndex(CharacterIndex))
	{
		SelectedCharacter_ = CharacterIndex;
		LoadCharacterAsync( GetSelectedCharacterSaveSlotName() );
	}
	else
	{
		SelectedCharacter_ = -1;
	}
	
	SaveMetaData();
	OnCharacterSelected.Broadcast(
		GetSelectedCharacterSaveSlotName(),
		GetSelectedCharacterIndex());
}

int ATalesGameStateBase::GetNextCharacterIndex()
{
	if (GetIsCreatingCharacter())
	{
		return -1;
	}
	
	// Increment to the next index
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_ + 1))
	{
		++SelectedCharacter_;
		SetSelectedCharacter(SelectedCharacter_);
		return SelectedCharacter_;
	}
	
	// Get first index
	if (SavedCharacters_.Num() > 0)
	{
		SetSelectedCharacter(0);
		return SelectedCharacter_;
	}

	// Return Invalid
	return -1;
}

int ATalesGameStateBase::GetPrevCharacterIndex()
{
	if (GetIsCreatingCharacter())
	{
		return -1;
	}
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_ - 1))
	{
		SelectedCharacter_--;
		SetSelectedCharacter(SelectedCharacter_);
		return SelectedCharacter_;
	}
	if (SavedCharacters_.Num() > 0)
	{
		SetSelectedCharacter( GetLastCharacterIndex() );
		return SelectedCharacter_;
	}
	return -1;
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
		OnCharacterSelected.Broadcast(
			GetSelectedCharacterSaveSlotName(),
			GetSelectedCharacterIndex());
	}
}

void ATalesGameStateBase::SaveGameDelegate(
		const FString& SlotName, const int32 UserIndex, bool bSuccess) const
{
	OnGameSaved.Broadcast(bSuccess);
}

void ATalesGameStateBase::SaveCharacterDelegate(
		const FString& SlotName, const int32 UserIndex, bool bSuccess) const
{
	if (GetIsCreatingCharacter())
	{
		return;
	}
	SaveMetaDataAsync(); // Save the metadata, too.
	OnCharacterSaved.Broadcast(bSuccess);
}

void ATalesGameStateBase::OnRep_CheatMode(bool OldState)
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
	if (!UGameplayStatics::DoesSaveGameExist(SaveMetaName_, 0))
	{
		UGlobalSaveData* NewSave = Cast<UGlobalSaveData>(
			UGameplayStatics::CreateSaveGameObject(UGlobalSaveData::StaticClass()));
		if (!IsValid(NewSave))
		{
			UE_LOG(LogTemp, Fatal, TEXT("Could not create new save game meta object."));
			return false;
		}
		bSaveMetaIsReady = UGameplayStatics::SaveGameToSlot(NewSave, SaveMetaName_, 0);
		return bSaveMetaIsReady;
	}
	// Already Exists
	return false;
}

bool ATalesGameStateBase::LoadCharacterSync(FString SaveSlotName)
{
	if (GetIsCreatingCharacter())
	{
		return false;
	}
	const USavedCharacter* SavedCharacter = GetSavedCharacterData(SaveSlotName);
	if (IsValid(SavedCharacter))
	{
		ACharacterBase* CharacterBase = Cast<ACharacterBase>
				( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
		if (IsValid(CharacterBase))
		{
			CharacterBase->LoadCharacterData(SaveSlotName, 0, nullptr);
			return true;
		}
	}
	return false;
}

void ATalesGameStateBase::LoadCharacterAsync(FString SaveSlotName)
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::CharacterSaveLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadedDelegate);
}

bool ATalesGameStateBase::LoadSaveGameMetaSync()
{
	USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(SaveMetaName_, 0);
	if (IsValid(SaveGame))
	{
		const UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>(SaveGame);
		if (IsValid(SaveMeta))
		{
			SaveGameMetaLoaded(SaveMetaName_, 0, SaveGame);
			return true;
		}
	}
	return false;
}

void ATalesGameStateBase::LoadSaveGameMetaAsync()
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameMetaLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveMetaName_, 0, LoadedDelegate);
}
