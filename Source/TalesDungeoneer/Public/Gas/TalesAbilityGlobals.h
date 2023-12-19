// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"

#include "TalesAbilityGlobals.generated.h"

/**
 * 
 */
UCLASS()
class TALESDUNGEONEER_API UTalesAbilityGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()
public:
	UTalesAbilityGlobals();

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
