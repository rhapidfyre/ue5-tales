// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterBase.h" // Includes core and actor files

#include "PlayerCharacterBase.generated.h"

/**
 * Player Character Base is the base C++ class for all logic, methods and members that affect all
 * PLAYER based characters, prior to handling by child classes or dependent blueprint classes.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API APlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public: // functions
	
	APlayerCharacterBase();
	
};
