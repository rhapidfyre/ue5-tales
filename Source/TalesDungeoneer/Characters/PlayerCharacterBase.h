// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "CharacterBase.h" // Includes core and actor files
#include "Components/SphereComponent.h"

#include "PlayerCharacterBase.generated.h"

// Called when this player has fully spawned into the world
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerJoined);

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

	UPROPERTY(BlueprintAssignable) FOnPlayerJoined OnPlayerJoined;
	
protected:
	
	virtual void BeginPlay() override;
	
};
