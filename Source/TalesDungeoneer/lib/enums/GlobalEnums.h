#pragma once

#include "CoreMinimal.h"

#include "GlobalEnums.generated.h"

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	// Any damage dealt by a physical force, such as a rock
	PHYSICAL	UMETA(DisplayName="Physical"),
	
	// Any damage dealt by sound, such as a shock wave
	SONIC		UMETA(DisplayName="Sonic"),
	
	// Any damage dealt by heat, fire or radiant energy
	HEAT 		UMETA(DisplayName="Heat"),
	
	// Any damage dealt by frost, cold or ice
	COLD 		UMETA(DisplayName="Cold"),
	
	// Any damage dealt by electricity, such as lightning
	SHOCK		UMETA(DisplayName="Shock"),
	
	// Any damage dealt by acidic material, such as corrosives
	ACID 		UMETA(DisplayName="Acidic"),
	
	// Any damage dealt by evil magic, such as necrotic energy
	DARK 		UMETA(DisplayName="Negative Energy"),
	
	// Any damage dealt by holy magic. Heals living things.
	HOLY 		UMETA(DisplayName="Holy Energy"),
	
	// Any damage dealt by sharp edges, such as a sword
	SLASH		UMETA(DisplayName="Slashing"),
	
	// Any damage dealt by stabbing, such as floor spikes and daggers
	PIERCE		UMETA(DisplayName="Piercing"),
	
	// Any damage dealt by toxic material, such as radiation or hazardous waste
	TOXIC		UMETA(DisplayName="Toxicity"),
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	NONE     UMETA(DisplayName = "Not Applicable"),
	RELAXED  UMETA(DisplayName = "Relaxed"),
	ALERT    UMETA(DisplayName = "Alert"),
	ENGAGED  UMETA(DisplayName = "Engaged"),
	RECOVERY UMETA(DisplayName = "Recovering"),
	INJURED  UMETA(DisplayName = "Incapacitated")
};

UENUM(BlueprintType)
enum class EElementalType : uint8
{
	NONE, FIRE, FROST, NATURE, AIR, WATER, DARK, SHOCK
};

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	WIZARD		UMETA(DisplayName = "Wizard"),
	WARRIOR		UMETA(DisplayName = "Warrior"),
	NECRO		UMETA(DisplayName = "Necromancer"),
	KNIGHT		UMETA(DisplayName = "Knight"),
	RANGER		UMETA(DisplayName = "Ranger"),
	CLERIC		UMETA(DisplayName = "Cleric"),
	ROGUE		UMETA(DisplayName = "Assassin"),
	MERC		UMETA(DisplayName = "Mercenary"),
	DEVIANT		UMETA(DisplayName = "Deviant")		
};
