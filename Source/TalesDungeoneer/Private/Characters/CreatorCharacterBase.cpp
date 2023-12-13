// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "CreatorCharacterBase.h"

#include "TalesDungeoneer/Gamemode/BaseFiles/TalesGameStateBase.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACreatorCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ACreatorCharacterBase::BeginPlay()
{
	ATalesGameStateBase* gState = Cast<ATalesGameStateBase>( GetWorld()->GetGameState() );
	if (IsValid(gState))
	{
		InventoryComponent->StartingItems =
			gState->GetStartingInventoryData(
				GetCharacterRace(), GetCharacterClass());
		
		InventoryComponent->NumberOfInvSlots = gState->DefaultNumOfInventorySlots;
		InventoryComponent->EligibleEquipmentSlots = gState->DefaultEquipmentSlots;
	}
	Super::BeginPlay();
}

// Overrides the LoadSaveData of the APlayerCharacterBase
//	by saving default values instead of current values
void ACreatorCharacterBase::LoadSaveData(
		const FString& SaveName, const int32 UserIndex, USaveGame* SaveData)
{
	//Super::LoadSaveData(SaveName, UserIndex, SaveData);
	ATalesGameStateBase* gState = Cast<ATalesGameStateBase>( GetWorld()->GetGameState() );
	if (IsValid(gState))
	{
		// Dissect the save data and pass it to the server, if applicable
		const USavedCharacter* CharacterData = Cast<USavedCharacter>(SaveData);
		if (IsValid(CharacterData))
		{
			Server_InitializeCharacter(
				CharacterData->CharacterName,
				CharacterData->CharacterLevel,
				CharacterData->CharacterRace,
				CharacterData->CharacterClass,
				CharacterData->ExperiencePoints	);
			
			Server_SetupMeshMerge(CharacterData->MeshesToMerge,
				CharacterData->MeshSectionMappings,
				CharacterData->UvTransformsPerMesh);
	
			// Re-initialize Vitality Component Data
			if (IsValid(VitalityWelfare))
			{
				VitalityWelfare->InitializeHealthSubsystem(
					CharacterData->UseHealthSubsystem,
					CharacterData->StartingHealthCurrent,
					CharacterData->StartingHealthMaximum,
					CharacterData->PassiveHealthRegen);
				
				VitalityWelfare->InitializeStaminaSubsystem(
					CharacterData->UseStaminaSubsystem,
					CharacterData->StartingStaminaCurrent,
					CharacterData->StartingStaminaMaximum,
					CharacterData->PassiveStaminaRegen);
					
				VitalityWelfare->InitializeMagicSubsystem(
					CharacterData->UseMagicSubsystem,
					CharacterData->StartingMagicCurrent,
					CharacterData->StartingMagicMaximum,
					CharacterData->PassiveMagicRegen);
						
				VitalityWelfare->InitializeSurvivalSubsystem(
					CharacterData->UseSurvivalSubsystem,
					CharacterData->StartingHydrationCurrent,
					CharacterData->StartingHydrationMaximum,
					CharacterData->PassiveHydrationDrain,
					CharacterData->StartingHungerCurrent,
					CharacterData->StartingHungerMaximum,
					CharacterData->PassiveHungerDrain);
			}
			
			if (IsValid(VitalityStats))
			{
				// Restore Natural Stats
				VitalityStats->InitializeCoreStats(
					CharacterData->BaseStats.GetCoreStatValue(EVitalityStat::STRENGTH),
					CharacterData->BaseStats.GetCoreStatValue(EVitalityStat::AGILITY),
					CharacterData->BaseStats.GetCoreStatValue(EVitalityStat::FORTITUDE),
					CharacterData->BaseStats.GetCoreStatValue(EVitalityStat::INTELLECT),
					CharacterData->BaseStats.GetCoreStatValue(EVitalityStat::ASTUTENESS),
					CharacterData->BaseStats.GetCoreStatValue(EVitalityStat::CHARISMA));
				
				// Restore Natural Damage Bonus & Resistance
				VitalityStats->InitializeNaturalDamageBonuses(CharacterData->BaseStats.DamageBonuses);
				VitalityStats->InitializeNaturalDamageResists(CharacterData->BaseStats.DamageResists);
			}

			if (IsValid(InventoryComponent))
			{
				InventoryComponent->RestoreInventory(
					InventoryComponent->CopyInventorySlots(),
					InventoryComponent->CopyEquipmentSlots());
			}
	
			// Restore unlock points
			if (IsValid(AbilityComponent))
			{
				AbilityComponent->InitializePoints(0);
			}
			
			// Restore Active Effects
			if (IsValid(VitalityStats))
			{
				VitalityEffects->InitializeEffects(
					gState->GetStartingEffects(GetCharacterRace(), GetCharacterClass()));
			}
			
			UE_LOG(LogTemp, Display, TEXT("LoadSaveData(%s): Successfully restored character from Save Slot '%s'"),
				HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *SaveName);
			CharacterRestoredFromSave(SaveName);
		}
	}
}
