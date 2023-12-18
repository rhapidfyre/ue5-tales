// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Characters/PlayerCharacterBase.h"

#include "TalesDungeoneerGameMode.generated.h"

UCLASS(minimalapi)
class ATalesDungeoneerGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATalesDungeoneerGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ACharacterBase> DefaultCharacter = APlayerCharacterBase::StaticClass();
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
};



