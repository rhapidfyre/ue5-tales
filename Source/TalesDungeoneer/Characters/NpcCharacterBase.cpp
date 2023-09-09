// Fill out your copyright notice in the Description page of Project Settings.


#include "NpcCharacterBase.h"

#include "TalesDungeoneer/lib/datastructures/LootData.h"
#include "PickupActorBase.h"
#include "Controllers/CombatAiControllerBase.h"


// Sets default values
ANpcCharacterBase::ANpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ANpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	if (IsValid(MeshMergeComponent))
		MeshMergeComponent->PerformMeshMerge();
	SetCharacterTeam(ECharacterTeam::ENEMY);
	SetCharacterLevel(StartingLevel);
	SetupLootTable();
}

void ANpcCharacterBase::SetupLootTable()
{
	// Only the server needs to consider the loot table.
	if (!HasAuthority())
		return;
	
	if (IsValid(LootTable))
	{

		// Loop through each entry of the loot table
		for (const FName LootTableRowName : LootTable->GetRowNames())
		{
			const FString ErrorCaught;
			const FStLootData* LootRowPtr = LootTable->FindRow<FStLootData>(LootTableRowName, ErrorCaught);

			if (LootRowPtr != nullptr)
			{	
				if (!LootRowPtr->ItemName.IsNone())
				{
					float RollOutChance = FMath::RandRange(0.f,1.f);
					bool DropSuccess = RollOutChance <= LootRowPtr->DropChance;
					int dropCount = 1;
					int loopIterations = LootRowPtr->MaxIterations;
					
					if (LootRowPtr->bOneChanceForAll)
					{
						dropCount = LootRowPtr->MaxIterations;
						loopIterations = 1;
					}
					
					int i = 0;

					// do-while: Executes the loot item at least once.
					// Loop terminates when MaxIterations is reached
					do
					{
						if (DropSuccess)
						{
							// If table does not contain this item, add an empty one
							if (! _LootTable.Contains(LootRowPtr->ItemName))
								_LootTable.Add(LootRowPtr->ItemName, 0);

							// Increment the existing item
							_LootTable[LootRowPtr->ItemName] += dropCount;
						}
						
						// Re-roll Chance
						RollOutChance = FMath::RandRange(0.f,1.f);
						DropSuccess = RollOutChance <= LootRowPtr->DropChance;
						i++;
						
					} while (i < loopIterations);
					
				}
			}
		}
	}

	// Listen for the death event to spawn the loot
	if (!VitalityWelfare->OnDeath.IsAlreadyBound(this, &ANpcCharacterBase::DropLootTable))
		 VitalityWelfare->OnDeath.AddDynamic(this, &ANpcCharacterBase::DropLootTable);
	
}

void ANpcCharacterBase::OnConstruction(const FTransform& Transform)
{
	// Disallow NPCs from picking up pick up actors
	InventoryComponent->bPickupItems = false;
	SetCharacterLevel(StartingLevel);
	
	// Update the pawn controller with the custom controller
	if (IsValid(AiControllerBase))
		AIControllerClass = AiControllerBase;
}

/**
 * @brief Called when the NPC dies, spawning the loot table on the ground
 * @param MyKiller Unused in this call
 */
 void ANpcCharacterBase::DropLootTable(AActor* MyKiller)
{

	// Award the Experience
	const int NpcLevel = GetCharacterLevel();
	const float ExperienceWorth = GetExperienceWorth();
	const ACombatAiControllerBase* AiController = Cast<ACombatAiControllerBase>(GetController());
	if (IsValid(AiController))
	{
		for (const TPair<ACharacterBase*, float> HatedActor : AiController->GetHateList())
		{
			if (IsValid(HatedActor.Key))
			{
				HatedActor.Key->AwardExperiencePoints(NpcLevel, ExperienceWorth);
			}
		}
	}
	
	// Disperse the Loot
	for (const TPair<FName, int> LootData : _LootTable)
	{
		FTransform SpawnTransform( GetActorLocation() );
		SpawnTransform.SetRotation(FQuat(FMath::RandRange(0.f,359.9f),
										 FMath::RandRange(0.f,359.9f),
										 FMath::RandRange(0.f,359.9f), 0.f));
		
		APickupActorBase* PickupActor = GetWorld()->SpawnActorDeferred<APickupActorBase>(
			APickupActorBase::StaticClass(),SpawnTransform);

		if (IsValid(PickupActor))
		{
			PickupActor->SpawnQuantity = LootData.Value;
			PickupActor->SetupItemFromName(LootData.Key);
			PickupActor->FinishSpawning(SpawnTransform);
		}
	}
	
	Destroy();
}
