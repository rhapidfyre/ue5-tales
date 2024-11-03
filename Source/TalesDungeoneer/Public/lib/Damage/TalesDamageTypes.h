// Take Five Games, LLC

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/DamageType.h"

#include "TalesDamageTypes.generated.h"

/**
 * The top level master damage type for all damage dealt in the Tales game-mode.
 * Any other types of damage should not be considered
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API UTalesDamageBase : public UDamageType
{
	GENERATED_BODY()

public:

	UTalesDamageBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DamageValue;

	// Tick rate, representing hits-per-second... <= 0 means one-time-only.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DamageRate;

	// The most appropriate damage tag that describes this damage type at the deepest level
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag DamageTypeTag;

	// Gameplay tags that describe this damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer DamageTags;

};
