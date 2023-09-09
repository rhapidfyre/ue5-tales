// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "TalesDungeoneer/Characters/NpcCharacterBase.h"
#include "TalesDungeoneer/Characters/Controllers/AiControllerBase.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"

#include "CombatAiControllerBase.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACombatAiControllerBase : public AAiControllerBase
{
	GENERATED_BODY()
	
public:
	
	ACombatAiControllerBase() {};

	/* Hate List Methods */
	
	UFUNCTION(BlueprintCallable) float AddHateTowardsTarget(
		ACharacterBase* HateTarget, float HatePoints = 1.f);

	UFUNCTION(BlueprintCallable) float RemoveHateFromTarget(
		ACharacterBase* HateTarget, float HatePoints = 1.f);

	UFUNCTION(BlueprintCallable) bool ClearTargetFromHateList(
		ACharacterBase* HateTarget);

	UFUNCTION(BlueprintCallable) bool WipeHateListMemory();
	
	UFUNCTION(BlueprintPure) ACharacterBase* GetMostHatedTarget(float& HatePoints) const;

	UFUNCTION(BlueprintPure) TMap<ACharacterBase*, float> GetHateList() const { return _HateList; }
	
	UFUNCTION(BlueprintCallable) void RememberDamage(AActor* DamagingActor, float DamageValue);
	
private:
	
	UFUNCTION() void CheckCombatState(ECombatState OldCombatState, ECombatState NewCombatState);
	
protected:
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ECharacterTeam AiTeam = ECharacterTeam::ENEMY;
	
private:
	
	// Keeps a list of all hated targets by this AI
	// The current target is the actor with the highest value
	UPROPERTY() TMap<ACharacterBase*, float> _HateList;

	// Retains a running total of actors that dealt damage to this AI
	UPROPERTY() TMap<ACharacterBase*, float> _DamageList;

	UPROPERTY() ANpcCharacterBase* CharacterReference = nullptr;
	
};
