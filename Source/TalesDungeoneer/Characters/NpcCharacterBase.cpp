// Fill out your copyright notice in the Description page of Project Settings.


#include "NpcCharacterBase.h"

#include "PickupActorBase.h"


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
	SetCharacterTeam(ECharacterTeam::ENEMY);
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

			if (LootRowPtr == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("%s(%s): ERROR CAUGHT init loot -> %s"),
					*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *ErrorCaught);
			}
			
			else
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
							if (! _DropLoot.Contains(LootRowPtr->ItemName))
								_DropLoot.Add(LootRowPtr->ItemName, 0);

							// Increment the existing item
							_DropLoot[LootRowPtr->ItemName] += dropCount;
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
	if (!VitalityComponent->OnDeath.IsAlreadyBound(this, &ANpcCharacterBase::DropLootTable))
		 VitalityComponent->OnDeath.AddDynamic(this, &ANpcCharacterBase::DropLootTable);
	
}

void ANpcCharacterBase::OnConstruction(const FTransform& Transform)
{
	// Disallow NPCs from picking up pick up actors
	InventoryComponent->bPickupItems = false;
}

/**
 * @brief Called when the NPC dies, spawning the loot table on the ground
 * @param MyKiller Unused in this call
 */
 void ANpcCharacterBase::DropLootTable(AActor* MyKiller)
{
	for (const TPair<FName, int> LootData : _DropLoot)
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
