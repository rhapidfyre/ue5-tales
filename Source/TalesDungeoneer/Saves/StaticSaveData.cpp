// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "StaticSaveData.h"
#include "SavedCharacters.h"

#include "Kismet/GameplayStatics.h"


UGlobalSaveData::UGlobalSaveData()
{
	
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
	for (FString TempSaveName : _SavedCharacters)
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

/**
 * @brief Saves the given character's data to a USaveGame. Synchronous function.
 * @param CharacterData The character being saved. Nullptr does nothing.
 * @param SaveSlotName The name for the save slot
 * @return True on success, false on failure
 */
bool UGlobalSaveData::SaveCharacter(APlayerCharacterBase* CharacterData, FString SaveSlotName)
{
	if (!IsValid(CharacterData))
		return false;
	
	if (SaveSlotName.IsEmpty())
		return false;

	CreateSaveSlotIfNotExists(SaveSlotName);

	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(GetSavedCharacterData(SaveSlotName));
	if (!IsValid(SavedCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveCharacter() FAILED: Could not retrieve save game object."));
		return false;
	}

	SaveCharacterValues(SavedCharacter, CharacterData);
	
	return UGameplayStatics::SaveGameToSlot(SavedCharacter, SaveSlotName, 0);
}

bool UGlobalSaveData::CreateCharacter(ACreatorCharacterBase* CharacterData, FString SaveSlotName)
{
	if (!IsValid(CharacterData))
		return false;
	
	if (SaveSlotName.IsEmpty())
		return false;

	CreateSaveSlotIfNotExists(SaveSlotName);

	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(GetSavedCharacterData(SaveSlotName));
	if (!IsValid(SavedCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("SaveCharacter() FAILED: Could not retrieve save game object."));
		return false;
	}

	SaveCharacterValues(SavedCharacter, CharacterData);
	
	return UGameplayStatics::SaveGameToSlot(SavedCharacter, SaveSlotName, 0);
}

/**
 * @brief Saves the given character without waiting for the response.
 * @param CharacterData A pointer to the character being saved
 * @param SaveSlotName The name for the save slot
 */
void UGlobalSaveData::SaveCharacterAsync(APlayerCharacterBase* CharacterData, FString SaveSlotName)
{
	if (!IsValid(CharacterData))
		return;
	
	if (SaveSlotName.IsEmpty())
		return;

	CreateSaveSlotIfNotExists(SaveSlotName);

	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(GetSavedCharacterData(SaveSlotName));
	if (IsValid(SavedCharacter))
	{
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &UGlobalSaveData::SaveCharacterDelegate);
		SaveCharacterValues(SavedCharacter, CharacterData);
		UE_LOG(LogTemp, Error, TEXT("Calling AsyncSaveGameToSlot()"));	
		UGameplayStatics::AsyncSaveGameToSlot(SavedCharacter, SaveSlotName, 0, SaveDelegate);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SaveCharacter() FAILED: Could not retrieve save game object."));	
	}
}

void UGlobalSaveData::SaveCharacterDelegate(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	UE_LOG(LogTemp, Error, TEXT("SaveCharacterDelegate()"));
}

/**
 * @brief Checks if the save slot exists, and if it does not, creates it.
 * @param SaveSlotName The name of the save slot to check or create
 * @return True if exists or was successfully created
 */
bool UGlobalSaveData::CreateSaveSlotIfNotExists(FString SaveSlotName)
{
	if (!_SavedCharacters.Contains(SaveSlotName))
	{
		if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			USavedCharacter* NewSave = Cast<USavedCharacter>(
				UGameplayStatics::CreateSaveGameObject(USavedCharacter::StaticClass()));
			if (!IsValid(NewSave))
			{
				UE_LOG(LogTemp, Fatal, TEXT("SaveCharacter() FAILED: Could not create new save game object."));
				return false;
			}
			NewSave->SaveSlotName = SaveSlotName;
			_SavedCharacters.Add(SaveSlotName);
			return UGameplayStatics::SaveGameToSlot(NewSave, SaveSlotName, 0);
		}
	}
	return true;
}

void UGlobalSaveData::SaveCharacterValues(USavedCharacter* SaveData, ACharacterBase* PlayerReference)
{
	SaveData->CharacterName  = PlayerReference->GetCharacterName();
	SaveData->CharacterLevel = PlayerReference->GetCharacterLevel();
	SaveData->CharacterClass = PlayerReference->GetCharacterClass();
	SaveData->CharacterRace  = PlayerReference->GetCharacterRace();

	// Save the version of the game when this character was saved
	SaveData->SaveVersion    = UGlobalData::GetAppVersion();
	UE_LOG(LogTemp, Display, TEXT("Successfully Saved '%s'"), *SaveData->CharacterName);
}

