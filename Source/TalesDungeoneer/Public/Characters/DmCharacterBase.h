// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"

#include "DmCharacterBase.generated.h"

/**
 * Dm Character Base is the base C++ class for all logic, methods and members that affect all
 * DUNGEONEER based characters, prior to handling by child classes or dependent blueprint classes.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ADmCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public: // functions
	
	ADmCharacterBase();

protected:
	
	virtual void BeginPlay() override;
	
};
