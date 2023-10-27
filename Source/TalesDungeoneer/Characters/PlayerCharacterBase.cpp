// Copyright Take Five Games, LLC 2023 - All rights reserved


#include "PlayerCharacterBase.h"

#include "Kismet/GameplayStatics.h"
#include "TalesDungeoneer/Gamemode/BaseFiles/TalesGameStateBase.h"
#include "TalesDungeoneer/Saves/SavedCharacters.h"


// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

/**
 * Restores the character from a local save, publishing to the server
 * @param SaveName The string name of the save location
 * @param UserIndex Should be zero
 * @param SaveData The actual save data
 */
void APlayerCharacterBase::LoadSaveData(const FString& SaveName, const int32 UserIndex, USaveGame* SaveData)
{
	Super::LoadSaveData(SaveName, UserIndex, SaveData);
	const USavedCharacter* CharacterData = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterData))
	{
		// Set Character Persona Data
		UE_LOG(LogTemp, Display, TEXT("LoadSaveData(%s): %s, Lv. %d %s %s"),
			GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"),
			*CharacterData->CharacterName, CharacterData->CharacterLevel,
			*UEnum::GetValueAsString(CharacterData->CharacterRace),
			*UEnum::GetValueAsString(CharacterData->CharacterClass));
		Server_InitializeCharacter(
			CharacterData->CharacterName,
			CharacterData->CharacterLevel,
			CharacterData->CharacterRace,
			CharacterData->CharacterClass,
			CharacterData->ExperiencePoints	);

		// Restore Character Design
		if (IsValid(MeshMergeComponent))
		{
			MeshMergeComponent->InitializeMeshMerge(CharacterData);
		}

		// Reinitialize Vitality Component Data
		// Must occur before equipment or vitality stats will be incorrect
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
				CharacterData->Strength, CharacterData->Agility,
				CharacterData->Fortitude, CharacterData->Intellect,
				CharacterData->Astuteness, CharacterData->Charisma);
			
			// Restore Natural Damage Bonus & Resistance
			VitalityStats->InitializeNaturalDamageBonuses(CharacterData->BaseStats.DamageBonuses);
			VitalityStats->InitializeNaturalDamageResists(CharacterData->BaseStats.DamageResistances);
		}

		// Restore unlock points
		if (IsValid(AbilityComponent))
		{
			AbilityComponent->InitializePoints(CharacterData->UnlockPointsAvailable);
		}
		
		// Restore Active Effects
		if (IsValid(VitalityStats))
		{
			VitalityEffects->InitializeEffects(CharacterData->SavedEffects);
		}
		
		UE_LOG(LogTemp, Display, TEXT("LoadSaveData(): Successfully restored character from Save Slot '%s'"),
			*SaveName);
		CharacterRestoredFromSave(SaveName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoadSaveData(): Could not find character Save Slot '%s'"),
			*SaveName);
	}
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterTeam(ECharacterTeam::PLAYER);
	const AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GameStateBase);
	if (IsValid(TalesGameState))
	{
		const FString SaveSlotName = TalesGameState->GetSelectedCharacterSaveSlotName();
		USavedCharacter* SavedCharacter = Cast<USavedCharacter>(
			UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		
		if (IsValid(SavedCharacter))
			LoadSaveData(SaveSlotName, 0, SavedCharacter);
		
	}
	OnPlayerJoined.Broadcast();
}

void APlayerCharacterBase::Server_InitializeCharacter_Implementation(const FString& NewName, int NewLevel,
	ECharacterRace NewRace, ECharacterClass NewClass, float NewExperience)
{
	if (bHasInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeCharacter(%s): Already Initialized!"),
			GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		return;
	}
	
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Display, TEXT("InitializeCharacter(%s): %s, Lv. %d %s %s"),
			GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"),
			*NewName, NewLevel,	*UEnum::GetValueAsString(NewRace),
			*UEnum::GetValueAsString(NewClass));
		SetCharacterName(NewName);
		SetCharacterLevel(NewLevel);
		SetCharacterRace(NewRace);
		SetCharacterClass(NewClass);
		SetExperiencePoints(NewExperience);
	}
}
