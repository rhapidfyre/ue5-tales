// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "GameFramework/GameState.h"
#include "GameFramework/SaveGame.h"
#include "Delegates/Delegate.h"
#include "TalesDungeoneer/Saves/StaticSaveData.h"

#include "CoreMinimal.h"
#include "TalesGameStateBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveGameObjectReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSaved, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterSaved, bool, bSuccess);


/* The tales game state base holds all of the logic for operations that
 * are independent of what mode the player is playing (adventure, title screen, etc)
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ATalesGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public: // methods
	
	ATalesGameStateBase();

	/**
	 * @brief Sets the new save game meta file name,
	 *			   creating it if it does not exist.
	 * @param SaveSlotName The name of the new save meta file
	 */
	UFUNCTION(BlueprintCallable) void SetSaveGameMetaName(FString SaveSlotName);

	/**
	 *		SAVING
	 */
	
	// Called whenever save metadata has been modified
	UFUNCTION(BlueprintCallable) bool SaveMetaData();

	// Forces an asynchronous save of the save game meta file
	UFUNCTION(BlueprintCallable) void SaveMetaDataAsync();
	

	// Performs synchronous save of the currently active character
	// Internally updates the save game meta file
	UFUNCTION(BlueprintCallable) bool SaveCharacter(const FString SaveSlotName);

	// Performs asynchronous save of the currently active character
	// Internally updates the save game meta file
	UFUNCTION(BlueprintCallable) void SaveCharacterAsync(const FString SaveSlotName);

	/**
	 *		LOADING
	 */
	
	// Returns the current name of the save game meta file
	UFUNCTION(BlueprintCallable)
	FString GetSaveGameMetaName() const { return _SaveMetaName; };
	
	// Retrieves a USaveGame object with the current meta
	UFUNCTION(BlueprintCallable) USaveGame* GetSaveGameMeta() const;
	
	// Performs an async metadata load, calling SaveGameMetaLoaded when done
	UFUNCTION(BlueprintCallable) void LoadSaveGameMetaAsync();

	// Returns an array of all known saved character slot names
	UFUNCTION(BlueprintCallable) TArray<FString> GetSavedCharacterSlotNames() const;

	/**
	 * @brief Triggers 'LoadSaveData' delegate if/when a save is located (async)
	 * @param SaveSlotName The FName of the saved character slot being requested
	 * @param PlayerCharacter Reference to the character being saved
	 */
	UFUNCTION(BlueprintCallable)
	void GetSavedCharacterDataAsync(FString SaveSlotName,
			APlayerCharacterBase* PlayerCharacter);

	/**
	 * @brief Returns the save data for the requested character slot. Return requires validation.
	 * @param SaveSlotName Optional. If empty string, will get the currently selected character.
	 * @return Saved data on success. Nullptr on failure.
	 */
	UFUNCTION(BlueprintCallable)
	USavedCharacter* GetSavedCharacterData(FString SaveSlotName = "");
	
	// Return the string of '_CharacterNames' for the selected character slot index
	UFUNCTION(BlueprintPure) FString GetSelectedCharacterSlotSaveName() const;

	/**
	 *		OTHER METHODS
	 */
	
	UFUNCTION(BlueprintCallable)
	void SetSavedCharacterNameList(TArray<FString> RestoredCharacters);
	
	UFUNCTION(BlueprintPure) int GetSelectedCharacterIndex() const;
	
	
protected: // methods
	
	virtual void BeginPlay() override;
	
	/**
	 * @brief Saves the character
	 * @param SaveResponse Response from the save attempt, whether successful or failure
	 * @param RunAsync If true, runs the save async and ignores the response
	 * @return True if ran async, or sync-save is successful. False otherwise.
	 */
	bool SaveCurrentCharacter(FString& SaveResponse, bool RunAsync = true);

	// Called when LoadSaveGameMetaAsync executes
	void SaveGameMetaLoaded(const FString& SlotName,
				const int32 UserIndex, USaveGame* LoadedGameData);

	
private: // methods
	
	// Sets the values in the save metadata
	void Helper_SetSaveValues(UGlobalSaveData* SaveMeta);

	// Loads the values from save metadata into memory (this object)
	void Helper_LoadSavedValues(const UGlobalSaveData* SaveMeta);

	// Sets the character save values from game
	void Helper_SetCharacterValues(
			const ACharacterBase* CharacterBase, USaveGame* SaveData);

	// Sets the character save values from game
	void Helper_LoadCharacterValues(const FString SaveSlotName);

	// Called when an asynchronous save has finished
	UFUNCTION()	void SaveGameDelegate(const FString& SlotName,
		const int32 UserIndex, bool bSuccess);

	UFUNCTION(BlueprintCallable) void SaveCharacterDelegate(
		const FString& SlotName, const int32 UserIndex, bool bSuccess);

	// Creates the save metadata file if it doesn't exist.
	// Returns true if the file exists or was created.
	bool CreateSaveGameIfNotExists();

	// Creates the save data for the current character if it doesn't exist.
	// Returns true if the file exists or was created.
	bool CreateCharacterSaveIfNotExists();
	
public: // members

	// Called when the save object is successfully loaded or created
	UPROPERTY(BlueprintAssignable)	FOnSaveGameObjectReady	OnSaveGameObjectReady;

	// Called when the game state saves the save meta object
	UPROPERTY(BlueprintAssignable)	FOnGameSaved			OnGameSaved;
	
	// Called when a character has been saved
	UPROPERTY(BlueprintAssignable)	FOnCharacterSaved		OnCharacterSaved;
	
private: // members

	// The name of the meta file
	UPROPERTY() FString _SaveMetaName = "metadata";

	// Set to true once the meta data file has been loaded or created
	bool bSaveMetaIsReady = false;

	// A TArray of SaveSlotName of saved characters
	TArray<FString> _SavedCharacters;

	// Which index of the '_SavedCharacters' is currently selected or being played
	// Negative indicates no character selected, or playing as the DM.
	int _SelectedCharacter = -1;
	
};