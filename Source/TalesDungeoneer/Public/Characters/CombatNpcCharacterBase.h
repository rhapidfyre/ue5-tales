// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "NpcCharacterBase.h"

#include "CombatNpcCharacterBase.generated.h"

/* A combat NPC is any NPC character whose primary purpose is for combat. The
 * NPCs team and sensory data settings determine who the NPC fights.
 */
UCLASS()
class TALESDUNGEONEER_API ACombatNpcCharacterBase : public ANpcCharacterBase
{
	GENERATED_BODY()

public:
	
	ACombatNpcCharacterBase();
	
	virtual void Tick(float DeltaTime) override;

	// Checks if this NPC can perform an attack, or needs to wait
	UFUNCTION() bool NpcCanAttemptAttack();

	// Checks if this NPC can perform an ability, or needs to wait
	UFUNCTION() bool NpcCanActivateAbility();

	// Time added to the weapon delay to add variety to NPC attacks
	// 0: Minimum Time, 1: Maximum Time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat NPC Settings")
	TArray<float> TimeBetweenAttacks = {2.f, 3.f};

	// The absolute time (in approx. sec) between ability activations
	// X: Minimum Time, Y: Maximum Time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat NPC Settings")
	TArray<float> TimeBetweenAbilities = {6.f, 10.f};

	// Chance of synergy activation when ability activation is chosen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat NPC Settings")
	float SynergyChance = 0.33;

	UFUNCTION(BlueprintCallable)
	virtual void PerformAttack(EWeaponSlots WeaponSlot) override;

protected:

	virtual void BeginPlay() override;

	UFUNCTION()	virtual void ProcessPrimaryAttack();

	UFUNCTION()	virtual void ProcessSecondaryAttack();

private:

	FTimerHandle _AttackTimer = {};

};
