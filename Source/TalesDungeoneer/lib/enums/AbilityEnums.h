
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AbilityEnums.generated.h"

UENUM(BlueprintType)
enum class ETargetingOption : uint8
{
	ONE			UMETA(DisplayName = "Target Self"),
	NEAR_ENEMY	UMETA(DisplayName = "Nearest Enemy"),
	NEAR_NPC	UMETA(DisplayName = "Nearest Npc"),
	NEAR_PARTY  UMETA(DisplayName = "Nearest Party Member"),
	TWO			UMETA(DisplayName = "Party Member 2"),
	THREE		UMETA(DisplayName = "Party Member 3"),
	FOUR		UMETA(DisplayName = "Party Member 4"),
	FIVE		UMETA(DisplayName = "Party Member 5")
};

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
	
	// A synergetic Ability is an ability feeds on the power of other synergetic abilities
	SYNERGY		UMETA(DisplayName = "Synergy"),
};

UENUM()
enum class ESpellCastingType : uint8
{
	ONE_OFF 	UMETA(DisplayName = "One Handed Offense"),
	TWO_OFF		UMETA(DisplayName = "Two Handed Offense"),
	ONE_BUFF	UMETA(DisplayName = "One Handed Defensive"),
	TWO_BUFF	UMETA(DisplayName = "Two Handed Defensive"),
	ONE_AOE		UMETA(DisplayName = "One Handed Area Effect"),
	TWO_AOE		UMETA(DisplayName = "Two Handed Area Effect")
};

UENUM(BlueprintType)
enum class EAbilityTarget : uint8
{
	// Only affects the person using the ability
	SELF	UMETA(DisplayName = "Self Only"),
	
	// Targets all group members within a certain range
	GROUP	UMETA(DisplayName = "Nearby Group Members"),
	
	// Targets a single entity
	TARGET	UMETA(DisplayName = "Single Target"),
	
	// Hits everything in a radius around the target (if selected) or impact location
	AOE		UMETA(DisplayName = "Area of Target"),
	
	// Hits everything in a radius around the caster
	NEAR	UMETA(DisplayName = "Area of Self"),
	
	// Hits everything in a certain fan-pattern from the origination point
	CONE	UMETA(DisplayName = "Forward Cone"),
	
	// Ability fires a projectile, like a fireball or an arrow
	PROJECTILE	UMETA(DisplayName = "Moving Projectile")
};

