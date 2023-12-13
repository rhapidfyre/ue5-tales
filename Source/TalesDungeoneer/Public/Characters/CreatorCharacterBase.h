// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "PlayerCharacterBase.h" // Includes core and actor files

#include "CreatorCharacterBase.generated.h"


/**
 * Base class for all logic regarding the character creator
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACreatorCharacterBase : public APlayerCharacterBase
{
	GENERATED_BODY()

public: // functions
	
	ACreatorCharacterBase();

protected:

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	virtual void LoadSaveData(const FString& SaveName,
		const int32 UserIndex, USaveGame* SaveData) override;
	
};