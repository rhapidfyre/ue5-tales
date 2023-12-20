// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Saves/StaticSaveData.h"
#include "Saves/SavedCharacters.h"
#include "Kismet/GameplayStatics.h"
#include "lib/datastructures/GlobalData.h"


UGlobalSaveData::UGlobalSaveData() :
	CharacterData_({}), SelectedCharacter_(-1)
{
	SaveVersion_ = UGlobalData::GetAppVersion();
}

UGlobalSaveData::UGlobalSaveData(
	const TArray<FSaveMeta>& RestoredCharacters, const int& SelectedCharacter) :
	CharacterData_(RestoredCharacters), SelectedCharacter_(SelectedCharacter)
{
	SaveVersion_ = UGlobalData::GetAppVersion();
}

/**
 * @brief Sets the character names of the save game object.
 * @param RestoredCharacters The list of character names saved
 */
// ReSharper disable once CppPassValueParameterByConstReference
void UGlobalSaveData::SetSavedCharacterNameList(TArray<FSaveMeta> RestoredCharacters)
{
	CharacterData_.Empty();
	if (RestoredCharacters.Num() > 0)
	{
		CharacterData_ = RestoredCharacters;
	}
}

void UGlobalSaveData::SetSelectedCharacter(int CharacterIndex)
{
	SelectedCharacter_ = -1;
	if (CharacterData_.IsValidIndex(CharacterIndex))
	{
		SelectedCharacter_ = CharacterIndex;
	}
}
