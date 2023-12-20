// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SkeletalMeshMerge.h"
#include "Components/ActorComponent.h"

#include "MeshMergeComponent.generated.h"

//FWD DECLARE
class USavedCharacter;

USTRUCT(BlueprintType)
struct TALESDUNGEONEER_API FStMeshMergeData
{
	GENERATED_BODY()

	FStMeshMergeData() {};
	FStMeshMergeData(FName DataRowName, bool IsMale = true);
	
	// The item name associated with this mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ItemName = FName();
	// What body part this mesh is associated with
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag AssociatedBodyPart = {};
	// Which part parts will be hidden if this mesh is equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer HidesBodyParts = {};
	// The mesh to be used
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* MeshAsset = nullptr;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UMeshMergeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMeshMergeComponent();

	UFUNCTION(BlueprintCallable) bool PerformMeshMerge();

	UFUNCTION(BlueprintPure)
	bool GetIsMeshMergeSystemReady() const { return bHasInitialized; }

	void InitializeMeshMerge(const USavedCharacter* CharacterData = nullptr);

	UFUNCTION(Server, Reliable)
	void Server_InitializeMeshMerge(USkeleton* NewSkeleton,
		const TArray<FSkelMeshMergeSectionMapping>& NewMeshMaps,
		const TArray<FSkelMeshMergeUVTransformMapping>& NewUvTransforms,
		const TArray<FStMeshMergeData>& NewMeshes);

	UFUNCTION(BlueprintPure)
	int GetMeshMergeIndexByTag(FGameplayTag SearchTag) const;

	UFUNCTION(BlueprintCallable)
	int AddNewMeshToArrayByTag(FGameplayTag GameTag, FName EquipmentName, bool IsMale = true);

	UFUNCTION(BlueprintCallable)
	void SetNewMeshByIndex(FName EquipmentName, int ArrayIndex = -1, bool IsMale = true, bool MergeNow = false);

	UFUNCTION(BlueprintCallable)
	void SetNewMeshByTag(FName EquipmentName, FGameplayTag GameTag, bool IsMale = true, bool MergeNow = false);

protected:
	
	virtual void BeginPlay() override;

	virtual void OnComponentCreated() override;

	virtual void InitializeComponent() override;
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:

	void InitializeDefaultMeshes();

public:
	
	// An optional array to map sections from the source meshes to merged section entries
	UFUNCTION(NetMulticast, Reliable) void OnRep_MeshSectionMappings();
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_MeshSectionMappings)
	TArray < FSkelMeshMergeSectionMapping > MeshSectionMappings = {};
	
	// An optional array to transform the UVs in each mesh
	UFUNCTION(NetMulticast, Reliable) void OnRep_UvTransformsPerMesh();
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_UvTransformsPerMesh)
	TArray < FSkelMeshMergeUVTransformMapping > UvTransformsPerMesh = {};
	
	// The list of skeletal meshes to merge.
	UFUNCTION(NetMulticast, Reliable) void OnRep_MeshesToMerge();
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite, ReplicatedUsing=OnRep_MeshesToMerge)
	TArray < FStMeshMergeData > MeshesToMerge = {};

	TArray < FStMeshMergeData > DefaultMeshes= {} ;
	
	// The number of high LODs to remove from input meshes
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StripTopLODS;
	
	// Whether or not the resulting mesh needs to be accessed by the CPU for any reason (e.g. for spawning particle effects).
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bNeedsCpuAccess : 1;
	
	// Update skeleton before merge. Otherwise, update after.
	// Skeleton must also be provided.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint32 bSkeletonBefore : 1;
	
	// Skeleton that will be used for the merged mesh.
	// Leave empty if the generated skeleton is OK.
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly)
	USkeleton* Skeleton = nullptr;
	
	// The animation blueprint that will be used
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UAnimInstance> AnimBlueprint = nullptr;

private:
	
	// This way the initial setups on OnConstruction only run once
	bool bHasInitialized = false;

	bool bMeshSaveRestored = false;
	
};
