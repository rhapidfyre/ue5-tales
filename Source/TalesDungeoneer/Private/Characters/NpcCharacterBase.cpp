// Starcache Studios, LLC (c) 2024


#include "Characters/NpcCharacterBase.h"

#include "Characters/Controllers/CombatAiControllerBase.h"
#include "Perception/AISense_Sight.h"
#include "Logging/StructuredLog.h"
#include "DataAssets/CharacterDefaults.h"
#include "Interfaces/RsAnimInstance.h"

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
	InitializeCharacter();
	InitializeStartingItems();
}

void ANpcCharacterBase::InitializeCharacter()
{
	UCharacterRaceData* RaceData = GetCharacterRaceData();
	UCharacterClassData* ClassData = GetCharacterClassData();
	if (IsValid(CharacterData))
	{
		SetCharacterName(CharacterData->CharacterName);
	}
	if (IsValid(RaceData))
	{
		SetCharacterRace(RaceData->GameplayTag);
	}
	if (IsValid(ClassData))
	{
		SetCharacterClass(ClassData->GameplayTag);
	}
	UpdateCoreStats();
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

void ANpcCharacterBase::CharacterDeath_Internal()
{
	Super::CharacterDeath_Internal();
	if (HasAuthority())
	{
		if (AAIController* AiController = GetController<AAIController>())
		{
			if (AiController->BrainComponent)
			{
				AiController->BrainComponent->StopLogic("Death");
			}
			AiController->Destroy();
		}
	}
}

void ANpcCharacterBase::CharacterRevived_Internal()
{
	Super::CharacterRevived_Internal();
}

/**
 * \brief Called when the NPC dies, spawning the loot table on the ground
 * \param MyKiller Unused in this call
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
