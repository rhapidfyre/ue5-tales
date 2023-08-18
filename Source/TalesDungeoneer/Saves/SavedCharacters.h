// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
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