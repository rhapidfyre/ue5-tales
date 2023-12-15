// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Characters/CharacterBase.h"

#include "CreatorGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class TALESDUNGEONEER_API ACreatorGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACreatorGameModeBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACharacterBase> BpCharacter = ACharacterBase::StaticClass();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
};
