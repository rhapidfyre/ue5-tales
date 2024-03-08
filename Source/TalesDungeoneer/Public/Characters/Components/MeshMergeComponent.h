// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "SkeletalMeshMerge.h"
#include "Components/ActorComponent.h"
#include "lib/EquipmentData.h"
#include "Delegates/Delegate.h"
#include "GameFramework/SaveGame.h"

#include "MeshMergeComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMeshMergeCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeshMergeRestored, const bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMeshMergeSaved, const bool, bSuccess);


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

	// The number of meshes holding this mesh as hidden
	int numSuperiorMeshes = 0;

	// Optional gameplay tags that describe this mesh (underwear, male, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
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
	TArray<USkeletalMesh*>		AccompaniedMeshes = {};
	
	// A map section from the source mesh to merged section entry
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkelMeshMergeSectionMapping>	 SectionMappings	= {};
	
	// A transform for the UVs in mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkelMeshMergeUVTransformMapping> MeshUvTransforms	= {};
	
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

	// Optional material instance corrections (Skin Color, Hair Color, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<UMaterialInstance*, FLinearColor>	 MaterialInstances	= {};
	
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

	UPROPERTY(BlueprintAssignable) FOnMeshMergeCompleted OnMeshMergeCompleted;
	UPROPERTY(BlueprintAssignable) FOnMeshMergeRestored  OnMeshMergeRestored;
	UPROPERTY(BlueprintAssignable) FOnMeshMergeSaved     OnMeshMergeSaved;
	
	UFUNCTION(BlueprintCallable)
	bool PerformMeshMerge(bool bMergeMeshesOnly = false);

	UFUNCTION(BlueprintPure)
	bool GetIsMeshMergeSystemReady() const { return bMeshMergeReady; }

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

	int FindMeshMappingByTag(const FGameplayTag& searchTag);
	FMeshMergeMappings GetMeshMappingFromIndex(int index = -1);

	UFUNCTION(BlueprintPure)
	FGameplayTag GetBodyPartFromEquipmentSlot(const FGameplayTag& EquipmentSlotTag);

	// Private Member Accessors
	TArray<FMeshMergeMappings>	GetAllMeshMergeMappings() const { return MeshMergeData; }
	FLinearColor 				GetEyeColor() const				{ return EyeColor_;     }
	FLinearColor 				GetSkinColor() const			{ return SkinColor_;    }
	FLinearColor 				GetHairColor() const			{ return HairColor_;    }
	FLinearColor 				GetBeardColor() const			{ return BeardColor_;   }
	USkeleton*	 				GetSkeleton() const				{ return Skeleton;      }
	TSubclassOf<UAnimInstance>	GetAnimBlueprint() const		{ return AnimBlueprint; }

	UFUNCTION(BlueprintCallable) void SetEyeColor(const FLinearColor NewColor);
	UFUNCTION(BlueprintCallable) void SetSkinColor(const FLinearColor NewColor);
	UFUNCTION(BlueprintCallable) void SetHairColor(const FLinearColor NewColor);
	UFUNCTION(BlueprintCallable) void SetBeardColor(const FLinearColor NewColor);

	void LoadMeshMerge(FString& LoadResponse, FString& SaveSlotName,
			int32 SaveUserIndex = 0, bool bIsAsync = true);

	FString SaveMeshMerge(FString& responseStr, bool isAsync = true);

	UFUNCTION(BlueprintPure) FString GetMeshMergeSaveName() const { return SaveSlotName_; }

	bool HasAuthority() const;
	
protected:
	
	virtual void BeginPlay() override;

	virtual void OnComponentCreated() override;

	virtual void InitializeComponent() override;
	
	virtual void GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:

	// Set false if the owner actor uses a full body mesh that doesn't need to be merged
	// This is for things such as skeletons that are not modular, or spectres that always look the same
	// Defaults to TRUE
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseMeshMerge = true;
	
	// Override to force all-masculine or feminine options, or both (Non-Binary)
	UPROPERTY(EditAnywhere)	ECharacterSex SexSkeleton = ECharacterSex::NONBINARY;
	
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

private:

	void ValidateMeshMergeMappings(const FMeshMergeMappings& MapReference, bool bWasRemoved = false);

	UFUNCTION(NetMulticast, Reliable) void OnRep_EyeColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_SkinColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_HairColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_BeardColor();
	UFUNCTION(NetMulticast, Reliable) void OnRep_MeshMergeData();
	UFUNCTION(NetMulticast, Reliable) void OnRep_AnimInstance();
	UFUNCTION(NetMulticast, Reliable) void OnRep_MeshBodyData();

	void UpdateMeshMaterials(const UMaterialInterface* MaterialInterface = nullptr);

	UFUNCTION() void LoadDataDelegate(const FString& SaveName, const int32 UserIndex, USaveGame* LoadGameData);
	UFUNCTION() void SaveDataDelegate(const FString& SaveName, int UserIndex, bool bSuccess = false);
	
	UFUNCTION(Server, Reliable)
	void Server_RestoreSkeleton(USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance);
	void RestoreSkeleton(USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance);
	
	UFUNCTION(Server, Reliable)
	void Server_RestoreMappings(const TArray<FMeshMergeMappings>& NewMeshMappings,
								const TArray<FMeshBodyMappings>&  NewBodyMappings);
	void RestoreMappings(const TArray<FMeshMergeMappings>& NewMeshMappings,
						 const TArray<FMeshBodyMappings>&  NewBodyMappings);
	
	UFUNCTION(Server, Reliable)
	void Server_RestoreMaterials(FLinearColor NewEyeColor, FLinearColor NewSkinColor,
								 FLinearColor NewHairColor, FLinearColor NewBeardColor);
	void RestoreMaterials(FLinearColor NewEyeColor, FLinearColor NewSkinColor,
						  FLinearColor NewHairColor, FLinearColor NewBeardColor);

	UPROPERTY(ReplicatedUsing=OnRep_EyeColor)
	FLinearColor EyeColor_   = FLinearColor(0.1, 0.1, 0.1, 255);

	UPROPERTY(ReplicatedUsing=OnRep_SkinColor)
	FLinearColor SkinColor_  = FLinearColor(0.949, 0.937, 0.933, 255);

	UPROPERTY(ReplicatedUsing=OnRep_HairColor)
	FLinearColor HairColor_  = FLinearColor(0.02, 0.02, 0.02, 255);

	UPROPERTY(ReplicatedUsing=OnRep_BeardColor)
	FLinearColor BeardColor_ = FLinearColor(0.02, 0.02, 0.02, 255);

	// These are the "optional" overlaid meshes in addition to the MeshBodyData
	UPROPERTY(ReplicatedUsing=OnRep_MeshMergeData)
	TArray<FMeshMergeMappings> MeshMergeData = {};

	UPROPERTY()
	class ACharacterBase* CharacterBase_ = nullptr;

	FString SaveSlotName_  = "";
	int32   SaveUserIndex_ = 0;
	
	// This way the initial setups on OnConstruction only run once
	bool bMeshMergeReady	= false;

	bool bRestoredFromSave  = false;

	bool bSavesOnServer     = false;
	
	bool bMeshSaveRestored	= false;

	FString SaveFolder = "MeshMerge/";

	
};
