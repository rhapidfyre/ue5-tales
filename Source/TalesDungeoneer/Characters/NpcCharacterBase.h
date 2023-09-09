// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CharacterBase.h"
#include "Controllers/AiControllerBase.h"

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

	// Used to set the starting level for this NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Settings")
	int StartingLevel = 1;

	
protected:
	
	virtual void BeginPlay() override;

	// Generates loot when the NPC begins play, instead of calculating on death.
	// Avoids any lag spike or skip when the NPC dies
	virtual void SetupLootTable();

	virtual void OnConstruction(const FTransform& Transform) override;

private:

	UFUNCTION()
	void DropLootTable(AActor* MyKiller);

	// The items to drop upon death (KEY = Item Name, VALUE = quantity)
	UPROPERTY() TMap<FName, int> _LootTable;

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AAiControllerBase> AiControllerBase = AAiControllerBase::StaticClass();
	
};
