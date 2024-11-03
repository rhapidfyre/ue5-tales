// Starcache Studios, LLC (c) 2024

#pragma once

#include "CharacterBase.h"
#include "Controllers/AiControllerBase.h"
#include "Delegates/Delegate.h"
#include "Engine/DataTable.h"

#include "NpcCharacterBase.generated.h"


// An item added to the NPCs inventory to be awarded upon death
USTRUCT(BlueprintType)
struct FStNpcStartingItem : public FTableRowBase
{
	GENERATED_BODY()
	// If TRUE, this item will be dropped from the NPC when it dies
	// If FALSE, this item will be deleted when the NPC dies
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool DroppedOnDeath	= false;
	// If set, the NPC will equip this item if they are able to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEquipOnSpawn	= false;
	// The item to be spawned when the NPC spawns
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FDataTableRowHandle ItemData = FDataTableRowHandle();
	// Percent change that the NPC spawns with this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ChanceToSpawn	= 0.f;
	// If TRUE, Chance applies to each quantity Min thru Max
	// If FALSE, Chance applies once and a random quantity Min to Max is chosen
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bRollChanceEach = false;
	// The minimum amount this item should spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int DropQuantityMin = 0;
	// The maximum amount this item should spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int DropQuantityMax = 0;
	// If this item is generated, it prevents any of the items in this array from spawning
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> IncompatibleItems = {};
};

USTRUCT(BlueprintType)
struct FStNpcLootItem
{
	GENERATED_BODY();
	FStNpcLootItem() : ItemName(FName()), Quantity(0) {};
	FStNpcLootItem(FName iName, int iQuantity = 1)
		{ ItemName = iName; Quantity = iQuantity; }
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int   Quantity;
};


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

	UFUNCTION(BlueprintPure) float GetDistanceFromOriginPoint() const;

	UFUNCTION() void DestroyNpc();

protected:

	virtual void BeginPlay() override;

	virtual void InitializeCharacter();

	// These are the items the NPC will have, including loot.
	virtual void InitializeStartingItems();

	virtual void OnConstruction(const FTransform& Transform) override;


public:

	// These items will be added to the NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	TArray<FDataTableRowHandle> StartingItems = {};

	// If specified, the NPC will choose items from the specified data tables.
	// This can be used with, or in place of, StartingItems and LootItemSetup.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	TArray<UDataTable*> LootTables = {};

	// The minimum level variance from the adjusted dungeon level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	int MinimumLevelSpread = 0;

	// The maximum level variance from the adjusted dungeon level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	int MaximumLevelSpread = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	TSubclassOf<AAiControllerBase> AiControllerBase = AAiControllerBase::StaticClass();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	UAIPerceptionStimuliSourceComponent* AiStimuli = nullptr;

private:

	UFUNCTION()
	void DropLootTable(AActor* MyKiller);

	bool bIsPatrollingNpc = false;

	// The coordinates where this NPC spawned
	FVector _SpawnOrigin = FVector(0.f);

};
