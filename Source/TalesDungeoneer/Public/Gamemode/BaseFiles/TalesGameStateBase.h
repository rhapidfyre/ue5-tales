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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSelected, int, SelectedIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterDeleted, FString, SaveSlotName, int, DeletedIndex);


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
	FString GenerateAlphanumeric(FString OptionalPath = "") const;
	
	UFUNCTION(BlueprintPure)
	FString GetMetaDataSaveName() const { return SaveMetaName_; }
	
	UFUNCTION(BlueprintPure)
	static int32 GetMetaDataSaveIndex() { return 0; }

	UFUNCTION(BlueprintCallable)
	void SetIsCreatingCharacter(bool isCreating = true);
	
	UFUNCTION(BlueprintPure)
	bool GetIsCreatingCharacter() const { return bIsCreating; }

	// Use for later
	//UFUNCTION(BlueprintPure)
	//int32 GetCurrentSaveUserIndex() const { return 0; }

	ATalesGameStateBase();
	
	UPROPERTY(BlueprintAssignable)	FOnSaveGameObjectReady	OnSaveGameObjectReady;
	UPROPERTY(BlueprintAssignable)	FOnGameSaved			OnGameSaved;
	UPROPERTY(BlueprintAssignable)	FOnCharacterSelected	OnCharacterSelected;
	UPROPERTY(BlueprintAssignable)	FOnCharacterDeleted		OnCharacterDeleted;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_NewNotification(
		const FString& NewTitle, const FString& NewMessage, int NewPriority = 2);

	UFUNCTION(BlueprintCallable)
	void LocalNotification(FString NewTitle, FString NewMessage, int NewPriority = 2);
	
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
	void SaveMetaDataAsync();

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
	FString GetSaveGameMetaName() const { return SaveMetaName_; };
	
	// Retrieves a USaveGame object with the current meta
	UFUNCTION(BlueprintCallable) USaveGame* GetSaveGameMeta() const;
	
	// Returns an array of all known saved character slot names
	UFUNCTION(BlueprintCallable)
	TArray<FSaveMeta> GetSavedCharacterSlotNames() const;

	UFUNCTION(BlueprintPure)
	int GetIndexOfSavedCharacter(const FString& SlotName, const int32& UserIndex) const;

	/**
	 * @brief Returns the save data for the requested character slot. Return requires validation.
	 * @param SaveSlotName Optional. If empty string, will get the currently selected character.
	 * @return Saved data on success. Nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable)
	USavedCharacter* GetSavedCharacterData(FString SaveSlotName = "");
	
	UFUNCTION(BlueprintPure)
	FString GetCharacterSlotName() const;
	
	UFUNCTION(BlueprintPure)
	int GetCharacterUserIndex() const;
	
	UFUNCTION(BlueprintPure)
	bool GetIsValidCharacterSelected() const;
	
	UFUNCTION(BlueprintPure)
	bool GetDoesCharacterSaveExist() const;
	
	UFUNCTION(BlueprintPure)
	int GetSelectedCharacter() const;
	
	UFUNCTION(BlueprintCallable)
	void SetSelectedCharacter(int CharacterIndex);

	UFUNCTION(BlueprintCallable)
	int GetNextCharacterIndex();
	
	UFUNCTION(BlueprintCallable)
	int GetPrevCharacterIndex();
	
	UFUNCTION(BlueprintCallable)
	int GetLastCharacterIndex() const;
	
protected: // methods
	
	virtual void BeginPlay() override;

	// Called when LoadSaveGameMetaAsync executes
	void SaveGameMetaLoaded(const FString& SlotName,
		const int32 UserIndex, USaveGame* LoadedGameData);
	
	// Called when LoadCharacterASync executes
	void CharacterSaveLoaded(const FString& SlotName,
		int32 UserIndex, USaveGame* LoadedGameData);

	// Performs an sync character data load, returning true on success
	bool LoadCharacterSync(const FString& SaveSlotName = "", const uint32 SaveUserIndex = 0);

	// Performs an async character data load, calling CharacterLoaded when done
	void LoadCharacterAsync(const FString& SaveSlotName = "", const uint32 SaveUserIndex = 0);

	// Performs a sync character data load, returning true on success
	bool LoadSaveGameMetaSync();
	
	// Performs an async metadata load, calling SaveGameMetaLoaded when done
	void LoadSaveGameMetaAsync();

	bool SaveCharacterSync();

	void ResetCharacter();

	// Performs asynchronous save of the currently active character
	// Internally updates the save game meta file
	void SaveCharacterAsync();
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty> &OutLifetimeProps) const override;

private: // methods
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SendNotification(
		const FString& NewTitle, const FString& NewMessage, int NewPriority = 2);
	
	// Sets the values in the save metadata
	USaveGame* Helper_SetSaveValues();

	// Loads the values from save metadata into memory (this object)
	void Helper_LoadSavedValues(const USaveGame* SaveMeta);

	// Called when an asynchronous save has finished
	UFUNCTION()	void SaveGameDelegate(const FString& SlotName,
		const int32 UserIndex, bool bSuccess) const;
	
public: // members

	// The data table containing NPC data for NPCs spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	UDataTable* NpcDataTable = nullptr;

	// The data table containing all items in the game
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	UDataTable* ItemLookupTable = nullptr;

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
	bool bSavesOnServer = false;
	
private: // members

	// The name of the meta file
	UPROPERTY() FString SaveMetaName_ = "";

	UPROPERTY(ReplicatedUsing=OnRep_IsCreating) bool bIsCreating = false;
	UFUNCTION(Client, Reliable) void OnRep_IsCreating();

	// Set to true once the meta data file has been loaded or created
	UPROPERTY(ReplicatedUsing=OnRep_SaveMetaReady) bool bSaveMetaIsReady = false;
	UFUNCTION(Client, Reliable) void OnRep_SaveMetaReady();

	// A TArray of saved characters
	UPROPERTY() TArray<FSaveMeta> SavedCharacters_;

	// Which character is currently selected, where -1
	// indicates no character selected, or user is in the creator.
	UPROPERTY(ReplicatedUsing=OnRep_SelectedCharacter) int SelectedCharacter_ = -1;
	UFUNCTION(Client, Reliable) void OnRep_SelectedCharacter(const int OldSelection);

	UFUNCTION(NetMulticast, Reliable) void OnRep_CheatMode(bool OldState);
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CheatMode)
	bool CheatMode_ = false;
	
};