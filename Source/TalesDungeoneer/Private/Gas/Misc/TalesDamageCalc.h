// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "TalesDamageCalc.generated.h"

/**
 * 
 */
UCLASS()
class TALESDUNGEONEER_API UTalesDamageCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	
	UTalesDamageCalc();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InDamageTag = FName("Damage.SetByCaller");
};
