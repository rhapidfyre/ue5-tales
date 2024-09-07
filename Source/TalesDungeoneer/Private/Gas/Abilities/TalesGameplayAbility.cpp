// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gas/Abilities/TalesGameplayAbility.h"

#include "Gas/TalesAbilityGlobals.h"

UTalesGameplayAbility::UTalesGameplayAbility()
	: TimeToCooldown(0),
	  TimeToCast(0),
	  AbilityCategory(TAG_Ability_Beneficial.GetTag()),
	  AbilityRaces({}),
	  AbilityClasses({}),
	  AbilityInputID(EAbilityInputID::Activate),
	  EffectsSuccessSelf({}),
	  EffectsActivatedSelf({}),
	  EffectsFailedSelf({}),
	  EffectsSuccessTarget({}),
	  EffectsActivatedTarget({}),
	  EffectsFailedTarget({})
{
}
