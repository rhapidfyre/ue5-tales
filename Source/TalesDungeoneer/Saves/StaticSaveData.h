// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TalesDungeoneer/Characters/PlayerCharacterBase.h"
#include "TalesDungeoneer/Characters/CreatorCharacterBase.h"
#include "Delegates/Delegate.h"

#include "StaticSaveData.generated.h"


UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API UGlobalSaveData : public USaveGame
{
	GENERATED_BODY()
	
public:
	
	UGlobalSaveData();

	UGlobalSaveData(TArray<FString> RestoredCharacters);

	/**
	 * @brief Returns an array of all slot names of existing characters
	 * @return An array of all save slot names available
	 */
	UFUNCTION(BlueprintCallable)
	TArray<FString> GetAllCharacterSaves() const { return _CharacterNames; }

	UFUNCTION(BlueprintCallable)
	USavedCharacter* GetSavedCharacterData(FString SaveSlotName);

	UFUNCTION(BlueprintCallable)
	void SetSavedCharacterNameList(TArray<FString> RestoredCharacters);
	
	UFUNCTION(BlueprintCallable)
	void GetSavedCharacterDataAsync(
			FString SaveSlotName, APlayerCharacterBase* PlayerCharacter);

	UFUNCTION(BlueprintCallable)
	bool SaveCharacter(APlayerCharacterBase* CharacterData, FString SaveSlotName);

	UFUNCTION(BlueprintCallable)
	bool CreateCharacter(ACreatorCharacterBase* CharacterData, FString SaveSlotName);

	UFUNCTION(BlueprintCallable)
	void SaveCharacterAsync(APlayerCharacterBase* CharacterData, FString SaveSlotName);

	UFUNCTION(BlueprintCallable)
	void SaveCharacterDelegate(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	// Return the FName of '_CharacterNames' for the selected character
	UFUNCTION(BlueprintPure)
	FString GetSelectedCharacterSlotSaveName() const;
	
	// Return the FName of '_CharacterNames' for the selected character
	UFUNCTION(BlueprintPure)
	int GetSelectedCharacterIndex() const;
	
	UFUNCTION(BlueprintCallable)
	bool SetSelectedCharacter(int CharacterIndex);
	
protected:

	bool CreateSaveSlotIfNotExists();
	
	bool CreateCharacterSaveIfNotExists();
	
private:

	void SaveCharacterValues(USavedCharacter* SaveData, ACharacterBase* PlayerReference);

	TArray<FString> _CharacterNames;
	
	int _SelectedCharacter = -1;

};
