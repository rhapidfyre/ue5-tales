// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Delegates/Delegate.h"

#include "StaticSaveData.generated.h"


USTRUCT(BlueprintType)
struct FSaveMeta
{
	GENERATED_BODY();
	FSaveMeta() : SaveName(FString()), SaveIndex(0) {};
	FSaveMeta(FString NewName, int32 NewIndex) {SaveName=NewName;SaveIndex=NewIndex;}
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) FString SaveName;
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadWrite) int32 SaveIndex;
};

UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API UGlobalSaveData : public USaveGame
{
	GENERATED_BODY()
	
public:
	
	UGlobalSaveData();

	TArray<FSaveMeta> GetAllCharacterSaves() const { return CharacterData_; }

	void SetSelectedCharacter(int CharacterIndex = -1);
	
	void SetSavedCharacterNameList(const TArray<FSaveMeta>& RestoredCharacters);

	int GetSelectedCharacterIndex() const { return SelectedCharacter_; };

	UPROPERTY(SaveGame) TArray<FSaveMeta> CharacterData_ = {};
	
	UPROPERTY(SaveGame) int		SelectedCharacter_ = -1;
	UPROPERTY(SaveGame) FString SaveVersion_ = "x";
	
};
