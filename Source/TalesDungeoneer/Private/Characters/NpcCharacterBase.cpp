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

	InventoryComponent->IssueStartingItems();
}

void ANpcCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	AiStimuli->RegisterForSense(UAISense_Sight::StaticClass());
	AiStimuli->RegisterWithPerceptionSystem();
}

/**
 * @brief Called when the NPC dies, spawning the loot table on the ground
 * @param MyKiller Unused in this call
 */
 void ANpcCharacterBase::DropLootTable(AActor* MyKiller)
{	
	// Destroy the NPC after the loot has dropped
	FTimerHandle DeathDeleteTimer;
	FTimerDelegate DeathDelegate;
	DeathDelegate.BindUObject(this, &ANpcCharacterBase::DestroyNpc);
	GetWorldTimerManager().SetTimer(DeathDeleteTimer, DeathDelegate, 5.f, false);
	Destroy();
}
