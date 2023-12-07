// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "lib/StatusEffects.h"
#include "lib/VitalityData.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"
#include "TalesDungeoneer/Characters/Components/MeshMergeComponent.h"
#include "lib/InventorySlot.h"

#include "SavedCharacters.generated.h"

struct FStCharacterStats;

/**
 * The object used for saving all PLAYER character data
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API USavedCharacter : public USaveGame
{
	GENERATED_BODY()
	
public:

	USavedCharacter() {};
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	FString SaveVersion = "v?";
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	FString SaveSlotName = TEXT("QuickName");
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	FString CharacterName = "Invalid Character";
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	int CharacterLevel = 1;
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	float ExperiencePoints = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	ECharacterClass CharacterClass = ECharacterClass::ANY;
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	ECharacterRace CharacterRace = ECharacterRace::ANY;

	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	uint32 UserIndex = 0;

	UPROPERTY(VisibleAnywhere, Category = "Mesh Merge Save Data")
	USkeleton* Skeleton = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Mesh Merge Save Data")
	TArray < FStMeshMergeData > MeshesToMerge = {};
	
	UPROPERTY(VisibleAnywhere, Category = "Mesh Merge Save Data")
	TArray < FSkelMeshMergeSectionMapping > MeshSectionMappings = {};
	
	UPROPERTY(VisibleAnywhere, Category = "Mesh Merge Save Data")
	TArray < FSkelMeshMergeUVTransformMapping > UvTransformsPerMesh = {};

	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") bool  UseHealthSubsystem = true;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	bool  UseStaminaSubsystem = true;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	bool  UseMagicSubsystem = true;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	bool  UseSurvivalSubsystem = true;
	
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingHealthCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingHealthMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float PassiveHealthRegen = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float HealthTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingStaminaCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingStaminaMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float PassiveStaminaRegen = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StaminaTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingMagicCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingMagicMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float PassiveMagicRegen = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float MagicTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingHydrationCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingHungerCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingHydrationMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float StartingHungerMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float PassiveHydrationDrain = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float PassiveHungerDrain = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float HydrationTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data")	float CaloriesTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") FStVitalityStats BaseStats;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") TArray<FStVitalityEffects> SavedEffects;

	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") int Strength = 0;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") int Agility = 0;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") int Fortitude = 0;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") int Intellect = 0;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") int Astuteness = 0;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") int Charisma = 0;
	
	UPROPERTY(VisibleAnywhere, Category = "Vitality Save Data") int UnlockPointsAvailable = 0;

	UPROPERTY(VisibleAnywhere, Category = "Inventory Save Data") TArray<FStInventorySlot> SavedInventory = {};
	UPROPERTY(VisibleAnywhere, Category = "Inventory Save Data") TArray<FStInventorySlot> SavedEquipment = {};
};

/**
 * The object used for saving all DM character data
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API USavedDmCharacter : public USaveGame
{
	GENERATED_BODY()
	
public:

	USavedDmCharacter() {}
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	FString SaveVersion = "v?";
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	FString SaveSlotName = TEXT("QuickName");
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	int EngineeringLevel = 1;
	
	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	float ExperiencePoints = 0;

	UPROPERTY(VisibleAnywhere, Category = "Character Save Data")
	uint32 UserIndex = 0;
	
};