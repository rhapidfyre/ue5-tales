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

void ANpcCharacterBase::AddHate(AActor* HatedActor, float HatePoints)
{
	if (!FMath::IsNearlyZero(HatePoints, 0.001f))
	{
		const float* OldValue = _HateList.Find(HatedActor);
		if (OldValue == nullptr)
			_HateList.Add(HatedActor, HatePoints);
		else
			_HateList.Add(HatedActor, *OldValue + HatePoints);
	}
}

void ANpcCharacterBase::RemoveHate(AActor* HatedActor, float HatePoints)
{
	if (!FMath::IsNearlyZero(HatePoints, 0.001f))
	{
		const float newValue = _HateList.Add(HatedActor, _HateList[HatedActor] - abs(HatePoints));
		if (FMath::IsNearlyZero(newValue, 0.001f))
			ResetHateList(HatedActor);
	}
}

void ANpcCharacterBase::ResetHateList(AActor* HatedActor)
{
	if (IsValid(HatedActor))
		_HateList.Remove(HatedActor);
	else
		_HateList.Empty();
}

void ANpcCharacterBase::RememberDamage(AActor* DamagingActor, float DamageValue)
{
	if (IsValid(DamagingActor))
	{
		if (!FMath::IsNearlyZero(DamageValue, 0.001f))
		{
			ACharacterBase* CharacterBase = Cast<ACharacterBase>(DamagingActor);
			if (IsValid(CharacterBase))
			{
				// Remember the damage that has occurred
				const float* OldValue = _DamagingActors.Find(DamagingActor);
				if (OldValue == nullptr)
					_DamagingActors.Add(DamagingActor, DamageValue);
				else
					_DamagingActors.Add(DamagingActor, *OldValue + DamageValue);

				// Add the damage dealer to the hate list
				AddHate(DamagingActor, abs(DamageValue) * 0.2);
			}
		}
	}
}

void ANpcCharacterBase::CheckCombatState(ECombatState OldCombatState, ECombatState NewCombatState)
{
	const bool isHateListEmpty = _HateList.IsEmpty();

	// Special behaviors for when the state makes specific changes
	switch (OldCombatState)
	{
		
	// NPC was engaged in combat
	case ECombatState::ENGAGED:
		break;
		
	// NPC was aware of a threat, or ending an engagement
	case ECombatState::ALERT:
		break;

	// NPC was incapacitated
	case ECombatState::INJURED:
		break;
		
	// NPC was recovering from combat or alertness
	case ECombatState::RECOVERY:
		break;

	// NPC was relaxed
	case ECombatState::RELAXED:
		break;
		
	default:
		break;
	}

	// If the NPC has restarted combat, keep the damage memory and hate list
	// If the NPC has returned to a recovery state, wipe memory lists
	switch (NewCombatState)
	{
	case ECombatState::ALERT:
		__fallthrough;
	case ECombatState::ENGAGED:
		return; // do nothing
	default:
		break; // wipe memory
	}
	
	ResetHateList();
	_DamagingActors.Empty();
	
}

void ANpcCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterTeam(ECharacterTeam::ENEMY);
	SetupLootTable();
	
	if (!VitalityWelfare->OnDamageTaken.IsAlreadyBound(this, &ANpcCharacterBase::RememberDamage))
		 VitalityWelfare->OnDamageTaken.AddDynamic(this, &ANpcCharacterBase::RememberDamage);
	
	if (!VitalityWelfare->OnCombatStateChanged.IsAlreadyBound(this, &ANpcCharacterBase::CheckCombatState))
		 VitalityWelfare->OnCombatStateChanged.AddDynamic(this, &ANpcCharacterBase::CheckCombatState);
	
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
	if (!VitalityWelfare->OnDeath.IsAlreadyBound(this, &ANpcCharacterBase::DropLootTable))
		 VitalityWelfare->OnDeath.AddDynamic(this, &ANpcCharacterBase::DropLootTable);
	
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

	// Award the Experience
	for (const TPair<AActor*, float> HatedActor : _HateList)
	{
		ACharacterBase* CharacterBase = Cast<ACharacterBase>(HatedActor.Key);
		if (IsValid(CharacterBase))
		{
			CharacterBase->AwardExperiencePoints(
					GetCharacterLevel(), GetExperienceWorth() );
		}
	}
	
	// Disperse the Loot
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
