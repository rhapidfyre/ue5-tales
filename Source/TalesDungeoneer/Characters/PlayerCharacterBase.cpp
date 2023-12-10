// Copyright Take Five Games, LLC 2023 - All rights reserved


#include "PlayerCharacterBase.h"

#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
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

	// Dedicated Servers never have character save data to load
	if (NetMode == NM_DedicatedServer) return;
	
	const bool isControlledByLocalPlayer =
		GetController() == Cast<AController>(UGameplayStatics::GetPlayerController(GetWorld(),0));
	
	// Dissect the save data and pass it to the server, if applicable
	const USavedCharacter* CharacterData = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterData))
	{
		// Only the player who is controlling the character can submit save data
		// If the player is also the server, this will still work
		if (isControlledByLocalPlayer)
		{
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
				FString ResponseString = "";
				if (!InventoryComponent->LoadInventory(
					ResponseString,	CharacterData->SavedInventory, true))
				{
					UE_LOGFMT(LogTemp, Warning,
						"Failed to Restore Saved Inventory. Reason: {ResponseStr}", ResponseString);
				}
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
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LoadSaveData(%s): Could not find character Save Slot '%s'"),
			HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *SaveName);
	}
}

/** Saves the character's components and internal data to file.
 * Runs SYNCHRONOUSLY. If you want this to run async, it needs to be called
 * by something that is running on the worker thread.
 */
bool APlayerCharacterBase::SaveCharacterData()
{
	// Create a new save, or reload an existing save file
	USaveGame* SaveData;
	if (!UGameplayStatics::DoesSaveGameExist(CharacterSaveName, CharacterSaveIndex))
		SaveData = UGameplayStatics::CreateSaveGameObject( USavedCharacter::StaticClass() );
	else
		SaveData = UGameplayStatics::LoadGameFromSlot(CharacterSaveName, CharacterSaveIndex);

	// If the save is valid, collect all of the data we want to save
	USavedCharacter* CharacterSave = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterSave))
	{
		// Set Character Persona Data
		CharacterSave->CharacterName  		= GetCharacterName();
		CharacterSave->CharacterLevel 		= GetCharacterLevel();
		CharacterSave->CharacterRace  		= GetCharacterRace();
		CharacterSave->CharacterClass 		= GetCharacterClass();
		CharacterSave->ExperiencePoints		= GetExperiencePoints();
		
		// Save Mesh Mesh Data
		CharacterSave->Skeleton            = MeshMergeComponent->Skeleton;
		CharacterSave->MeshSectionMappings = MeshMergeComponent->MeshSectionMappings;
		CharacterSave->UvTransformsPerMesh = MeshMergeComponent->UvTransformsPerMesh;
		CharacterSave->MeshesToMerge       = MeshMergeComponent->MeshesToMerge;

		// Save Vitality Data
		if (IsValid(VitalityWelfare))
		{
			// Set initial values
			CharacterSave->UseHealthSubsystem		= VitalityWelfare->UseHealthSubsystem;
			CharacterSave->UseStaminaSubsystem		= VitalityWelfare->UseStaminaSubsystem;
			CharacterSave->UseMagicSubsystem		= VitalityWelfare->UseMagicSubsystem;
			CharacterSave->UseSurvivalSubsystem		= VitalityWelfare->UseSurvivalSubsystem;
			
			float CurrentHealth, MaximumHealth;
			VitalityWelfare->GetCurrentHealth(CurrentHealth, MaximumHealth);
			CharacterSave->StartingHealthCurrent	= CurrentHealth;
			CharacterSave->StartingHealthMaximum	= MaximumHealth;
			CharacterSave->PassiveHealthRegen		= VitalityWelfare->PassiveHealthRegen;
			CharacterSave->HealthTimerTickRate		= VitalityWelfare->HealthTimerTickRate;
			
			float CurrentStamina, MaximumStamina;
			VitalityWelfare->GetCurrentStamina(CurrentStamina, MaximumStamina);
			CharacterSave->StartingStaminaCurrent	= CurrentStamina;
			CharacterSave->StartingStaminaMaximum	= MaximumStamina;
			CharacterSave->PassiveStaminaRegen		= VitalityWelfare->PassiveStaminaRegen;
			CharacterSave->StaminaTimerTickRate		= VitalityWelfare->StaminaTimerTickRate;
			
			float CurrentMagic, MaximumMagic;
			VitalityWelfare->GetCurrentMagic(CurrentMagic, MaximumMagic);
			CharacterSave->StartingMagicCurrent		= CurrentMagic;
			CharacterSave->StartingMagicMaximum		= MaximumMagic;
			CharacterSave->PassiveMagicRegen		= VitalityWelfare->PassiveMagicRegen;
			CharacterSave->MagicTimerTickRate		= VitalityWelfare->MagicTimerTickRate;
			
			float CurrentHydration, CurrentCalories, MaximumHydration, MaximumCalories;
			VitalityWelfare->GetCurrentHydration(CurrentHydration, MaximumHydration);
			VitalityWelfare->GetCurrentHunger(CurrentCalories, MaximumCalories);
			CharacterSave->StartingHydrationCurrent	= CurrentHydration;
			CharacterSave->StartingHungerCurrent	= CurrentCalories;
			CharacterSave->StartingHydrationMaximum	= MaximumHydration;
			CharacterSave->StartingHungerMaximum	= MaximumCalories;
			
			CharacterSave->PassiveHydrationDrain	= VitalityWelfare->PassiveHydrationDrain;
			CharacterSave->PassiveHungerDrain		= VitalityWelfare->PassiveHungerDrain;
			CharacterSave->HydrationTimerTickRate	= VitalityWelfare->HydrationTimerTickRate;
			CharacterSave->CaloriesTimerTickRate	= VitalityWelfare->CaloriesTimerTickRate;
		}
		
		if (IsValid(AbilityComponent))
			CharacterSave->UnlockPointsAvailable = AbilityComponent->GetNumberOfUnlockPoints();

		// The inventory component saves internally.
		// We just need to remember the name of the save file to restore it.
		if (IsValid(InventoryComponent))
		{
			// If the inventory is SAVED before it is LOADED, it will create a new save file.
			// This should only happen on new characters.
			FString ResponseString = "";
			FString InvSaveName = InventoryComponent->SaveInventory(ResponseString, true);
			
			if (!InvSaveName.IsEmpty())
				CharacterSave->SavedInventory = InvSaveName;
			else
			{
				UE_LOGFMT(LogTemp, Error,
					"Failed to Save Inventory of '{cName}'. Reason: {ResponseStr}",
					GetCharacterName(), ResponseString);
			}
		}
		
		// Restore Natural Stats
		if (IsValid(VitalityStats))
		{
			CharacterSave->BaseStats = VitalityStats->GetAllNaturalStats();
		}

		// Restore Active Effects
		if (IsValid(VitalityEffects))
		{
			CharacterSave->SavedEffects = VitalityEffects->GetAllActiveEffects();
		}
		
		// Save the version of the game when this character was saved
		CharacterSave->SaveVersion = UGlobalData::GetAppVersion();

		// Performs the actual hard file save
		UGameplayStatics::SaveGameToSlot(CharacterSave,
					CharacterSaveName, CharacterSaveIndex);
		
		return true;
	}
	return false;
}

/** Loads an existing character and the characters components.
 * Runs SYNCHRONOUSLY, so if you want this to run Async, you need to call it
 * from the same async call.
 */
bool APlayerCharacterBase::LoadCharacterData(const FString SaveSlotName, const int32 UserIndex)
{
	if (Super::LoadCharacterData(SaveSlotName, UserIndex))
	{
		// Make sure the character isn't being loaded by a dedicated server,
		// nor is it being saved by a different player.
		if (
			GetController() ==
			Cast<AController>(UGameplayStatics::GetPlayerController(GetWorld(),0)))
		{
			if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, UserIndex))
			{
				CharacterSaveName = SaveSlotName;
				CharacterSaveIndex = UserIndex;
				USaveGame* SaveData =
					UGameplayStatics::LoadGameFromSlot(CharacterSaveName, CharacterSaveIndex);
				if (IsValid(SaveData))
				{
				
				}
			}
		}
	}
	return false;
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
		if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
		{
			USavedCharacter* SavedCharacter = Cast<USavedCharacter>(
				UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
			if (IsValid(SavedCharacter))
				LoadSaveData(SaveSlotName, 0, SavedCharacter);
		}
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

void APlayerCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
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
			// Run async initialization of game state to reload character data
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
	