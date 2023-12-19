// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TalesDungeoneer/TalesDungeoneer.h"

#include "TalesGameplayAbility.generated.h"


/**
 * 
 */
UCLASS()
class TALESDUNGEONEER_API UTalesGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EAbilityInputID AbilityInputID { EAbilityInputID::None };
	
};
