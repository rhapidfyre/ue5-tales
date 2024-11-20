// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Saves/StaticSaveData.h"
#include "Saves/SavedCharacters.h"
#include "Kismet/GameplayStatics.h"
#include "lib/datastructures/TalesGlobalData.h"


UGlobalSaveData::UGlobalSaveData()
{
	SaveVersion_ = UTalesGlobalData::GetAppVersion();
}

/**
 * \brief Sets the character names of the save game object.
 * \param RestoredCharacters The list of character names saved
 */
void UGlobalSaveData::SetSavedCharacterNameList(const TArray<FSaveMeta>& RestoredCharacters)
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
