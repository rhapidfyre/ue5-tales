#pragma once

#include "CoreMinimal.h"

#include "GlobalEnums.generated.h"


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