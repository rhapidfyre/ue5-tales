#pragma once

#include "CoreMinimal.h"

#include "WeaponEnums.generated.h"

UENUM(BlueprintType)
enum class EWeaponSlots : uint8
{
	NONE		UMETA(DisplayName = "Invalid Slot"),
	PRIMARY		UMETA(DisplayName = "Primary Hand"),
	SECONDARY	UMETA(DisplayName = "Secondary Hand"),
};

/**
 * Mostly used for animation purposes.
 */
UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	NONE	UMETA(DisplayName = "No Weapon / Punch"),
	PISTOL	UMETA(DisplayName = "Single Hand Pistol"),
	SWORD	UMETA(DisplayName = "One Hand Swing"),
	PICKAXE UMETA(DisplayName = "Two Hand Swing"),
	SPEAR	UMETA(DisplayName = "One Hand Piercing"),
	PIKE	UMETA(DisplayName = "Two Hand Piercing"),
	BOW		UMETA(DisplayName = "Longbow / Archery"),
	THROW	UMETA(DisplayName = "Thrown / Grenade"),
	SHIELD  UMETA(DisplayName = "Shield")
};