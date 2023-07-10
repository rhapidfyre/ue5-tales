// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "LootData.generated.h"


USTRUCT(BlueprintType, Blueprintable)
struct TALESDUNGEONEER_API FStLootData : public FTableRowBase
{
	GENERATED_BODY()
	
	// The name of the item (From FStItemData table) to award
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemName = FName();

	// The maximum number of this item that can be dropped
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int MaxIterations = 1;

	// The chance of this item to drop, per iteration
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DropChance = 0.05;

	// If true, the DropChance will apply once to ALL MaxIterations
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bOneChanceForAll = false;

	// If this item drops, the other rows in this loot table by these names will be ignored
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> IgnoredItems = {};
	
};
