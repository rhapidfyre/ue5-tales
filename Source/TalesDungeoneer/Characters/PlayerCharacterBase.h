// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "CharacterBase.h" // Includes core and actor files

#include "PlayerCharacterBase.generated.h"


class USavedCharacter;

// Called when this player has fully spawned into the world
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerJoined);

/**
 * Player Character Base is the base C++ class for all logic, methods and members that affect all
 * PLAYER based characters, prior to handling by child classes or dependent blueprint classes.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API APlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public: // functions

	UPROPERTY(VisibleAnywhere, BlueprintAssignable) FOnPlayerJoined OnPlayerJoined;
	
	APlayerCharacterBase();

	virtual void LoadSaveData(const FString& SaveName,
		const int32 UserIndex, USaveGame* SaveData) override;

	virtual bool SaveCharacterData() override;
	virtual bool LoadCharacterData(const FString SaveSlotName, const int32 UserIndex) override;

	UFUNCTION() void AwaitGameState();

protected:

	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void BeginPlay() override;

	// Used to reinitialize the character with client's data
	UFUNCTION(Server, Reliable)
	void Server_InitializeCharacter(const FString& NewName, int NewLevel,
		ECharacterRace NewRace, ECharacterClass NewClass, float NewExperience);

	UFUNCTION(Server, Reliable)
	void Server_SetupMeshMerge(
		const TArray<FStMeshMergeData>& MeshesToMerge,
		const TArray<FSkelMeshMergeSectionMapping>& MeshSectionMappings,
		const TArray<FSkelMeshMergeUVTransformMapping>& UvTransformsPerMesh);
	
	bool bHasInitialized = false;

	FString CharacterSaveName = "";
	int32 CharacterSaveIndex = 0;
};
