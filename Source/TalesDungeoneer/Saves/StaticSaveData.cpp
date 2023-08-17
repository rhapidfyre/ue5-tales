// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "StaticSaveData.h"
#include "SavedCharacters.h"

#include "Kismet/GameplayStatics.h"


UGlobalSaveData::UGlobalSaveData() {}

UGlobalSaveData::UGlobalSaveData(TArray<FString> RestoredCharacters)
{
	if (RestoredCharacters.Num() > 0)
	{
		_CharacterNames = RestoredCharacters;
	}
}

/**
 * @brief Sets the character names of the savegame object.
 * @param RestoredCharacters The list of character names saved
 */
void UGlobalSaveData::SetSavedCharacterNameList(TArray<FString> RestoredCharacters)
{
	_CharacterNames = RestoredCharacters;
}

/**
 * @brief Returns a pointer to the USavedCharacter data requested.
 *			Return requires validation before use.
 *			For threading, use 'GetSavedCharacterDataAsync'
 * @param SaveSlotName The FString of the saved character slot being requested
 * @return Nullptr on failure
 */
USavedCharacter* UGlobalSaveData::GetSavedCharacterData(FString SaveSlotName)
{
	if (SaveSlotName.IsEmpty())
		return nullptr;
	
	// Loop through saved characters to find the requested save slot
	for (FString TempSaveName : _CharacterNames)
	{
		USavedCharacter* SaveReference = Cast<USavedCharacter>(
			UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		
		if (IsValid(SaveReference))
		{
			if (SaveSlotName == SaveReference->SaveSlotName)
				return SaveReference;
		}
		
	}
	return nullptr;
}

/**
 * @brief Triggers 'LoadSaveData' delegate if/when a save is located (async)
 * @param SaveSlotName The FName of the saved character slot being requested
 * @param PlayerCharacter Reference to the character being saved
 */
void UGlobalSaveData::GetSavedCharacterDataAsync(FString SaveSlotName,
		APlayerCharacterBase* PlayerCharacter)
{
	FAsyncLoadGameFromSlotDelegate LoadDelegate;
	LoadDelegate.BindUObject(PlayerCharacter, &APlayerCharacterBase::LoadSaveData);
	UGameplayStatics::AsyncLoadGameFromSlot(SaveSlotName, 0, LoadDelegate);
}
