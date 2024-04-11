// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Gamemode/BaseFiles/TalesGameStateBase.h"

#include "Saves/SavedCharacters.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "../TalesDungeoneer.h"
#include "Characters/CharacterBase.h"
#include "Characters/PlayerCharacterBase.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Gamemode/AdventureMode/TalesHudBase.h"

#include "lib/datastructures/GlobalData.h"

ACharacter* FindPlayerCharacter(const UWorld* WorldContext)
{
	if (WorldContext == nullptr)
	{
		return nullptr;
	}

	for (FConstPlayerControllerIterator It = WorldContext->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->IsLocalController())
		{
			return Cast<ACharacter>(PC->GetPawn());
		}
	}

	return nullptr;
}


bool ATalesGameStateBase::LoadDataAsset(const FName& AssetName)
{
	const FPrimaryAssetType AssetType = UDataAsset::StaticClass()->GetFName();
	const auto				AssetId   = FPrimaryAssetId(AssetType, AssetName);
	return LoadDataAsset(AssetId);
}

bool ATalesGameStateBase::LoadDataAsset(const FPrimaryAssetId& AssetId)
{
	if (!AssetId.IsValid()) { return false; }

	auto& AssetManager = UAssetManager::Get();
	const UDataAsset* DataAsset = Cast<UDataAsset>( AssetManager.GetPrimaryAssetObject(AssetId) );

	// The asset is already loaded
	if (IsValid(DataAsset))
	{
		LoadDataAssetDelegate(AssetId);
		return true;
	}

	const TArray<FName>       AssetBundle    = {};
	const FStreamableDelegate StreamDelegate = FStreamableDelegate::CreateUObject(
		this, &ATalesGameStateBase::LoadDataAssetDelegate, AssetId);

	AssetManager.LoadPrimaryAsset(AssetId, AssetBundle, StreamDelegate);
	return true;
}


void ATalesGameStateBase::LoadDataAssetDelegate(const FPrimaryAssetId AssetId)
{
	const auto&			AssetManager	= UAssetManager::Get();
	const UDataAsset*	DataAsset		= Cast<UDataAsset>( AssetManager.GetPrimaryAssetObject(AssetId) );
	
	if (IsValid(DataAsset) && OnDataAssetLoaded.IsBound())
	{
		OnDataAssetLoaded.Broadcast(DataAsset, true);
	}
}

UTexture2D* ATalesGameStateBase::GetEquipmentIcon(const FGameplayTag& EquipmentTag) const
{
	if (EquipmentSlotIcons.Contains(EquipmentTag))
	{
		return *EquipmentSlotIcons.Find(EquipmentTag);
	}
	return nullptr;
}

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
	
	USaveGame* SaveMeta = Helper_SetSaveValues();
	if (!IsValid(SaveMeta))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveTheGame() FAILED: Could not retrieve save game meta object."));
		return false;
	}

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveMeta, GetMetaDataSaveName(), 0);
	return bSuccess;
}

void ATalesGameStateBase::SaveMetaDataAsync()
{
	USaveGame* SaveMeta = Helper_SetSaveValues();
	if (!IsValid(SaveMeta))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveTheGame() FAILED: Could not retrieve save game meta object."));
		return;
	}
	
	FAsyncSaveGameToSlotDelegate SaveDelegate;
	SaveDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameDelegate);
	UGameplayStatics::AsyncSaveGameToSlot(SaveMeta, GetMetaDataSaveName(), 0, SaveDelegate);
}

void ATalesGameStateBase::RemoveSelectedCharacter()
{
	if ((HasAuthority() && !bSavesOnServer) || (!HasAuthority() && bSavesOnServer))
	{
		const ENetMode netMode = GetNetMode();
		if (netMode != NM_Standalone && netMode != NM_ListenServer)
		{
			UE_LOGFMT(LogGameState, Warning, "RemoveSelectedCharacter({Sv}}) "
				"FAILED: Authority Violation", HasAuthority()?"SRV":"CLI");
			return;
		}
	}
	
	const int DeleteIndex = GetSelectedCharacter();
	if (SavedCharacters_.IsValidIndex(DeleteIndex))
	{
		const FString SaveSlotName = GetCharacterSlotName();
		const int32 SaveUserIndex  = GetCharacterUserIndex();
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
		{
			// Disallow operations while the saves are being manipulated
			const bool metaWasReady = GetIsSaveMetaReady();
			bSaveMetaIsReady = false;

			const USavedCharacter* SavedCharacter = Cast<USavedCharacter> (
				UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex) );
			
			if (IsValid(SavedCharacter))
			{
				// Delete subservient save games (inventory, abilities, etc)
				const FString InventorySave = UGlobalData::InventorySaveFolder() + SavedCharacter->SavedInventory;
				const FString MeshMergeSave = UGlobalData::MeshMergeSaveFolder() + SavedCharacter->SavedMeshMerge;
				
				UGameplayStatics::DeleteGameInSlot(InventorySave, GetCharacterUserIndex());
				UGameplayStatics::DeleteGameInSlot(MeshMergeSave, GetCharacterUserIndex());
			}

			// Delete the actual save game
			UGameplayStatics::DeleteGameInSlot(SaveSlotName, SaveUserIndex);
			
			SavedCharacters_.RemoveAt(DeleteIndex);
			OnCharacterDeleted.Broadcast(SaveSlotName, DeleteIndex);
			bSaveMetaIsReady = metaWasReady;
			
			// Internally calls SaveMetaData
			SetSelectedCharacter(DeleteIndex - 1);
		}
	}
}

// Performs save of the currently active character - Updates the save meta.
// Calls 'OnCharacterSaved' when finished
bool ATalesGameStateBase::SaveCharacterSync(USaveGame*& SaveGame)
{
	APlayerCharacterBase* CharacterBase = Cast<APlayerCharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	if (!IsValid(CharacterBase))
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacter({sv}) FAILED: Could not retrieve CharacterBase.", HasAuthority()?"S":"C");
		return false;
	}

	SaveGame = CharacterBase->SaveCharacter();
	return (IsValid(SaveGame));
}

void ATalesGameStateBase::ResetCharacter()
{
	// TODO
}

void ATalesGameStateBase::SaveCharacterAsync()
{
	USaveGame* SaveGame = nullptr;
	SaveCharacterSync(SaveGame);
	AddOrUpdateSavedCharacter(SaveGame);
}

void ATalesGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ATalesGameStateBase, CheatMode_,			COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ATalesGameStateBase, bSaveMetaIsReady,		COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ATalesGameStateBase, SelectedCharacter_,	COND_OwnerOnly);
	
	DOREPLIFETIME(ATalesGameStateBase, DungeonLevel);
}

void ATalesGameStateBase::Multicast_SendNotification_Implementation(
	const FString& NewTitle, const FString& NewMessage, int NewPriority)
{
	
}

USaveGame* ATalesGameStateBase::GetSaveGameMeta() const
{
	if (CheckIsServer() && !CheckIsPlayableClient()) { return nullptr; }
	USaveGame* SavedMeta = Cast<USaveGame>(
			UGameplayStatics::LoadGameFromSlot(GetMetaDataSaveName(),0));
	return IsValid(SavedMeta) ? SavedMeta : nullptr;
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
	if (SavedCharacters_.IsValidIndex(SelectedCharacter_))
	{
		return SavedCharacters_[SelectedCharacter_].SaveIndex;
	}
	return 0;
}

bool ATalesGameStateBase::GetIsValidCharacterSelected() const
{
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
	SaveMetaName_ = "SaveMeta_" + UGlobalData::GetAppVersion(true, true);
	if ( GetMetaDataSaveName().IsEmpty() ) {SaveMetaName_ = "SaveMeta"; }
	
	// Load the save game data from the previous session
	LoadSaveGameMeta(false);
	
	OnSaveGameObjectReady.Broadcast();

	// Attempt to load the selected character
	if (GetDoesCharacterSaveExist())
	{
		ACharacterBase* CharacterBase = Cast<ACharacterBase>
				( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
		if (IsValid(CharacterBase))
		{
			CharacterBase->LoadCharacter(GetCharacterSlotName(), GetCharacterUserIndex());
		}
	}
}

void ATalesGameStateBase::AddOrUpdateSavedCharacter(USaveGame* SaveGame)
{
	const USavedCharacter* savedCharacter = Cast<USavedCharacter>(SaveGame);
	if (IsValid(savedCharacter))
	{
		for (const FSaveMeta& character : SavedCharacters_)
		{
			if (character.SaveName == savedCharacter->SaveSlotName && character.SaveIndex == savedCharacter->UserIndex)
			{
				// Character already exists
				return;
			}
		}

		// Adds the new character save to the list of new characters
		const int NewIndex = SavedCharacters_.Add(FSaveMeta(savedCharacter->SaveSlotName, savedCharacter->UserIndex));
		SetSelectedCharacter(NewIndex); // Internally calls SaveMetaData
	}
}

bool ATalesGameStateBase::SaveCurrentCharacter(FString& SaveResponse, bool RunAsync)
{
	if ((HasAuthority() && !bSavesOnServer) || (!HasAuthority() && bSavesOnServer))
	{
		const ENetMode netMode = GetNetMode();
		if (netMode != NM_Standalone && netMode != NM_ListenServer)
		{
			UE_LOGFMT(LogTales, Warning, "SaveCurrentCharacter({Sv}) FAILED: "
				"Authority Violation", HasAuthority()?"SRV":"CLI");
			SaveResponse = "Authority Violation";
			return false;
		}
	}
	
	if (!GetIsSaveMetaReady())
	{
		SaveResponse = "SaveMeta isn't ready yet.";
		UE_LOGFMT(LogTales, Warning, "SaveCurrentCharacter({Sv}) FAILED: "
			"SaveMeta is not ready yet.", HasAuthority()?"SRV":"CLI");
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
	
	USaveGame* SaveGame = nullptr;
	if (SaveCharacterSync(SaveGame))
	{
		AddOrUpdateSavedCharacter(SaveGame);
		SaveResponse = "Synchronous Save Successful";
		return true;
	}
	
	SaveResponse = "SaveCharacterSync returned FALSE";
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
	UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>( LoadedGameData );
	if (!IsValid(SaveMeta))
	{
		SaveMeta = Cast<UGlobalSaveData>
			( UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex) );
	}

	// If the save meta was loaded successfully, then we can restore the save state
	const bool bSaveValid = IsValid(SaveMeta);
	if (bSaveValid)
	{
		Helper_LoadSavedValues(SaveMeta);
	}// If it was not loaded, there is no save to restore
	
	bSaveMetaIsReady = true;
	
	if (OnGameSaved.IsBound())
		{ OnGameSaved.Broadcast(bSaveValid); }
}

void ATalesGameStateBase::SetSelectedCharacter(int CharacterIndex)
{
	if (SavedCharacters_.IsValidIndex(CharacterIndex))
	{
		SelectedCharacter_ = CharacterIndex;
	}
	else
	{
		// Will either be -1, or a valid index
		SelectedCharacter_ = SavedCharacters_.Num() - 1;
		ResetCharacter();
	}
	
	// If the newly selected character fails to load, deselect.
	if (!LoadCharacterSync( GetCharacterSlotName(), GetCharacterUserIndex() ))
	{
		SelectedCharacter_ = -1;
		ResetCharacter();
	}

	// Must save the metadata whenever the selection changes
	SaveMetaDataAsync();
	OnCharacterSelected.Broadcast(SelectedCharacter_);
}

int ATalesGameStateBase::GetSelectedCharacter() const
{
	return SelectedCharacter_;
}

int ATalesGameStateBase::GetNextCharacterIndex()
{
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

USaveGame* ATalesGameStateBase::Helper_SetSaveValues()
{
	UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>(
		UGameplayStatics::CreateSaveGameObject( UGlobalSaveData::StaticClass() ));
	SaveMeta->SetSavedCharacterNameList(SavedCharacters_);
	SaveMeta->SetSelectedCharacter(SelectedCharacter_);
	return SaveMeta;
}

void ATalesGameStateBase::Helper_LoadSavedValues(const USaveGame* SaveMeta)
{
	const UGlobalSaveData* SaveData = Cast<UGlobalSaveData>(SaveMeta);
	if (IsValid(SaveData))
	{
		SavedCharacters_	= SaveData->GetAllCharacterSaves();
		SelectedCharacter_	= SaveData->GetSelectedCharacterIndex();
		OnSaveGameObjectReady.Broadcast();
		OnCharacterSelected.Broadcast( GetSelectedCharacter() );
	}
}

void ATalesGameStateBase::SaveGameDelegate(
		const FString& SlotName, const int32 UserIndex, bool bSuccess) const
{
	UGlobalSaveData* SaveData = Cast<UGlobalSaveData>(
			UGameplayStatics::LoadGameFromSlot(SlotName,UserIndex));
	if (OnGameSaved.IsBound())
		{ OnGameSaved.Broadcast(bSuccess); }
}

void ATalesGameStateBase::OnRep_SaveMetaReady_Implementation()
{
	OnSaveGameObjectReady.Broadcast();
}

void ATalesGameStateBase::OnRep_SelectedCharacter_Implementation(const int OldSelection)
{
	OnCharacterSelected.Broadcast( GetSelectedCharacter() );
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

bool ATalesGameStateBase::LoadCharacterSync(const FString& SaveSlotName, const uint32 SaveUserIndex)
{
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

void ATalesGameStateBase::LoadCharacterAsync(const FString& SaveSlotName, const uint32 SaveUserIndex)
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::CharacterSaveLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadedDelegate);
}

bool ATalesGameStateBase::LoadSaveGameMetaSync()
{
	if (UGameplayStatics::DoesSaveGameExist(GetMetaDataSaveName(), 0))
	{
		SaveGameMetaLoaded(GetMetaDataSaveName(), 0, nullptr);
		return true;
	}
	// There is no save to load. We are ready to operate.
	bSaveMetaIsReady = true;
	return false;
}

void ATalesGameStateBase::LoadSaveGameMetaAsync()
{
	if (UGameplayStatics::DoesSaveGameExist(GetMetaDataSaveName(), 0))
	{
		FAsyncLoadGameFromSlotDelegate LoadedDelegate;
		LoadedDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameMetaLoaded);
		UGameplayStatics::AsyncLoadGameFromSlot(GetMetaDataSaveName(), 0, LoadedDelegate);
	}
	// There is no save to load. We are ready to operate.
	else { bSaveMetaIsReady = true; }
}
