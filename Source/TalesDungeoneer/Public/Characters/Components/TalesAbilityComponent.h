// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags/Public/GameplayTags.h"
#include "AbilitySystemComponent.h"

#include "TalesAbilityComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UTalesAbilityComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTalesAbilityComponent();

	UFUNCTION(BlueprintCallable) void EnableStatCalculation(bool bCalculateNow = true);
	UFUNCTION(BlueprintCallable) void DisableStatCalculation();

	UFUNCTION(BlueprintPure) FGameplayTag GetCharacterRace() const;
	UFUNCTION(BlueprintPure) FGameplayTag GetCharacterClass() const;

	UFUNCTION(BlueprintPure) float GetCoreStatByTag(const FGameplayTag& StatTag) const;
	UFUNCTION(BlueprintPure) float GetDamageResistanceByTag(const FGameplayTag& DamageTag) const;
	UFUNCTION(BlueprintPure) float GetDamageBonusByTag(const FGameplayTag& DamageTag) const;
	
	UFUNCTION(BlueprintCallable) void PerformTotalRecalculation();
	UFUNCTION(BlueprintCallable) void RecalculateCoreStats();
	UFUNCTION(BlueprintCallable) void RecalculateDamageResists();
	UFUNCTION(BlueprintCallable) void RecalculateDamageBonuses();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusMultiple = 10.f; // For every this value, BonusModifier will be applied
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BonusModifier = 5.f; // +This is added/removed for every +/- BonusMultiple

private:
	bool bAllowCalculation = false;
};
