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
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString SaveName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SaveIndex;
};

UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API UGlobalSaveData : public USaveGame
{
	GENERATED_BODY()
	
public:
	
	UGlobalSaveData();

	UGlobalSaveData(const TArray<FSaveMeta>& RestoredCharacters,
					const int& SelectedCharacter = -1);

	TArray<FSaveMeta> GetAllCharacterSaves() const { return CharacterData_; }

	void SetSelectedCharacter(int CharacterIndex = -1);
	
	void SetSavedCharacterNameList(TArray<FSaveMeta> RestoredCharacters);

	int GetSelectedCharacterIndex() const { return SelectedCharacter_; };

	
private:

	TArray<FSaveMeta> CharacterData_;
	
	int		SelectedCharacter_;
	FString SaveVersion_ = "x";
	
};
