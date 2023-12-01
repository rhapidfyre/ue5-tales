// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "TalesGameStateBase.h"
#include "TalesDungeoneer/Saves/SavedCharacters.h"
#include "VitalityStatComponent.h"
#include "VitalityEffectsComponent.h"
#include "VitalityWelfareComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Returns TRUE if the game state is any type of server
bool ATalesGameStateBase::CheckIsServer() const
{
	return GetNetMode() < NM_Client;
}

// Returns TRUE if the game state is server, but also a client
bool ATalesGameStateBase::CheckIsPlayableClient() const
{
	ENetMode NetMode = GetNetMode();
	return NetMode == NM_ListenServer || NetMode == NM_Standalone;
}

FString GetCleanedSaveSlotString(FString UncleanedString)
{
	return UncleanedString.Replace(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase);	
}


ATalesGameStateBase::ATalesGameStateBase() {}

UDataTable* ATalesGameStateBase::GetNpcDataTable()
{
	UDataTable* dataTable = NpcDataTable;
	if (IsValid(dataTable))
		return dataTable;
	return nullptr;
}

FStNpcData ATalesGameStateBase::GetNpcData(FName NpcName)
{
	if ( const UDataTable* dt = GetNpcDataTable() )
	{
		if (IsValid(dt))
		{
			const FString errorCaught;
			FStNpcData* recipePtr = dt->FindRow<FStNpcData>(NpcName, errorCaught);
			if (recipePtr != nullptr)
				return *recipePtr;
		}
	}
	return {};
}

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

void ATalesGameStateBase::SaveMetaDataAsync() const
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

void ATalesGameStateBase::RemoveSelectedCharacter()
{
	if (!CheckIsPlayableClient()) return;
	
	if (_SavedCharacters.IsValidIndex( GetSelectedCharacterIndex() ))
	{
		const FString SaveSlotName = GetSelectedCharacterSaveSlotName();
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			const int DeleteIndex = _SelectedCharacter;
			_SavedCharacters.RemoveAt(DeleteIndex);
			_SelectedCharacter = _SavedCharacters.Num() - 1;
			UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
			SaveMetaDataAsync(); // Update the save file
			OnCharacterDeleted.Broadcast(SaveSlotName, DeleteIndex);
		}
	}
}

bool ATalesGameStateBase::SaveCharacterSync(const FString SaveSlotName)
{
	// Characters always save on the client
	if (!CheckIsPlayableClient())
	{
		return false;
	}
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
		UE_LOG(LogTemp, Error, TEXT("SaveCharacterSync() FAILED: Could not retrieve save game object."));
		return false;
	}

	Helper_SetCharacterValues(CharacterBase, SavedCharacter);
	_SavedCharacters.Add(SaveSlotName);
	return UGameplayStatics::SaveGameToSlot(SavedCharacter, SaveSlotName, 0);
}

void ATalesGameStateBase::SaveCharacterAsync(const FString SaveSlotName)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		return;
	}
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>
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
		_SavedCharacters.Add(SaveSlotName);
		UGameplayStatics::AsyncSaveGameToSlot(SavedCharacter, SaveSlotName, 0, SaveDelegate);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SaveCharacterSync() FAILED: Could not retrieve save game object."));	
	}
}

void ATalesGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATalesGameStateBase, DungeonLevel);
}

USaveGame* ATalesGameStateBase::GetSaveGameMeta() const
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		return nullptr;
	}
	USaveGame* SavedMeta = Cast<USaveGame>(
			UGameplayStatics::LoadGameFromSlot(_SaveMetaName,0));
	if (IsValid(SavedMeta))
		return SavedMeta;
	return nullptr;
}

TArray<FString> ATalesGameStateBase::GetSavedCharacterSlotNames() const
{
	return _SavedCharacters;
}

void ATalesGameStateBase::GetSavedCharacterDataAsync(FString SaveSlotName, ACharacterBase* PlayerCharacter)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		return;
	}
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(PlayerCharacter, &ACharacterBase::LoadSaveData);
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
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter))
		return _SavedCharacters[_SelectedCharacter];
	return "";
}

int ATalesGameStateBase::GetSelectedCharacterIndex() const
{
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter))
		return _SelectedCharacter;
	return -1;
}

void ATalesGameStateBase::SetSavedCharacterNameList(TArray<FString> RestoredCharacters)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		return;
	}
	_SavedCharacters = RestoredCharacters;
}

void ATalesGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	// Create the save if it doesn't exist
	if ( !_SaveMetaName.IsEmpty() )
		CreateSaveGameIfNotExists();

	// Load the save game data from the previous session
	LoadSaveGameMeta( HasAuthority() );
	OnSaveGameObjectReady.Broadcast();
}

bool ATalesGameStateBase::SaveCurrentCharacter(FString& SaveResponse, bool RunAsync)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveTheSaveCurrentCharacterGame() FAILED: Saves are clientside."));
		return false;
	}
	SaveResponse = "Saving Character FAILED - Reason Unknown"; 
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
	if (SaveCharacterSync(SaveSlotName))
	{
		if (SaveMetaData())
		{
			SaveResponse = "Synchronous Save Successful";
			return true;
		}
		SaveResponse = "Synchronous Save FAILED";
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
	if (_SaveMetaName.IsEmpty())
		return false;
	
	if (LoadAsync)
	{
		LoadSaveGameMetaAsync();
		return true;
	}
	return LoadSaveGameMetaSync();
}

/**
 * @brief Loads the requested (or selected) character slot into memory.
 * @param SaveSlotName [opt] The name of the save slot. If empty string (default),
 *						will load the currently selected character.
 * @param LoadAsync True if loading can be ran asynchronously
 * @return True on successful load or async run, false otherwise.
 */
bool ATalesGameStateBase::LoadCharacter(FString SaveSlotName, bool LoadAsync)
{
	if (!CheckIsPlayableClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadCharacter() FAILED: Saves are clientside."));
		return false;
	}
	UE_LOG(LogTemp, Display, TEXT("LoadCharacter() Proceeding"));
	
	if (SaveSlotName.IsEmpty())
		SaveSlotName = GetSelectedCharacterSaveSlotName();
	
	if (SaveSlotName.IsEmpty())
		return false;
	
	if (LoadAsync)
	{
		LoadCharacterAsync(SaveSlotName);
		return true;
	}
	return LoadCharacterSync(SaveSlotName);
}

void ATalesGameStateBase::CharacterSaveLoaded(
		const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	if (!CheckIsPlayableClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("CharacterSaveLoaded() FAILED: Saves are clientside."));
		return;
	}
	const USavedCharacter* SavedCharacter = Cast<USavedCharacter>( LoadedGameData );
	if (!IsValid(LoadedGameData))
	{
		UE_LOG(LogTemp, Display, TEXT("CharacterSaveLoaded(): Save Slot '%s' Not found. Creating..."),
									*SlotName);
		CreateSaveGameIfNotExists();
		SavedCharacter = Cast<USavedCharacter>( GetSavedCharacterData() );
	}
	
	if (IsValid(SavedCharacter))
	{
		Helper_LoadCharacterValues(SlotName);
		return;
	}
	
	UE_LOG(LogTemp, Error, TEXT("CharacterSaveLoaded(): Save Slot '%s' Not found."),
								*SlotName);
}

void ATalesGameStateBase::SaveGameMetaLoaded(
		const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("SaveGameMetaLoaded() FAILED: Saves are clientside."));
		return;
	}
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
		Helper_LoadSavedValues(SaveMeta);
		OnSaveGameObjectReady.Broadcast();
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("SaveGameMetaLoaded(): Save Slot '%s' Not found."),
								*SlotName);
}

void ATalesGameStateBase::SetSelectedCharacter(int CharacterIndex)
{
	_SelectedCharacter = -1;
	if (_SavedCharacters.IsValidIndex(CharacterIndex))
	{
		_SelectedCharacter = CharacterIndex;
		LoadCharacterAsync(GetSelectedCharacterSaveSlotName());
	}
	SaveMetaData();
	OnCharacterSelected.Broadcast(
		GetSelectedCharacterSaveSlotName(),
		GetSelectedCharacterIndex());
}

int ATalesGameStateBase::GetNextCharacterIndex()
{
	// Increment to the next index
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter + 1))
	{
		++_SelectedCharacter;
		SetSelectedCharacter(_SelectedCharacter);
		return _SelectedCharacter;
	}
	
	// Get first index
	if (_SavedCharacters.Num() > 0)
	{
		SetSelectedCharacter(0);
		return _SelectedCharacter;
	}

	// Return Invalid
	return -1;
}

int ATalesGameStateBase::GetPrevCharacterIndex()
{
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter - 1))
	{
		_SelectedCharacter--;
		SetSelectedCharacter(_SelectedCharacter);
		return _SelectedCharacter;
	}
	if (_SavedCharacters.Num() > 0)
	{
		SetSelectedCharacter( GetLastCharacterIndex() );
		return _SelectedCharacter;
	}
	return -1;
}

int ATalesGameStateBase::GetLastCharacterIndex() const
{
	return _SavedCharacters.Num() - 1;
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
		SaveMeta->SetSavedCharacterNameList(_SavedCharacters);
		SaveMeta->SetSelectedCharacter(_SelectedCharacter);
	}
}

void ATalesGameStateBase::Helper_LoadSavedValues(const UGlobalSaveData* SaveMeta)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		UE_LOG(LogTemp, Warning, TEXT("Helper_LoadSavedValues() FAILED: Saves are clientside."));
		return;
	}
	if (IsValid(SaveMeta))
	{
		_SavedCharacters	= SaveMeta->GetAllCharacterSaves();
		_SelectedCharacter	= SaveMeta->GetSelectedCharacterIndex();
		LoadCharacterAsync(SaveMeta->GetSelectedCharacterSaveSlotName());
		OnCharacterSelected.Broadcast(
			GetSelectedCharacterSaveSlotName(),
			GetSelectedCharacterIndex());
	}
}

void ATalesGameStateBase::Helper_SetCharacterValues(
	const ACharacterBase* CharacterBase, USaveGame* SaveData) const
{
	if (!CheckIsPlayableClient()) return;
	
	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(SaveData);
	if (IsValid(SavedCharacter))
	{
		// Set Character Persona Data
		SavedCharacter->CharacterName  		= CharacterBase->GetCharacterName();
		SavedCharacter->CharacterLevel 		= CharacterBase->GetCharacterLevel();
		SavedCharacter->CharacterRace  		= CharacterBase->GetCharacterRace();
		SavedCharacter->CharacterClass 		= CharacterBase->GetCharacterClass();
		SavedCharacter->ExperiencePoints	= CharacterBase->GetExperiencePoints();
		
		// Save Mesh Mesh Data
		SavedCharacter->Skeleton            = CharacterBase->MeshMergeComponent->Skeleton;
		SavedCharacter->MeshSectionMappings = CharacterBase->MeshMergeComponent->MeshSectionMappings;
		SavedCharacter->UvTransformsPerMesh = CharacterBase->MeshMergeComponent->UvTransformsPerMesh;
		SavedCharacter->MeshesToMerge       = CharacterBase->MeshMergeComponent->MeshesToMerge;

		// Save Vitality Data
		const UVitalityWelfareComponent* VitalityWelfare = CharacterBase->VitalityWelfare;
		if (IsValid(VitalityWelfare))
		{
			// Set initial values
			SavedCharacter->UseHealthSubsystem			= VitalityWelfare->UseHealthSubsystem;
			SavedCharacter->UseStaminaSubsystem			= VitalityWelfare->UseStaminaSubsystem;
			SavedCharacter->UseMagicSubsystem			= VitalityWelfare->UseMagicSubsystem;
			SavedCharacter->UseSurvivalSubsystem		= VitalityWelfare->UseSurvivalSubsystem;
			
			float CurrentHealth, MaximumHealth;
			VitalityWelfare->GetCurrentHealth(CurrentHealth, MaximumHealth);
			SavedCharacter->StartingHealthCurrent		= CurrentHealth;
			SavedCharacter->StartingHealthMaximum		= MaximumHealth;
			SavedCharacter->PassiveHealthRegen			= VitalityWelfare->PassiveHealthRegen;
			SavedCharacter->HealthTimerTickRate			= VitalityWelfare->HealthTimerTickRate;
			
			float CurrentStamina, MaximumStamina;
			VitalityWelfare->GetCurrentStamina(CurrentStamina, MaximumStamina);
			SavedCharacter->StartingStaminaCurrent		= CurrentStamina;
			SavedCharacter->StartingStaminaMaximum		= MaximumStamina;
			SavedCharacter->PassiveStaminaRegen			= VitalityWelfare->PassiveStaminaRegen;
			SavedCharacter->StaminaTimerTickRate		= VitalityWelfare->StaminaTimerTickRate;
			
			float CurrentMagic, MaximumMagic;
			VitalityWelfare->GetCurrentMagic(CurrentMagic, MaximumMagic);
			SavedCharacter->StartingMagicCurrent		= CurrentMagic;
			SavedCharacter->StartingMagicMaximum		= MaximumMagic;
			SavedCharacter->PassiveMagicRegen			= VitalityWelfare->PassiveMagicRegen;
			SavedCharacter->MagicTimerTickRate			= VitalityWelfare->MagicTimerTickRate;
			
			float CurrentHydration, CurrentCalories, MaximumHydration, MaximumCalories;
			VitalityWelfare->GetCurrentHydration(CurrentHydration, MaximumHydration);
			VitalityWelfare->GetCurrentHunger(CurrentCalories, MaximumCalories);
			SavedCharacter->StartingHydrationCurrent	= CurrentHydration;
			SavedCharacter->StartingHungerCurrent		= CurrentCalories;
			SavedCharacter->StartingHydrationMaximum	= MaximumHydration;
			SavedCharacter->StartingHungerMaximum		= MaximumCalories;
			
			SavedCharacter->PassiveHydrationDrain		= VitalityWelfare->PassiveHydrationDrain;
			SavedCharacter->PassiveHungerDrain			= VitalityWelfare->PassiveHungerDrain;
			SavedCharacter->HydrationTimerTickRate		= VitalityWelfare->HydrationTimerTickRate;
			SavedCharacter->CaloriesTimerTickRate		= VitalityWelfare->CaloriesTimerTickRate;
		}
		
		if (IsValid(CharacterBase->AbilityComponent))
			SavedCharacter->UnlockPointsAvailable = CharacterBase->AbilityComponent->GetNumberOfUnlockPoints(); 
		
		const UVitalityStatComponent* VitalityStats = CharacterBase->VitalityStats;
		if (IsValid(VitalityStats))
		{
			// Restore Natural Stats
			SavedCharacter->BaseStats = VitalityStats->GetAllNaturalStats();
		}
		
		const UVitalityEffectsComponent* VitalityFx = CharacterBase->VitalityEffects;
		if (IsValid(VitalityFx))
		{
			// Restore Natural Stats
			SavedCharacter->SavedEffects = VitalityFx->GetAllActiveEffects();
		}
		
		// Save the version of the game when this character was saved
		SavedCharacter->SaveVersion    = UGlobalData::GetAppVersion();
	}
}

void ATalesGameStateBase::Helper_LoadCharacterValues(const FString SaveSlotName)
{
	if (!CheckIsPlayableClient()) return;
	USavedCharacter* SavedCharacter = GetSavedCharacterData(SaveSlotName);
	if (IsValid(SavedCharacter))
	{
		ACharacterBase* CharacterBase = Cast<ACharacterBase>
				( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
		if (IsValid(CharacterBase))
			CharacterBase->LoadSaveData(SaveSlotName, 0, SavedCharacter);
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
	
	SaveMetaDataAsync(); // Save the metadata, too.
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
	return false;
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
			USavedCharacter* NewSave = Cast<USavedCharacter>(
				UGameplayStatics::CreateSaveGameObject(USavedCharacter::StaticClass()));
			if (!IsValid(NewSave))
			{
				UE_LOG(LogTemp, Fatal, TEXT("Could not create new save game meta object."));
				return false;
			}
			return UGameplayStatics::SaveGameToSlot(NewSave, SaveSlotName, 0);
		}
	}
	return false;
}

bool ATalesGameStateBase::LoadCharacterSync(FString SaveSlotName)
{
	const USavedCharacter* SavedCharacter = GetSavedCharacterData(SaveSlotName);
	if (IsValid(SavedCharacter))
	{
		Helper_LoadCharacterValues(SaveSlotName);
		return true;
	}
	return false;
}

void ATalesGameStateBase::LoadCharacterAsync(FString SaveSlotName)
{
	if (!CheckIsPlayableClient()) return;
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::CharacterSaveLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadedDelegate);
}

bool ATalesGameStateBase::LoadSaveGameMetaSync()
{
	const USaveGame* SaveGame = UGameplayStatics::LoadGameFromSlot(_SaveMetaName, 0);
	if (IsValid(SaveGame))
	{
		const UGlobalSaveData* SaveMeta = Cast<UGlobalSaveData>(SaveGame);
		if (IsValid(SaveMeta))
		{
			Helper_LoadSavedValues(SaveMeta);
			bSaveMetaIsReady = true;
			OnSaveGameObjectReady.Broadcast();
			return true;
		}
	}
	return false;
}

void ATalesGameStateBase::LoadSaveGameMetaAsync()
{
	FAsyncLoadGameFromSlotDelegate LoadedDelegate;
	LoadedDelegate.BindUObject(this, &ATalesGameStateBase::SaveGameMetaLoaded);
	UGameplayStatics::AsyncLoadGameFromSlot(_SaveMetaName, 0, LoadedDelegate);
}
