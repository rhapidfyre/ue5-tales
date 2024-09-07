// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "NativeGameplayTags.h"

#include "TalesAbilityGlobals.generated.h"

// Abilities that give detriment to the target of the ability
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Detrimental)
// Abilities that give benefits to the target of the ability
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Beneficial)
// Abilities that are neither detrimental nor beneficial
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Utility)
// Abilities used for development purposes
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Ability_Developer)



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
