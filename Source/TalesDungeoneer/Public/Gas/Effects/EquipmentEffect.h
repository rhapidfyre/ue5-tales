// Take Five Games, LLC

#pragma once

#include "CoreMinimal.h"
#include "TalesEffectBase.h"

#include "EquipmentEffect.generated.h"


/**
 * An effect that is tied to a piece of equipment. The effect is only applied
 * when the equipment is donned, and active.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API UEquipmentEffect : public UTalesEffectBase
{
	GENERATED_BODY()
};
