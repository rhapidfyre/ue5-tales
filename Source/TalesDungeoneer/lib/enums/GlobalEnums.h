#pragma once

#include "CoreMinimal.h"

#include "GlobalEnums.generated.h"

UENUM(BlueprintType)
enum class EFaction : uint8
{
	NONE		UMETA(DisplayName = "Neutral"),
	UNDEAD		UMETA(DisplayName = "All Undead"),
	DEMON		UMETA(DisplayName = "Demons & Devils"),
	DRAGON		UMETA(DisplayName = "Dragonkin"),
	HUMAN		UMETA(DisplayName = "Humankin"),
	DWARF		UMETA(DisplayName = "Dwarfkin"),
	ELF			UMETA(DisplayName = "Elfkin"),
	PLAYER		UMETA(DisplayName = "All Players & Pets"),
	CREATURE	UMETA(DisplayName = "All Creatures"),
	MERCHANT	UMETA(DisplayName = "Merchant NPCs"),
	GUARD		UMETA(DisplayName = "Guard NPCs"),
};

UENUM(BlueprintType)
enum class EFactionState : uint8
{
	// Combatants will ignore any actions taken against members of an indifferent faction
	// Noncombatants will treat them like any other neutral being
	NONE = 0		UMETA(DisplayName = "Indifferent"),
	
	// Hatred will result in combatants attacking on sight
	// Noncombatants will refuse service
	HATE = 1		UMETA(DisplayName = "Hated"),
	
	// Likefulness will result in combatants ignoring the faction's members
	// Noncombatants will offer better services
	LIKE = 2		UMETA(DisplayName = "Liked"),
	
	// Combatants will come to the aid of allied factions
	ALLY = 3		UMETA(DisplayName = "Allied"),
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
	DEVIANT		UMETA(DisplayName = "Deviant"),
	// Used for non-selections
	ANY			UMETA(DisplayName = "Any"),
};


UENUM(BlueprintType)
enum class ECharacterRace : uint8
{
	HUMAN		UMETA(DisplayName = "Human"),
	DWARF		UMETA(DisplayName = "Dwarf"),
	ELF			UMETA(DisplayName = "Elf"),
	FRIEND		UMETA(DisplayName = "Friendly NPCs"),
	ENEMY		UMETA(DisplayName = "Enemy NPCs"),
	// Used for non-selections
	ANY			UMETA(DisplayName = "Any")
};


UENUM(BlueprintType)
enum class ECharacterTeam : uint8
{
	SPECTATOR	UMETA(DisplayName = "Unassigned"),
	PLAYER		UMETA(DisplayName = "Players"),
	DUNGEONEER	UMETA(DisplayName = "Dungeoneer"),
	FRIEND		UMETA(DisplayName = "Friendly NPCs"),
	ENEMY		UMETA(DisplayName = "Enemy NPCs")
};