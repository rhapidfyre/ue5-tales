// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SkeletalMeshMerge.h"
#include "Components/ActorComponent.h"
#include "lib/EquipmentData.h"

#include "MeshMergeComponent.generated.h"

UENUM(BlueprintType)
enum class ECharacterSex : uint8
{
	NONBINARY = 0	UMETA(DisplayName = "Non-Binary"),
	MASCULINE		UMETA(DisplayName = "Masculine"),
	FEMININE		UMETA(DisplayName = "Feminine"),
	MAX				UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FMeshMergeMappings
{
	GENERATED_BODY()
	FMeshMergeMappings() : DataAsset(nullptr), SkeletalMesh(nullptr) {};
	FMeshMergeMappings(const UEquipmentItemData* NewAsset, bool isFeminine) : DataAsset(NewAsset)
	{
		if (IsValid(NewAsset))
		{
			USkeletalMesh* UsingMesh = NewAsset->MeshMasculine;
			if (isFeminine && IsValid(NewAsset->MeshFeminine))
			{
				UsingMesh = NewAsset->MeshFeminine;
			}
			SkeletalMesh = UsingMesh;
		}
	};

	// Gameplay tags that apply to this mesh merge
	UPROPERTY(EditAnywhere) FGameplayTagContainer GameplayTags = {};

	// Which equipment slot this mesh occupies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	FGameplayTag EquipSlotTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	const UEquipmentItemData* DataAsset;
	
	// The mesh to be used by this option
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USkeletalMesh* SkeletalMesh;
	
	// A map section from the source mesh to merged section entry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkelMeshMergeSectionMapping> SectionMappings = {};
	
	// A transform for the UVs in mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkelMeshMergeUVTransformMapping> MeshUvTransforms = {};
	
};


USTRUCT(BlueprintType)
struct FBodyPartData
{
	GENERATED_BODY()
	
	// The mesh to be used by this option
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh*			SkeletalMesh	= nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor			SkinColor		= FLinearColor(255, 206, 180);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag			BodyPartTag		= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer	BodyTags		= {};
	
};


USTRUCT(BlueprintType)
struct FMeshBodyMappings
{
	GENERATED_BODY()
	FMeshBodyMappings() : SkeletalMesh(nullptr) {};
	FMeshBodyMappings(USkeletalMesh* UsingMesh,
		const FGameplayTag& BodyTag, const FGameplayTagContainer& OptionTags);
	
	// The mesh to be used by this option
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* SkeletalMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag BodyPartTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsFeminine  = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsMasculine = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UMeshMergeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UMeshMergeComponent();
	
	UFUNCTION(BlueprintCallable)
	bool PerformMeshMerge();

	void SetupDefaultMeshes(TArray<FBodyPartData> BodyPartDatum);

	UFUNCTION(BlueprintPure)
	bool GetIsMeshMergeSystemReady() const { return bHasInitialized; }

	// Used for restoring from a save game
	void InitializeMeshMerge(
		USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance,
		const TArray<FMeshMergeMappings>& MergeMappings = {});

	UFUNCTION(BlueprintCallable)
	void SetMeshIsHidden(bool bIsHidden);
	
	UFUNCTION(BlueprintCallable)
	bool GetMeshIsHidden() const { return bHideMesh; }

	UFUNCTION(Server, Reliable)
	void Server_InitializeMeshMerge(
		USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance,
		const TArray<FMeshMergeMappings>& MergeMappings = {});

	int FindIndexOfMeshByTag(const FGameplayTag& SearchTag);

	TArray<FMeshMergeMappings> GetAllMeshMergeMappings() const { return MeshMergeData; }
	
	UFUNCTION(BlueprintCallable)
	FMeshMergeMappings CreateMeshMapping(const UEquipmentItemData* NewAsset,
		const FGameplayTag& EquipmentTag, const bool useFeminineMesh = false);
	
	UFUNCTION(BlueprintCallable)
	FMeshBodyMappings CreateBodyMapping(USkeletalMesh* UsingMesh,
		const FGameplayTag& BodyTag, FGameplayTagContainer BodyOptionTags);

	UFUNCTION(BlueprintCallable)
	void AddMeshToMerge(const FMeshMergeMappings& NewMapping);

	UFUNCTION(BlueprintCallable)
	void RemoveMeshFromMerge(const UEquipmentItemData* NewAsset, const FGameplayTag& EquipmentTag);

protected:
	
	virtual void BeginPlay() override;

	virtual void OnComponentCreated() override;

	virtual void InitializeComponent() override;
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	// The skeleton to use (Male/Female)
	// Non-Binary: Randomizes Male/Female
	UPROPERTY(EditAnywhere)	ECharacterSex SexSkeleton;
	
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

	// The default body meshes used in every single mesh merge
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMeshBodyMappings> MeshBodyData = {};

private:

	UFUNCTION(NetMulticast, Reliable) void OnRep_MeshMergeData();
	
	// These are the "optional" overlaid meshes in addition to the MeshBodyData
	UPROPERTY(ReplicatedUsing=OnRep_MeshMergeData)
	TArray<FMeshMergeMappings> MeshMergeData = {};

	// Start hidden by default
	UFUNCTION(NetMulticast, Reliable) void OnRep_HideMesh();
	UPROPERTY(ReplicatedUsing=OnRep_HideMesh) bool bHideMesh = true;
	
	// This way the initial setups on OnConstruction only run once
	bool bHasInitialized	= false;
	
	bool bMeshSaveRestored	= false;
	
};
