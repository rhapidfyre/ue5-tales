
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AbilityEnums.generated.h"

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	NONE		UMETA(DisplayName = "Invalid Ability"),
	
	// A benefit is an ability that helps the target in some way
	BENEFIT		UMETA(DisplayName = "Beneficial"),
	
	// A detriment is an ability that causes harm
	DETRIMENT	UMETA(DisplayName = "Detrimental"),
	
	// A utility ability is one that provide a non-beneficial assistance, such as levitation
	UTIL		UMETA(DisplayName = "Utility"),
	
	// A synergetic Ability is an ability that links up to other synergetic abilities
	SYNERGY		UMETA(DisplayName = "Synergy")
};

UENUM(BlueprintType)
enum class EAbilityTarget : uint8
{
	// Only affects the person using the ability
	SELF	UMETA(DisplayName = "Self Only"),
	
	// Targets all group members within a certain range
	GROUP	UMETA(DisplayName = "Group"),
	
	// Targets a single entity
	TARGET	UMETA(DisplayName = "Single Target"),
	
	// Hits everything in a radius around the origination point
	AOE		UMETA(DisplayName = "Area of Effect"),
	
	// Hits everything in a certain fan-pattern from the origination point
	CONE	UMETA(DisplayName = "Cone"),
	
	// Ability fires a projectile, like a fireball or an arrow
	PROJECTILE	UMETA(DisplayName = "Projectile")
};

