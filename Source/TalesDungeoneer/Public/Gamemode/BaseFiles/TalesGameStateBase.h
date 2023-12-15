// Copyright Take Five Games, LLC 2023 - All Rights Reserved

// ReSharper disable CppUEBlueprintCallableFunctionUnused

#pragma once

#include "GameFramework/GameState.h"
#include "GameFramework/SaveGame.h"
#include "Delegates/Delegate.h"
#include "Saves/StaticSaveData.h"

#include "CoreMinimal.h"
#include "TalesGameStateBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveGameObjectReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSaved, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSaved, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterSelected,
	FString, SaveSlotName, int, SelectedIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterDeleted,
	FString, SaveSlotName, int, DeletedIndex);


/* The tales game state base holds all of the logic for operations that
 * are independent of what mode the player is playing (adventure, title screen, etc)
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ATalesGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public: // methods

	bool CheckIsServer() const;

	bool CheckIsPlayableClient() const;

	UFUNCTION(BlueprintCallable)
	void SetIsCreatingCharacter(bool isCreating = true);
	
	UFUNCTION(BlueprintPure)
	bool GetIsCreatingCharacter() const { return bIsCreating; }

	ATalesGameStateBase();
	
	UDataTable* GetNpcDataTable();

	FStNpcData GetNpcData(FName NpcName);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_NewNotification(
		const FString& NewTitle, const FString& NewMessage, int NewPriority = 2);

	UFUNCTION(BlueprintCallable)
	void LocalNotification(FString NewTitle, FString NewMessage, int NewPriority = 2);
	
	/**
	 * @brief Sets the new save game meta file name,
	 *			   creating it if it does not exist.
	 * @param SaveSlotName The name of the new save meta file
	 */
	UFUNCTION(BlueprintCallable)
	void SetSaveGameMetaName(FString SaveSlotName);

	UFUNCTION(BlueprintPure)
	bool GetIsCheatModeEnabled() const { return CheatMode_; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	int DungeonLevel = 1;
	
	/**
	 *		SAVING
	 */
	
	// Called whenever save metadata has been modified
	UFUNCTION(BlueprintCallable)
	bool SaveMetaData();

	// Forces an asynchronous save of the save game meta file
	UFUNCTION(BlueprintCallable)
	void SaveMetaDataAsync() const;

	UFUNCTION(BlueprintPure)
	bool GetIsSaveMetaReady() const { return bSaveMetaIsReady; }
	
	// PERMANENTLY deletes the character and the associated save game
	UFUNCTION(BlueprintCallable)
	void RemoveSelectedCharacter();

	/**
	 * @brief Saves the character
	 * @param SaveResponse Response from the save attempt, whether successful or failure
	 * @param RunAsync If true, runs the save async and ignores the response
	 * @return True if ran async, or sync-save is successful. False otherwise.
	 */
	UFUNCTION(BlueprintCallable)
	bool SaveCurrentCharacter(FString& SaveResponse, bool RunAsync = true);
	
	
	/**
	 *		LOADING
	*/

	UFUNCTION(BlueprintCallable)
	bool LoadSaveGameMeta(bool LoadAsync = true);

	/**
	 *		OTHER METHODS
	*/
	
	// Returns the current name of the save game meta file
	UFUNCTION(BlueprintCallable)
	FString GetSaveGameMetaName() const { return _SaveMetaName; };
	
	// Retrieves a USaveGame object with the current meta
	UFUNCTION(BlueprintCallable) USaveGame* GetSaveGameMeta() const;
	
	// Returns an array of all known saved character slot names
	UFUNCTION(BlueprintCallable) TArray<FString> GetSavedCharacterSlotNames() const;

	/**
	 * @brief Triggers 'LoadSaveData' delegate if/when a save is located (async)
	 * @param SaveSlotName The FName of the saved character slot being requested
	 * @param PlayerCharacter Reference to the character being saved
	 */
	UFUNCTION(BlueprintCallable)
	void GetSavedCharacterDataAsync(FString SaveSlotName,
			ACharacterBase* PlayerCharacter);

	/**
	 * @brief Returns the save data for the requested character slot. Return requires validation.
	 * @param SaveSlotName Optional. If empty string, will get the currently selected character.
	 * @return Saved data on success. Nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable)
	USavedCharacter* GetSavedCharacterData(FString SaveSlotName = "");
	
	// Return the string of the '_CharacterNames' for the selected
	// character slot index. Returns empty string if no character selected.
	UFUNCTION(BlueprintPure) FString GetSelectedCharacterSaveSlotName() const;

	UFUNCTION(BlueprintPure) int GetSelectedCharacterIndex() const;
	
	UFUNCTION(BlueprintPure) bool GetDoesCharacterSaveExist() const;
	
	UFUNCTION(BlueprintCallable)
	void SetSavedCharacterNameList(TArray<FString> RestoredCharacters);
	
	// Sets which character is currently selected
	UFUNCTION(BlueprintCallable) void SetSelectedCharacter(int CharacterIndex);

	UFUNCTION(BlueprintCallable) int GetNextCharacterIndex();
	UFUNCTION(BlueprintCallable) int GetPrevCharacterIndex();
	UFUNCTION(BlueprintCallable) int GetLastCharacterIndex() const;
	
protected: // methods
	
	virtual void BeginPlay() override;

	// Called when LoadSaveGameMetaAsync executes
	void SaveGameMetaLoaded(const FString& SlotName,
		const int32 UserIndex, USaveGame* LoadedGameData);
	
	// Called when LoadCharacterASync executes
	void CharacterSaveLoaded(const FString& SlotName,
		int32 UserIndex, USaveGame* LoadedGameData);

	// Creates the save metadata file if it doesn't exist.
	// Returns true if created, false otherwise.
	bool CreateSaveGameIfNotExists();

	// Performs an sync character data load, returning true on success
	bool LoadCharacterSync(FString SaveSlotName = "");

	// Performs an async character data load, calling CharacterLoaded when done
	void LoadCharacterAsync(FString SaveSlotName = "");

	// Performs a synch character data load, returning true on success
	bool LoadSaveGameMetaSync();
	
	// Performs an async metadata load, calling SaveGameMetaLoaded when done
	void LoadSaveGameMetaAsync();

	// Performs synchronous save of the currently active character
	// Internally updates the save game meta file
	bool SaveCharacterSync(const FString& SaveSlotName);

	// Performs asynchronous save of the currently active character
	// Internally updates the save game meta file
	void SaveCharacterAsync(const FString& SaveSlotName);
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty> &OutLifetimeProps) const override;

private: // methods

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SendNotification(
		const FString& NewTitle, const FString& NewMessage, int NewPriority = 2);
	
	// Sets the values in the save metadata
	void Helper_SetSaveValues(UGlobalSaveData* SaveMeta) const;

	// Loads the values from save metadata into memory (this object)
	void Helper_LoadSavedValues(const UGlobalSaveData* SaveMeta);

	// Called when an asynchronous save has finished
	UFUNCTION()	void SaveGameDelegate(const FString& SlotName,
		const int32 UserIndex, bool bSuccess) const;

	UFUNCTION(BlueprintCallable) void SaveCharacterDelegate(const FString& SlotName,
		const int32 UserIndex, bool bSuccess) const;
	
public: // members

	// The data table containing NPC data for NPCs spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	UDataTable* NpcDataTable = nullptr;

	// The data table containing all items in the game
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	UDataTable* ItemLookupTable = nullptr;

	// Called when the save object is successfully loaded or created
	UPROPERTY(BlueprintAssignable)	FOnSaveGameObjectReady	OnSaveGameObjectReady;

	// Called when the game state saves the save meta object
	UPROPERTY(BlueprintAssignable)	FOnGameSaved			OnGameSaved;
	
	// Called when a character has been saved
	UPROPERTY(BlueprintAssignable)	FOnCharacterSaved		OnCharacterSaved;
	
	// Called when a character has been selected
	UPROPERTY(BlueprintAssignable)	FOnCharacterSelected	OnCharacterSelected;

	// Called when a character has been deleted
	UPROPERTY(BlueprintAssignable)	FOnCharacterDeleted OnCharacterDeleted;

	// If specified, pulls default start values from this data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* DataTableRace = nullptr;
	
	// If specified, pulls default start values from this data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* DataTableClass = nullptr;
	
	// If specified, pulls default start values from this data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* DataTableVitality = nullptr;
	
	// If specified, pulls default start values from this data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* DataTableInventory = nullptr;
	
	// If specified, pulls default start values from this data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* DataTableAbilities = nullptr;
	
	// If specified, pulls default start values from this data table
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* DataTableEffects = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int DefaultNumOfInventorySlots = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EEquipmentSlotType> DefaultEquipmentSlots = {
		EEquipmentSlotType::PRIMARY, EEquipmentSlotType::SECONDARY,
		EEquipmentSlotType::HELMET, EEquipmentSlotType::NECK,
		EEquipmentSlotType::EARRINGLEFT, EEquipmentSlotType::EARRINGRIGHT,
		EEquipmentSlotType::FACE, EEquipmentSlotType::SHOULDERS,
		EEquipmentSlotType::BACK, EEquipmentSlotType::SLEEVES,
		EEquipmentSlotType::WRISTLEFT, EEquipmentSlotType::WRISTRIGHT,
		EEquipmentSlotType::HANDS, EEquipmentSlotType::RINGLEFT,
		EEquipmentSlotType::RINGRIGHT, EEquipmentSlotType::TORSO,
		EEquipmentSlotType::WAIST, EEquipmentSlotType::LEGS,
		EEquipmentSlotType::FEET, EEquipmentSlotType::COSMETIC
	};
	
private: // members

	// The name of the meta file
	UPROPERTY() FString _SaveMetaName = "metadata";

	UPROPERTY(Replicated) bool bIsCreating = false;

	// Set to true once the meta data file has been loaded or created
	bool bSaveMetaIsReady = false;

	// A TArray of SaveSlotName of saved characters
	UPROPERTY()	TArray<FString> SavedCharacters_;

	// Which character is currently selected, where -1
	// indicates no character selected, or user is in the creator.
	UPROPERTY()	int _SelectedCharacter = -1;

	UFUNCTION() void OnRep_CheatMode(bool OldState);
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CheatMode)
	bool CheatMode_ = false;
	
};