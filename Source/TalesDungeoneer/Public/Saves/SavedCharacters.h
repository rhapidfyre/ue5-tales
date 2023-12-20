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
	
	UPROPERTY(SaveGame) FString CharacterName			= "";
	UPROPERTY(SaveGame) int		CharacterLevel			= 1;
	UPROPERTY(SaveGame) float	ExperiencePoints		= 0.f;
	
	UPROPERTY(SaveGame) ECharacterClass CharacterClass	= ECharacterClass::ANY;
	UPROPERTY(SaveGame) ECharacterRace  CharacterRace   = ECharacterRace::ANY;
	
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
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	FString SaveVersion = "v?";
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	FString SaveSlotName = TEXT("QuickName");

	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	FCharacterData CharacterData = {};

	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	uint32 UserIndex = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Mesh Merge Save Data")
	TSubclassOf<UAnimBlueprint> AnimBlueprint = UAnimBlueprint::StaticClass();

	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Mesh Merge Save Data")
	USkeleton* Skeleton = nullptr;

	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Mesh Merge Save Data")
	TArray < FStMeshMergeData > MeshesToMerge = {};
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Mesh Merge Save Data")
	TArray < FSkelMeshMergeSectionMapping > MeshSectionMappings = {};
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Mesh Merge Save Data")
	TArray < FSkelMeshMergeUVTransformMapping > UvTransformsPerMesh = {};

	// The name of the inventory save file
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Inventory Save Data")
	FString SavedInventory = "";
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
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	FString SaveVersion = "v?";
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	FString SaveSlotName = TEXT("QuickName");
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	int EngineeringLevel = 1;
	
	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	float ExperiencePoints = 0;

	UPROPERTY(SaveGame, VisibleAnywhere, Category = "Character Save Data")
	uint32 UserIndex = 0;
	
};