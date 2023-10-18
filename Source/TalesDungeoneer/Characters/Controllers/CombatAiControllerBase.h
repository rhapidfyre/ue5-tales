// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "TalesDungeoneer/Characters/NpcCharacterBase.h"
#include "TalesDungeoneer/Characters/Controllers/AiControllerBase.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"
#include "Delegates/Delegate.h"

#include "CombatAiControllerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHateListUpdated);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACombatAiControllerBase : public AAiControllerBase
{
	GENERATED_BODY()
	
public:
	
	ACombatAiControllerBase() {};

	UPROPERTY(BlueprintAssignable) FOnHateListUpdated OnHateListUpdated;

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

	UFUNCTION(BlueprintPure) bool IsTargetOnHateList(AActor* TargetActor);
	
	UFUNCTION(BlueprintPure) bool IsTargetValid(AActor* TargetActor);
	
protected:
	
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	UFUNCTION() void TargetPerception(AActor* StimulusActor, FAIStimulus StimulusData);
	
private:

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_HateListUpdated();
	
	UFUNCTION() void CheckCombatState(ECombatState OldCombatState, ECombatState NewCombatState);

	void SortHateList();
	
public:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	ECharacterTeam AiTeam = ECharacterTeam::ENEMY;
	
private:
	
	// Keeps a list of all targets this NPC can perceive & respond to
	UPROPERTY() TSet<ACharacterBase*> _ValidTargets;
	
	// Keeps a list of all hated targets by this AI
	// The current target is the actor with the highest value
	UPROPERTY() TMap<ACharacterBase*, float> _HateList;

	// Retains a running total of actors that dealt damage to this AI
	UPROPERTY() TMap<ACharacterBase*, float> _DamageList;

	UPROPERTY() ANpcCharacterBase* CharacterReference = nullptr;
	
};
