// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gas/TalesAbilityGlobals.h"

#include "Gas/Contexts/VitalityEffectContext.h"

UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Detrimental, "Ability.Type.Detrimental");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Beneficial, "Ability.Type.Beneficial");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Utility, "Ability.Type.Utility");
UE_DEFINE_GAMEPLAY_TAG(TAG_Ability_Developer, "Ability.Type.Developer");


UTalesAbilityGlobals::UTalesAbilityGlobals()
{

}

FGameplayEffectContext* UTalesAbilityGlobals::AllocGameplayEffectContext() const
{
	return new FVitalityEffectContext();
}
