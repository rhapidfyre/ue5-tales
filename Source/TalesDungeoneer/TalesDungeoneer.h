// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogTales, Log, Error);
DECLARE_LOG_CATEGORY_EXTERN(LogCharacterBase, Log, Error);
DECLARE_LOG_CATEGORY_EXTERN(LogCombatSystem, Log, Error);

#include "CoreMinimal.h"

/**
 * An enum for organizing the different ways an ability input can fire/trigger
 */
UENUM()
enum class EAbilityInputID : uint8
{
	// Not Used; For iteration/tracking purposes
	None		UMETA(Hidden),

	// Confirms the activation of certain abilities
	Confirm		UMETA(DisplayName = "Confirm Activation"),

	// Cancels the activation of certain abilities
	Cancel		UMETA(DisplayName = "Cancel Activation"),

	// Begins the activation of certain abilities
	Activate	UMETA(DisplayName = "Activate"),

	// Not used; For iteration/tracking purposes
	MAX			UMETA(Hidden)
};