// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterBase.h"
#include "Controllers/AiControllerBase.h"
#include "Delegates/Delegate.h"

#include "NpcCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatrolStatusChanged, bool, IsPatrollingNpc);

USTRUCT(BlueprintType)
struct FStNpcStartingItem : public FStStartingItem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ChanceToSpawn = 1.f;
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

	UFUNCTION(BlueprintPure) bool GetNpcIsPatroller() const { return bIsPatrollingNpc; }

	UFUNCTION(BlueprintCallable) void SetNpcAsPatroller(bool NewTruthValue = true);

	UFUNCTION(BlueprintPure) float GetDistanceFromOriginPoint() const;

	UFUNCTION() void DestroyNpc();
	
protected:
	
	virtual void BeginPlay() override;

	// Generates loot when the NPC begins play, instead of calculating on death.
	// Avoids any lag spike or skip when the NPC dies
	virtual void SetupLootTable();

	// Issues the starting equipment for this character
	virtual void SetupEquipment();

	virtual void OnConstruction(const FTransform& Transform) override;


public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ECharacterClass> EligibleClasses = {ECharacterClass::ANY};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ECharacterRace> EligibleRaces = {ECharacterRace::ANY};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FStNpcStartingItem> StartingItems = {};
	
	UPROPERTY(BlueprintAssignable) FOnPatrolStatusChanged OnPatrolStatusChanged;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAiControllerBase> AiControllerBase = AAiControllerBase::StaticClass();

	// Where KEY(FName) is the item name and VALUE(float) is the chance (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UDataTable* LootTable = nullptr;

	// The minimum level variance from the adjusted dungeon level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	int MinimumLevelSpread = 0;
	
	// The maximum level variance from the adjusted dungeon level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	int MaximumLevelSpread = 3;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	FName NpcDataTableRowName = FName();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	bool bNpcPatrolsWhenIdle = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Npc Settings")
	UAIPerceptionStimuliSourceComponent* AiStimuli = nullptr;
	
private:

	UFUNCTION()
	void DropLootTable(AActor* MyKiller);

	// The items to drop upon death (KEY = Item Name, VALUE = quantity)
	UPROPERTY() TMap<FName, int> _LootTable;

	bool bIsPatrollingNpc = false;

	// The coordinates where this NPC spawned
	FVector _SpawnOrigin = FVector(0.f);
	
};
