// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "CharacterBase.h" // Includes core and actor files

#include "CreatorCharacterBase.generated.h"


/**
 * Base class for all logic regarding the character creator
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACreatorCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public: // functions
	
	ACreatorCharacterBase();
	
protected:
	
	virtual void BeginPlay() override;

	virtual void CharacterRestoredFromSave(
		const FString SaveSlotName) override;
	
};