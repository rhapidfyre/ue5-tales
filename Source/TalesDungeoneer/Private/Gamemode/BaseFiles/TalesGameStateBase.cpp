// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "TalesGameStateBase.h"
#include "TalesDungeoneer/Saves/SavedCharacters.h"
#include "VitalityStatComponent.h"
#include "VitalityEffectsComponent.h"
#include "VitalityWelfareComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/TalesDungeoneer.h"
#include "TalesDungeoneer/Gamemode/AdventureMode/TalesHudBase.h"

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

const FStCharacterRaces ATalesGameStateBase::GetStartingRaceData(
	const ECharacterRace CharacterRace) const
{
	if (IsValid(DataTableRace))
	{
		const FString dtName = DataTableRace->GetName();
		TArray<FStCharacterRaces*> dtStartingRaces;
		DataTableRace->GetAllRows<FStCharacterRaces>(*dtName, dtStartingRaces);

		for (const FStCharacterRaces* StartingRace : dtStartingRaces)
		{
			if (StartingRace->RaceEnum == CharacterRace
				|| StartingRace->RaceEnum == ECharacterRace::ANY)
			{
				return *StartingRace;
			}
		}
	}
	return {};
}

const FStCharacterClasses ATalesGameStateBase::GetStartingClassData(
	const ECharacterClass CharacterClass) const
{
	if (IsValid(DataTableClass))
	{
		const FString dtName = DataTableClass->GetName();
		TArray<FStCharacterClasses*> dtStartingRaces;
		DataTableClass->GetAllRows<FStCharacterClasses>(*dtName, dtStartingRaces);

		for (const FStCharacterClasses* StartingClass : dtStartingRaces)
		{
			if (	StartingClass->ClassEnum == CharacterClass
				||	StartingClass->ClassEnum == ECharacterClass::ANY)
			{
				return *StartingClass;
			}
		}
	}
	return {};
}

TArray<FStStartingItem> ATalesGameStateBase::GetStartingInventoryData(
	const ECharacterRace CharacterRace, const ECharacterClass CharacterClass)
{
	TArray<FStStartingItem> StartingItems;
	if (IsValid(DataTableInventory))
	{
		const FString dtName = DataTableInventory->GetName();
		TArray<FStDefaultStartingItem*> dtStartingItems;
		DataTableInventory->GetAllRows<FStDefaultStartingItem>(*dtName, dtStartingItems);

		for (const FStDefaultStartingItem* StartingItem : dtStartingItems)
		{
			if (	StartingItem->AllowedClasses.Contains(CharacterClass)
				||	StartingItem->AllowedClasses.Contains(ECharacterClass::ANY))
			{
				if (StartingItem->AllowedRaces.Contains(CharacterRace)
				||	StartingItem->AllowedRaces.Contains(ECharacterRace::ANY))
				{
					StartingItems.Add(FStStartingItem(
						StartingItem->startingItem,
						StartingItem->quantity,
						StartingItem->bStartEquipped));
				}
			}
		}
	}
	return StartingItems;
}

TArray<FName> ATalesGameStateBase::GetStartingAbilityData(const ECharacterClass CharacterClass)
{
	TArray<FName> StartingAbilities;
	if (IsValid(DataTableAbilities))
	{
		const FString dtName = DataTableAbilities->GetName();
		TArray<FStAbilityData*> dtStartingAbilities;
		DataTableAbilities->GetAllRows<FStAbilityData>(*dtName, dtStartingAbilities);
		
		for (const FStAbilityData* StartingAbility : dtStartingAbilities)
		{
			if (StartingAbility->AllowedClasses.Contains(CharacterClass))
			{
				StartingAbilities.Add( StartingAbility->GameName );
			}
		}
	}
	return StartingAbilities;
}

TArray<FStVitalityEffects> ATalesGameStateBase::GetStartingEffects(
	const ECharacterRace CharacterRace,
	const ECharacterClass CharacterClass)
{
	TArray<FStVitalityEffects> StartingEffects;
	if (IsValid(DataTableEffects))
	{
		const FString dtName = DataTableEffects->GetName();
		TArray<FStDefaultStartingEffects*> dtStartingEffects;
		DataTableEffects->GetAllRows<FStDefaultStartingEffects>(*dtName, dtStartingEffects);
		
		for (const FStDefaultStartingEffects* StartingEffect : dtStartingEffects)
		{
			if (	StartingEffect->AllowedClasses.Contains(CharacterClass)
				||	StartingEffect->AllowedClasses.Contains(ECharacterClass::ANY))
			{
				if (StartingEffect->AllowedRaces.Contains(CharacterRace)
				||	StartingEffect->AllowedRaces.Contains(ECharacterRace::ANY))
				{
					StartingEffects.Add(FStVitalityEffects(StartingEffect->EffectName));
				}
			}
		}
	}
	return StartingEffects;
}

FString GetCleanedSaveSlotString(const FString& UncleanedString)
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
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: Does not execute on Non-Playable Server", HasAuthority()?"S":"C");
		return false;
	}
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	if (!IsValid(CharacterBase))
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: Could not retrieve CharacterBase.", HasAuthority()?"S":"C");
		return false;
	}
	
	if (SaveSlotName.IsEmpty())
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: Invalid SaveSlotName (EMPTY)", HasAuthority()?"S":"C");
		return false;
	}

	CreateCharacterSaveIfNotExists();
	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(GetSavedCharacterData(SaveSlotName));
	if (!IsValid(SavedCharacter))
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: No Saved Character Data Exists", HasAuthority()?"S":"C");
		return false;
	}

	UE_LOGFMT(LogTales, Log, "SaveCharacterSync({sv}): Successfully Saved Character '{CharacterName}'",
		HasAuthority()?"S":"C", SavedCharacter->CharacterName);
	Helper_SetCharacterValues(CharacterBase, SavedCharacter);
	_SavedCharacters.Add(SaveSlotName);
	return UGameplayStatics::SaveGameToSlot(SavedCharacter, SaveSlotName, 0);
}

void ATalesGameStateBase::SaveCharacterAsync(const FString SaveSlotName)
{
	if (CheckIsServer() && !CheckIsPlayableClient())
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: Does not execute on Non-Playable Server", HasAuthority()?"S":"C");
		return;
	}
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>
			( UGameplayStatics::GetPlayerCharacter(GetWorld(), 0) );
	
	if (!IsValid(CharacterBase))
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: CharacterBase Invalid/Not Found", HasAuthority()?"S":"C");
		return;
	}
	
	if (SaveSlotName.IsEmpty())
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: SaveSlotName Invalid (EMPTY)", HasAuthority()?"S":"C");
		return;
	}

	CreateCharacterSaveIfNotExists();

	USavedCharacter* SavedCharacter = GetSavedCharacterData(SaveSlotName);
	if (IsValid(SavedCharacter))
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) Successfully Saved Character '{CharacterName}' (Async)",
			HasAuthority()?"S":"C", CharacterBase->CharacterName);
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &ATalesGameStateBase::SaveCharacterDelegate);
		Helper_SetCharacterValues(CharacterBase, SavedCharacter);
		_SavedCharacters.Add(SaveSlotName);
		UGameplayStatics::AsyncSaveGameToSlot(SavedCharacter, SaveSlotName, 0, SaveDelegate);
	}
	else
	{
		UE_LOGFMT(LogTales, Error, "SaveCharacterSync({sv}) FAILED: Could not create save file '{SaveName}'",
			HasAuthority()?"S":"C", SaveSlotName);
	}
}

void ATalesGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(ATalesGameStateBase, bIsCreating, COND_OwnerOnly);
	
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
	if (GetIsCreatingCharacter())
		return "";
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter))
		return _SavedCharacters[_SelectedCharacter];
	return "";
}

int ATalesGameStateBase::GetSelectedCharacterIndex() const
{
	if (GetIsCreatingCharacter())
		return -1;
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter))
		return _SelectedCharacter;
	return -1;
}

bool ATalesGameStateBase::GetDoesCharacterSaveExist() const
{
	if (GetIsCreatingCharacter())
		return false;
	if (_SavedCharacters.IsValidIndex(_SelectedCharacter))
	{
		if (_SavedCharacters[_SelectedCharacter] != "")
		{
			return UGameplayStatics::DoesSaveGameExist(_SavedCharacters[_SelectedCharacter], 0);
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
		UE_LOGFMT(LogTales, Warning, "SaveTheSaveCurrentCharacterGame(S) FAILED: Save Attempted on Server.");
		SaveResponse = "Server Tried to Save";
		return false;
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
	else
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
	if (GetIsCreatingCharacter())
		return false;
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
	if (GetIsCreatingCharacter())
		return;
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
	if (GetIsCreatingCharacter())
		return -1;
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
	if (GetIsCreatingCharacter())
		return -1;
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
	if (GetIsCreatingCharacter())
		return;
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
	USavedCharacter* CharacterSave = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterSave))
	{
		// Set Character Persona Data
		CharacterSave->CharacterName  		= CharacterBase->GetCharacterName();
		CharacterSave->CharacterLevel 		= CharacterBase->GetCharacterLevel();
		CharacterSave->CharacterRace  		= CharacterBase->GetCharacterRace();
		CharacterSave->CharacterClass 		= CharacterBase->GetCharacterClass();
		CharacterSave->ExperiencePoints		= CharacterBase->GetExperiencePoints();
		
		// Save Mesh Mesh Data
		CharacterSave->Skeleton            = CharacterBase->MeshMergeComponent->Skeleton;
		CharacterSave->MeshSectionMappings = CharacterBase->MeshMergeComponent->MeshSectionMappings;
		CharacterSave->UvTransformsPerMesh = CharacterBase->MeshMergeComponent->UvTransformsPerMesh;
		CharacterSave->MeshesToMerge       = CharacterBase->MeshMergeComponent->MeshesToMerge;

		// Save Vitality Data
		const UVitalityWelfareComponent* VitalityWelfare = CharacterBase->VitalityWelfare;
		if (IsValid(VitalityWelfare))
		{
			// Set initial values
			CharacterSave->UseHealthSubsystem		= VitalityWelfare->UseHealthSubsystem;
			CharacterSave->UseStaminaSubsystem		= VitalityWelfare->UseStaminaSubsystem;
			CharacterSave->UseMagicSubsystem		= VitalityWelfare->UseMagicSubsystem;
			CharacterSave->UseSurvivalSubsystem		= VitalityWelfare->UseSurvivalSubsystem;
			
			float CurrentHealth, MaximumHealth;
			VitalityWelfare->GetCurrentHealth(CurrentHealth, MaximumHealth);
			CharacterSave->StartingHealthCurrent	= CurrentHealth;
			CharacterSave->StartingHealthMaximum	= MaximumHealth;
			CharacterSave->PassiveHealthRegen		= VitalityWelfare->PassiveHealthRegen;
			CharacterSave->HealthTimerTickRate		= VitalityWelfare->HealthTimerTickRate;
			
			float CurrentStamina, MaximumStamina;
			VitalityWelfare->GetCurrentStamina(CurrentStamina, MaximumStamina);
			CharacterSave->StartingStaminaCurrent	= CurrentStamina;
			CharacterSave->StartingStaminaMaximum	= MaximumStamina;
			CharacterSave->PassiveStaminaRegen		= VitalityWelfare->PassiveStaminaRegen;
			CharacterSave->StaminaTimerTickRate		= VitalityWelfare->StaminaTimerTickRate;
			
			float CurrentMagic, MaximumMagic;
			VitalityWelfare->GetCurrentMagic(CurrentMagic, MaximumMagic);
			CharacterSave->StartingMagicCurrent		= CurrentMagic;
			CharacterSave->StartingMagicMaximum		= MaximumMagic;
			CharacterSave->PassiveMagicRegen		= VitalityWelfare->PassiveMagicRegen;
			CharacterSave->MagicTimerTickRate		= VitalityWelfare->MagicTimerTickRate;
			
			float CurrentHydration, CurrentCalories, MaximumHydration, MaximumCalories;
			VitalityWelfare->GetCurrentHydration(CurrentHydration, MaximumHydration);
			VitalityWelfare->GetCurrentHunger(CurrentCalories, MaximumCalories);
			CharacterSave->StartingHydrationCurrent	= CurrentHydration;
			CharacterSave->StartingHungerCurrent	= CurrentCalories;
			CharacterSave->StartingHydrationMaximum	= MaximumHydration;
			CharacterSave->StartingHungerMaximum	= MaximumCalories;
			
			CharacterSave->PassiveHydrationDrain	= VitalityWelfare->PassiveHydrationDrain;
			CharacterSave->PassiveHungerDrain		= VitalityWelfare->PassiveHungerDrain;
			CharacterSave->HydrationTimerTickRate	= VitalityWelfare->HydrationTimerTickRate;
			CharacterSave->CaloriesTimerTickRate	= VitalityWelfare->CaloriesTimerTickRate;
		}
		
		if (IsValid(CharacterBase->AbilityComponent))
			CharacterSave->UnlockPointsAvailable = CharacterBase->AbilityComponent->GetNumberOfUnlockPoints();

		// The inventory component saves internally.
		// We just need to remember the name of the save file to restore it.
		if (IsValid(CharacterBase->InventoryComponent))
		{
			FString ResponseString		= "";
			FString InventorySaveName	= CharacterBase->InventoryComponent->SaveInventory(ResponseString);
			if ( InventorySaveName.IsEmpty() )
			{
				UE_LOGFMT(LogTemp, Error,
					"Failed to Save Inventory for '{Character}'. Reason: {ResponseStr}",
					CharacterBase->GetCharacterName(), ResponseString);
			}
			CharacterSave->SavedInventory = InventorySaveName;
		}
		
		const UVitalityStatComponent* VitalityStats = CharacterBase->VitalityStats;
		if (IsValid(VitalityStats))
		{
			// Restore Natural Stats
			// When the character's gear and effects are loaded, this will recalculate.
			CharacterSave->BaseStats = VitalityStats->GetAllNaturalStats();
		}
		
		const UVitalityEffectsComponent* VitalityFx = CharacterBase->VitalityEffects;
		if (IsValid(VitalityFx))
		{
			// Restore Natural Stats
			CharacterSave->SavedEffects = VitalityFx->GetAllActiveEffects();
		}
		
		// Save the version of the game when this character was saved
		CharacterSave->SaveVersion    = UGlobalData::GetAppVersion();
	}
}

void ATalesGameStateBase::Helper_LoadCharacterValues(const FString SaveSlotName)
{
	if (GetIsCreatingCharacter())
		return;
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
	if (GetIsCreatingCharacter())
		return;
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
	if (GetIsCreatingCharacter())
		return false;
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
	if (GetIsCreatingCharacter())
		return;
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
