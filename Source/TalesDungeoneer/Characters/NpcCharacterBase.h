// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterBase.h"
#include "TalesDungeoneer/lib/datastructures/LootData.h"

#include "NpcCharacterBase.generated.h"

/**
 * Player Character Base is the base C++ class for all logic, methods and members that affect all
 * PLAYER based characters, prior to handling by child classes or dependent blueprint classes.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ANpcCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public: // functions
	
	ANpcCharacterBase();

	// Where KEY(FName) is the item name and VALUE(float) is the chance (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UDataTable* LootTable;

	/**
	 * @brief Adds hate points to the actor. For managing aggro.
	 * @param HatedActor The actor to add hate towards
	 * @param HatePoints How many hate points to add
	 */
	UFUNCTION(BlueprintCallable)
	void AddHate(AActor* HatedActor, float HatePoints = 0);
	
	/**
	 * @brief Removes hate points from the actor. For managing aggro.
	 * @param HatedActor The actor to remove hate towards
	 * @param HatePoints How many hate points to remove
	 */
	UFUNCTION(BlueprintCallable)
	void RemoveHate(AActor* HatedActor, float HatePoints = 0);
	
	/**
	 * @brief Removes an actor or all actors from the hate list
	 * @param HatedActor The actor to remove. If nullptr, removes ALL actors.
	 */
	UFUNCTION(BlueprintCallable)
	void ResetHateList(AActor* HatedActor = nullptr);

	UFUNCTION(BlueprintPure) TMap<AActor*, float> GetHateList() const { return _HateList; }
	UFUNCTION() void RememberDamage(AActor* DamagingActor, float DamageValue);

	
protected:
	
	virtual void BeginPlay() override;
	
	UFUNCTION() void CheckCombatState(ECombatState OldCombatState, ECombatState NewCombatState);

	// Generates loot when the NPC begins play, instead of calculating on death.
	// Avoids any lag spike or skip when the NPC dies
	virtual void SetupLootTable();

	virtual void OnConstruction(const FTransform& Transform) override;

private:

	UFUNCTION()
	void DropLootTable(AActor* MyKiller);

	// The items to drop upon death (KEY = Item Name, VALUE = quantity)
	UPROPERTY() TMap<FName, int> _DropLoot;

	// List of all enemies who have "aggro" on this actor
	UPROPERTY() TMap<AActor*, float> _HateList;

	// List of all enemies who have damaged this actor during this engagement
	UPROPERTY() TMap<AActor*, float> _DamagingActors;
	
};
