// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TalesDungeoneer/Entities/ProjectileBase.h"
#include "TalesDungeoneer/lib/datastructures/AbilityData.h"

#include "SpellProjectileBase.generated.h"

// A simple projectile class for bullets, cannonballs, etc
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ASpellProjectileBase : public AProjectileBase
{
	GENERATED_BODY()

public:

	ASpellProjectileBase() {}
	
	// If the projectile uses an ability effect
	ASpellProjectileBase(FName AbilityName);

	/**
	 * @brief Used if the projectile mimics a spell or ability effect
	 * @param AbilityName The name of the ability to mimic
	 */
	void SetProjectileData(FName AbilityName);

	UFUNCTION(BlueprintPure)
	FName GetAbilityName() const { return _AbilityName; };

protected:

	virtual void BeginPlay() override;

	virtual void Destroyed() override;
	
	void SetFromAbilityData();

	virtual void ApplyHitEffect(AActor* HitActor, FVector HitVector) override;

private:

	// Allows tracking of effects for cleanup if this actor gets destroyed
	// Only needed for attached effects or effects that are looped
	UPROPERTY() TArray<UNiagaraComponent*> LoopingNiagaraEmitters;
	
	// Allows tracking of effects for cleanup if this actor gets destroyed
	// Only needed for attached effects or effects that are looped
	UPROPERTY() TArray<UAudioComponent*> LoopingSoundEmitters;

	//Helper Function
	void PlayAbilityEffects(FStAbilityFx VisualEffect);
	
	//Helper Function. Calls PlayAbilityEffects.
	// Can be called either directly, or by an FTimerHandle for a delay
	void ExecuteSpellEffect(FStAbilityFx AbilityFx, bool StopOnDestroyed = false);
	
	UPROPERTY() FStAbilityData	_AbilityData;
	UPROPERTY() FStSpellData	_SpellData;

	UPROPERTY(Replicated) FName _AbilityName = FName();
	
};
