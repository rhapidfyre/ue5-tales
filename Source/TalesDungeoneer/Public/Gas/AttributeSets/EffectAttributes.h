// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "UObject/Object.h"
#include "EffectAttributes.generated.h"

/**
 * Effect Attributes are any attributes that cause an effect on the player,
 * such as being chilled, speed boosts, etc.
 */
UCLASS()
class TALESDUNGEONEER_API UEffectAttributes : public UAttributeSet
{
	GENERATED_BODY()
};
