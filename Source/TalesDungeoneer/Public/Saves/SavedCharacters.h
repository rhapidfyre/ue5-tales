// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "lib/enums/GlobalEnums.h"
#include "Characters/Components/MeshMergeComponent.h"

#include "SavedCharacters.generated.h"

struct FStCharacterStats;

USTRUCT()
struct TALESDUNGEONEER_API FCharacterData
{
	GENERATED_BODY()
	
	FString CharacterName			= "";
	int		CharacterLevel			= 1;
	float	ExperiencePoints		= 0.f;
	
	ECharacterClass CharacterClass	= ECharacterClass::ANY;
	ECharacterRace  CharacterRace   = ECharacterRace::ANY;
	
};

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
	FCharacterData CharacterData = {};

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

	// The name of the inventory save file
	UPROPERTY(VisibleAnywhere, Category = "Inventory Save Data") FString SavedInventory = "";
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