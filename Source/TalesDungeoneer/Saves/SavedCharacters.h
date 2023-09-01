// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "lib/VitalityData.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"
#include "TalesDungeoneer/Characters/Components/MeshMergeComponent.h"

#include "SavedCharacters.generated.h"


struct FStCharacterStats;
struct FStMeshMergeData;

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

	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data") bool  UseHealthSubsystem = true;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	bool  UseStaminaSubsystem = true;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	bool  UseMagicSubsystem = true;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	bool  UseSurvivalSubsystem = false;
	
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingHealthCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingHealthMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float PassiveHealthRegen = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float HealthTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingStaminaCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingStaminaMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float PassiveStaminaRegen = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StaminaTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingMagicCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingMagicMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float PassiveMagicRegen = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float MagicTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingHydrationCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingHungerCurrent = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingHydrationMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float StartingHungerMaximum = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float PassiveHydrationDrain = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float PassiveHungerDrain = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float HydrationTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data")	float CaloriesTimerTickRate = 0.f;
	UPROPERTY(VisibleAnywhere, Category = "Vitality Welfare Save Data") FStVitalityStats BaseStats;
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