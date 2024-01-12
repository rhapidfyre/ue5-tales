// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SkeletalMeshMerge.h"
#include "Components/ActorComponent.h"
#include "lib/EquipmentData.h"
#include "Delegates/Delegate.h"

#include "MeshMergeComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeshMergeCompleted);


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

	// Optional gameplay tags that describe this mesh (underwear, male, etc)
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer		GameplayTags = {};

	// Which equipment or body slot this mesh occupies
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag				EquipSlotTag;

	// Is this used? Obsolete?
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	const UEquipmentItemData*	DataAsset;
	
	// The mesh to be used by this option
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh*				SkeletalMesh;

	// Additional meshes that must accompany this mesh, if visible
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USkeletalMesh*>		AccompaniedMeshes		= {};

	// Any element modifications before merge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UMaterialInstanceDynamic*> MaterialInstance	= {};
	
	// A map section from the source mesh to merged section entry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkelMeshMergeSectionMapping>		SectionMappings		= {};
	
	// A transform for the UVs in mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkelMeshMergeUVTransformMapping>	MeshUvTransforms	= {};
	
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
	
	// The primary tag for this body part (body.arms, body.torso, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag BodyPartTag;

	// The color for this specific mesh material with matching element index
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FLinearColor> ColorAtElementIndex;
	
	// Optional tags that describe this mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer OptionTags;
	
	// Any mesh in the merge with any of these tags will be visible if this mesh is visible 
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer RequiresMeshesWithTags;
	
	// Any mesh in the merge with any of these tags will be hidden if this mesh is visible
	// Does not hide any mesh that is marked as required by any other mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer HidesMeshesWithTags;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UMeshMergeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UMeshMergeComponent();

	UPROPERTY(BlueprintAssignable)
	FOnMeshMergeCompleted OnMeshMergeCompleted;

	UFUNCTION()
	void UpdateMeshMaterials(const FGameplayTag& BodyPartTag,
		UMaterialInstance* SpecificMaterial, FLinearColor MaterialColor = FLinearColor());
	
	UFUNCTION(BlueprintCallable)
	void SetAnimBlueprint(TSubclassOf<UAnimInstance> NewAnimInstance);

	UFUNCTION(BlueprintCallable)
	void SetEyeMaterial(const FLinearColor OptionalColor);
	
	UFUNCTION(BlueprintCallable)
	void SetSkinMaterial(const FLinearColor OptionalColor);

	UFUNCTION(BlueprintCallable)
	void SetHairMaterial(const FLinearColor OptionalColor);

	UFUNCTION(BlueprintCallable)
	void SetBeardMaterial(const FLinearColor OptionalColor);
	
	UFUNCTION(BlueprintCallable)
	bool PerformMeshMerge(bool bMergeMeshesOnly = false);

	UFUNCTION(BlueprintPure)
	bool GetIsMeshMergeSystemReady() const { return bHasInitialized; }

	int FindIndexOfMeshByTag(const FGameplayTag& SearchTag);
	
	UFUNCTION(BlueprintCallable)
	FMeshMergeMappings CreateMeshMapping(const UEquipmentItemData* NewAsset,
		const FGameplayTag& EquipmentTag, const bool useFeminineMesh = false);
	
	UFUNCTION(BlueprintCallable)
	FMeshBodyMappings CreateBodyMapping(USkeletalMesh* UsingMesh,
		const FGameplayTag& BodyTag, FGameplayTagContainer BodyOptionTags);

	UFUNCTION(BlueprintCallable)
	void AddMeshToMerge(const FMeshMergeMappings& NewMapping);

	UFUNCTION(BlueprintCallable)
	void RemoveMeshFromMerge(const UEquipmentItemData* NewAsset,
		const FGameplayTag& EquipmentTag);

	// Private Member Accessors
	TArray<FMeshMergeMappings>	GetAllMeshMergeMappings() const { return MeshMergeData; }
	FLinearColor 				GetEyeColor() const				{ return EyeColor_;     }
	FLinearColor 				GetSkinColor() const			{ return SkinColor_;    }
	FLinearColor 				GetHairColor() const			{ return HairColor_;    }
	FLinearColor 				GetBeardColor() const			{ return BeardColor_;   }
	USkeleton*	 				GetSkeleton() const				{ return Skeleton;      }
	TSubclassOf<UAnimInstance>	GetAnimBlueprint() const		{ return AnimBlueprint; }

protected:
	
	virtual void BeginPlay() override;

	virtual void OnComponentCreated() override;

	virtual void InitializeComponent() override;
	
	virtual void GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void Server_SetAnimBlueprint(TSubclassOf<UAnimInstance> NewAnimInstance);

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
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	USkeleton* Skeleton = nullptr;
	
	// The animation blueprint that will be used
	UPROPERTY(ReplicatedUsing=OnRep_AnimInstance, EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBlueprint = nullptr;

	// The default body meshes used in every single mesh merge
	UPROPERTY(ReplicatedUsing=OnRep_MeshBodyData, EditAnywhere, BlueprintReadOnly)
	TArray<FMeshBodyMappings> MeshBodyData = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* UseSkinMaterial  = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor UseSkinColor = FLinearColor(242, 239, 238, 255);

private:

	UFUNCTION(NetMulticast, Reliable) void OnRep_EyeColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_SkinColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_HairColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_BeardColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_MeshMergeData();
	UFUNCTION(NetMulticast, Reliable) void OnRep_AnimInstance();
	UFUNCTION(NetMulticast, Reliable) void OnRep_MeshBodyData();

	UPROPERTY(ReplicatedUsing=OnRep_EyeColor)
	FLinearColor EyeColor_   = FLinearColor(0.1, 0.1, 0.1, 255);

	UPROPERTY(ReplicatedUsing=OnRep_SkinColor)
	FLinearColor SkinColor_  = FLinearColor(242, 239, 238, 255);

	UPROPERTY(ReplicatedUsing=OnRep_HairColor)
	FLinearColor HairColor_  = FLinearColor(0.02, 0.02, 0.02, 255);

	UPROPERTY(ReplicatedUsing=OnRep_BeardColor)
	FLinearColor BeardColor_ = FLinearColor(0.02, 0.02, 0.02, 255);

	// These are the "optional" overlaid meshes in addition to the MeshBodyData
	UPROPERTY(ReplicatedUsing=OnRep_MeshMergeData)
	TArray<FMeshMergeMappings> MeshMergeData = {};

	UPROPERTY()
	class ACharacterBase* CharacterBase_ = nullptr;
	
	// This way the initial setups on OnConstruction only run once
	bool bHasInitialized	= false;
	
	bool bMeshSaveRestored	= false;

	
};
