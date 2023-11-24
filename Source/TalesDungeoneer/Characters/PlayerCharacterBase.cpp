// Copyright Take Five Games, LLC 2023 - All rights reserved


#include "PlayerCharacterBase.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
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

	const ENetMode NetMode = GetNetMode();
	if (NetMode == NM_DedicatedServer) return;

	// Dissect the save data and pass it to the server, if applicable
	const USavedCharacter* CharacterData = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterData))
	{
		// Set Character Persona Data (Works on client or server)
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
		
		UE_LOG(LogTemp, Display, TEXT("LoadSaveData(%s): Successfully restored character from Save Slot '%s'"),
			HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *SaveName);
		CharacterRestoredFromSave(SaveName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoadSaveData(%s): Could not find character Save Slot '%s'"),
			HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *SaveName);
	}
}

void APlayerCharacterBase::AwaitGameState()
{
	const AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GameStateBase);
	
	if (IsValid(TalesGameState))
	{
		while (!TalesGameState->GetIsSaveMetaReady())
		{
			
		}
		const FString SaveSlotName = TalesGameState->GetSelectedCharacterSaveSlotName();
		USavedCharacter* SavedCharacter = Cast<USavedCharacter>(
			UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		if (IsValid(SavedCharacter))
			LoadSaveData(SaveSlotName, 0, SavedCharacter);
	}
	
	// Re-fire if game state isn't valid
	// This shouldn't happen but it stops any potential initialization issues
	else
	{
		FTimerHandle TimerReference;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &APlayerCharacterBase::AwaitGameState);
		GetWorld()->GetTimerManager().SetTimer(TimerReference,	TimerDelegate,
			1, false);
	}
	
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetCharacterTeam(ECharacterTeam::PLAYER);
	const ENetMode NetMode = GetNetMode();
	if (NetMode == NM_Client || NetMode == NM_ListenServer || NetMode == NM_Standalone)
	{
		const AGameStateBase* GameStateBase = GetWorld()->GetGameState();
		const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GameStateBase);
		if (IsValid(TalesGameState))
		{
			// Run async initialization of game state
			FTimerHandle TimerReference;
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(this, &APlayerCharacterBase::AwaitGameState);
			GetWorld()->GetTimerManager().SetTimer(TimerReference, TimerDelegate,
				1, false);
		}
	}
	OnPlayerJoined.Broadcast();
}

void APlayerCharacterBase::Server_SetupMeshMerge_Implementation(
	const TArray<FStMeshMergeData>& MeshesToMerge,
	const TArray<FSkelMeshMergeSectionMapping>& MeshSectionMappings,
	const TArray<FSkelMeshMergeUVTransformMapping>& UvTransformsPerMesh)
{
	if (IsValid(MeshMergeComponent))
	{
		MeshMergeComponent->MeshesToMerge		= MeshesToMerge;
		MeshMergeComponent->MeshSectionMappings = MeshSectionMappings;
		MeshMergeComponent->UvTransformsPerMesh = UvTransformsPerMesh;
		MeshMergeComponent->PerformMeshMerge();
	}
}

void APlayerCharacterBase::Server_InitializeCharacter_Implementation(
	const FString& NewName, int NewLevel,
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
		bHasInitialized = true; // Prevents the clients from injecting
	}
}
	