// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"
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
	void SetSelectedCharacter(int CharacterIndex = -1);
	
	UFUNCTION(BlueprintCallable)
	FString GetSelectedCharacterSaveSlotName() const;
	
	UFUNCTION(BlueprintCallable)
	void SetSavedCharacterNameList(TArray<FString> RestoredCharacters);
	
	UFUNCTION(BlueprintCallable)
	void GetSavedCharacterDataAsync(
			FString SaveSlotName, ACharacterBase* PlayerCharacter);

	// Return the index of the selected character
	UFUNCTION(BlueprintPure) int GetSelectedCharacterIndex() const;

	
private:

	UPROPERTY()	TArray<FString> _CharacterNames;
	
	UPROPERTY()	int _SelectedCharacter = -1;
	
};
