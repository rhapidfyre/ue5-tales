// Fill out your copyright notice in the Description page of Project Settings.


#include "NpcCharacterBase.h"

#include "TalesDungeoneer/lib/datastructures/LootData.h"
#include "PickupActorBase.h"
#include "Controllers/CombatAiControllerBase.h"
#include "TalesDungeoneer/Gamemode/BaseFiles/TalesGameStateBase.h"


// Sets default values
ANpcCharacterBase::ANpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ANpcCharacterBase::SetNpcAsPatroller(bool NewTruthValue)
{
	bNpcPatrolsWhenIdle = NewTruthValue;
	bIsPatrollingNpc = NewTruthValue;
	OnPatrolStatusChanged.Broadcast(bIsPatrollingNpc);
}

float ANpcCharacterBase::GetDistanceFromOriginPoint() const
{
	return (GetActorLocation() - _SpawnOrigin).SquaredLength();
}

void ANpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
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

void ANpcCharacterBase::SetupEquipment()
{
	TArray<FStStartingItem> InventoryStartingItems = {};
	for (const FStNpcStartingItem StartingItem : StartingItems)
	{
		// Can this item be awarded?
		if (FMath::RandRange(0.f, 1.f) < StartingItem.ChanceToSpawn)
		{
			FStStartingItem StartItem;
			StartItem.quantity		= StartingItem.quantity;
			StartItem.startingItem	= StartingItem.startingItem;
			StartItem.equipType		= StartingItem.equipType;
			InventoryStartingItems.Add(StartItem);
		}
	}
	
	if (IsValid(InventoryComponent))
		InventoryComponent->StartingItems = InventoryStartingItems;
}

void ANpcCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// Disallow NPCs from picking up pick-up actors
	InventoryComponent->bPickupItems = false;

	// Set NPC level based on Adjusted Dungeon Level
	const ATalesGameStateBase* GameState = Cast<ATalesGameStateBase>(GetWorld()->GetGameState());
	SetCharacterLevel( !IsValid(GameState) ? 1
			: GameState->DungeonLevel + FMath::RandRange(
				0-MaximumLevelSpread,MaximumLevelSpread)
		);
	
	SetNpcAsPatroller(bNpcPatrolsWhenIdle);
	
	// Update the pawn controller with the custom controller
	if (IsValid(AiControllerBase))
		AIControllerClass = AiControllerBase;
	
	bIsPatrollingNpc = bNpcPatrolsWhenIdle;
	
	SetupEquipment(); // Determine starting equipment
	
	/* TODO To be implemented at a later, more stable version
	// Setup NPC Data, Faction & Starting Items/Equipment
	ATalesGameStateBase* GameState = Cast<ATalesGameStateBase>(GetWorld()->GetGameState());
	if (IsValid(GameState))
	{
		FStNpcData NpcData = GameState->GetNpcData(NpcDataTableRowName);
		if (!NpcData.CharacterName.IsEmpty())
		{
			SetCharacterName(NpcData.CharacterName);
			SetCharacterRace(NpcData.CharacterRace);
			SetCharacterClass(NpcData.CharacterClass);

			USkeleton* UseSkeleton = NpcData.MaleSkeleton;
			TSubclassOf<UAnimInstance> UseAnimBp = NpcData.MaleAnimationBp;
			TArray<FStMeshMergeData> UseMeshes = NpcData.MaleMeshes;
		
			if (NpcData.FemaleMeshes.Num() > 0)
			{
				if (FMath::RandBool())
				{
					UseSkeleton = IsValid(NpcData.FemaleSkeleton) ? NpcData.FemaleSkeleton : NpcData.MaleSkeleton;
					UseAnimBp   = IsValid(NpcData.FemaleAnimationBp) ? NpcData.FemaleAnimationBp : NpcData.MaleAnimationBp;
					UseMeshes   = NpcData.FemaleMeshes;
				}
			}
			
			// Mesh Merge will be called by Super
			MeshMergeComponent->Skeleton = UseSkeleton;
			MeshMergeComponent->AnimBlueprint = UseAnimBp;
			MeshMergeComponent->MeshesToMerge = UseMeshes;
		
			for (const EFaction FactionEnum : NpcData.FactionMemberships)
				SetFactionMembership(FactionEnum, true);

			for (const FStFactionDataMap DataMap : NpcData.FactionData.DataMap)
				SetFactionValue(DataMap.FactionEnum, DataMap.FactionValue);
		
			InventoryComponent->StartingItems = NpcData.StartingItems;
		}
	}
	*/
	if (IsValid(MeshMergeComponent))
		MeshMergeComponent->PerformMeshMerge();
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
	UnregisterAllComponents(false);
	Destroy();
}
