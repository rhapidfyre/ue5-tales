// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "CoreMinimal.h"
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

	UFUNCTION(BlueprintCallable)
	bool FinishCreation() { return CreateCharacter(); }

	virtual bool CreateCharacter();

protected:

	virtual void BeginPlay() override;

private:

	FString NewSaveSlotName_ = "";
	int32  NewSaveUserIndex_ = 0;
	
};