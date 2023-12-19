// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gas/TalesAbilityGlobals.h"

#include "Gas/Contexts/VitalityEffectContext.h"


UTalesAbilityGlobals::UTalesAbilityGlobals()
{
	
}

FGameplayEffectContext* UTalesAbilityGlobals::AllocGameplayEffectContext() const
{
	return new FVitalityEffectContext();
}

