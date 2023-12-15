// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NpcCharacterBase.h"

#include "lib/datastructures/LootData.h"
#include "PickupActorBase.h"
#include "Characters/Controllers/CombatAiControllerBase.h"
#include "Perception/AISense_Sight.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "Components/CapsuleComponent.h"
#include "Logging/StructuredLog.h"


// Sets default values
ANpcCharacterBase::ANpcCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	if (!IsValid(AiStimuli))
	{
		AiStimuli = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("AiStimuli");
		AiStimuli->bAutoRegister = true;
	}
	
	// Disallow NPCs from picking up pick-up actors
	InventoryComponent->bCanPickUpItems = false;
	
}

float ANpcCharacterBase::GetDistanceFromOriginPoint() const
{
	return (GetActorLocation() - _SpawnOrigin).SquaredLength();
}

void ANpcCharacterBase::DestroyNpc()
{
	UnregisterAllComponents(false);
	Destroy();
}

void ANpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeStartingItems();
}

void ANpcCharacterBase::InitializeStartingItems()
{
	if (!HasAuthority())
	{
		UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}): InitializeStartingItems() cannot be executed on the client.",
			GetName(), HasAuthority()?"SV":"CL");
		return;
	}

	TArray<FStStartingItem> InventoryStartingItems = {};

	// TODO - Add NPC Starting Items using Data Assets
	
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}): Spawning with {NumStartItems} items",
		GetName(), HasAuthority()?"SV":"CL", InventoryStartingItems.Num());
	
	InventoryComponent->IssueStartingItems();
}

void ANpcCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	AiStimuli->RegisterForSense(UAISense_Sight::StaticClass());
	AiStimuli->RegisterWithPerceptionSystem();
	
	if (IsValid(MeshMergeComponent))
	{
		MeshMergeComponent->PerformMeshMerge();
	}
}

/**
 * @brief Called when the NPC dies, spawning the loot table on the ground
 * @param MyKiller Unused in this call
 */
 void ANpcCharacterBase::DropLootTable(AActor* MyKiller)
{

	// Award the Experience
	/*
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
	*/
	
	// Disperse the Loot
	const TArray<FStInventorySlot> InventorySlots = InventoryComponent->GetCopyOfAllSlots(true);

	// Determine what items and how many of each are dropped
	TArray<FStNpcLootItem> ItemsToDrop = {};
	for (const FStInventorySlot& SlotCopy : InventorySlots)
	{
		if (!SlotCopy.IsSlotEmpty())
		{
			if (SlotCopy.GetMaxStackAllowance() > 1)
			{
				// Add to existing item if it exists
				for (int i = 0; i < ItemsToDrop.Num(); i++)
				{
					ItemsToDrop[i].Quantity += SlotCopy.SlotQuantity;
				}
			}
			// Not Stackable; Add whole new item.
			else
			{
				ItemsToDrop.Add(FStNpcLootItem(SlotCopy.ItemName, SlotCopy.SlotQuantity));
			}
		}
	}

	// Spawn the pick up actors
	for (const FStNpcLootItem LootItem : ItemsToDrop)
	{
		FTransform SpawnTransform( GetActorLocation() );
		SpawnTransform.SetRotation(FQuat(FMath::RandRange(0.f,359.9f),
										 FMath::RandRange(0.f,359.9f),
										 FMath::RandRange(0.f,359.9f), 0.f));
		
		APickupActorBase* PickupActor = GetWorld()->SpawnActorDeferred<APickupActorBase>(
			APickupActorBase::StaticClass(),SpawnTransform);

		if (IsValid(PickupActor))
		{
			PickupActor->SpawnQuantity = LootItem.Quantity;
			PickupActor->SetupItemFromName(LootItem.ItemName);
			PickupActor->FinishSpawning(SpawnTransform);
		}
	}

	// Destroy the NPC after the loot has dropped
	FTimerHandle DeathDeleteTimer;
	FTimerDelegate DeathDelegate;
	DeathDelegate.BindUObject(this, &ANpcCharacterBase::DestroyNpc);
	GetWorldTimerManager().SetTimer(DeathDeleteTimer, DeathDelegate, 5.f, false);
	Destroy();
}
