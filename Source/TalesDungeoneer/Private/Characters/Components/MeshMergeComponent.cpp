// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Characters/Components/MeshMergeComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "Characters/CharacterBase.h"
#include "DataAssets/CharacterDefaults.h"
#include "Logging/StructuredLog.h"


UMeshMergeComponent::UMeshMergeComponent() 
		: StripTopLODS(0), bNeedsCpuAccess(false), bSkeletonBefore(false)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	SexSkeleton	 = static_cast<ECharacterSex>(FMath::RandRange(0,2) );
}
FMeshBodyMappings::FMeshBodyMappings(USkeletalMesh* UsingMesh,
	const FGameplayTag& BodyTag, const FGameplayTagContainer& NewOptions)
{
	if (IsValid(UsingMesh))
		{ SkeletalMesh = UsingMesh; }
	BodyPartTag		= BodyTag;
	OptionTags		= NewOptions;
};

/**
 * Sets the anim instance used by this component's actor WITHOUT performing the mesh merge.
 * Passing the optional anim instance argument will overwrite AnimBlueprint variable.
 * Replicates to clients if ran on the server.
 * @param NewAnimInstance Optional anim instance
 */
void UMeshMergeComponent::SetAnimBlueprint(TSubclassOf<UAnimInstance> NewAnimInstance)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		const TSubclassOf<UAnimInstance> UsingAnimInstance = IsValid(NewAnimInstance)
										 ? NewAnimInstance : AnimBlueprint;
		if (IsValid(UsingAnimInstance))
		{
			CharacterBase->GetMesh()->SetAnimInstanceClass(NewAnimInstance);
			AnimBlueprint = NewAnimInstance;
		}
	}
}

/**
 * Sets the owner actor's skin color WITHOUT performing the mesh merge.
 * If executed on the server, it will be replicated to all clients internally.
 * @param OptionalColor Optional skin color. If unset, will be ignored.
 */
void UMeshMergeComponent::UpdateSkinMaterial(const FLinearColor OptionalColor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
		if (IsValid(CharacterMesh))
		{
			const FLinearColor BaseColor = FLinearColor();
			
			// If optional color or current skin color is not valid, generate one
			if (OptionalColor == BaseColor || SkinColor_ == BaseColor)
			{
				if (SkinColor_ == BaseColor)
				{
					TArray<FLinearColor> SkinColors		= CharacterBase->GetCharacterRaceData()->SkinColorOptions;
					const int 			 lastSkinIndex 	= SkinColors.Num() - 1;
					const int 			 idx    		= lastSkinIndex < 0 ? -1 : FMath::RandRange(0,lastSkinIndex);
					SkinColor_ = SkinColors.IsValidIndex(idx) ? SkinColors[idx] : FLinearColor(0,0,0,0);
				}
			}

			// If optional color has been provided, use it
			else { SkinColor_ = OptionalColor; }
			
			// Create dynamic instance
			UMaterialInstanceDynamic* dynMaterial =
				UMaterialInstanceDynamic::Create(SkinMaterial, this);

			if (IsValid(dynMaterial))
			{
				// Set Color Parameter
				dynMaterial->SetVectorParameterValue("Param", SkinColor_);
				CharacterMesh->SetMaterial(0, dynMaterial);
			}
		}
	}
}

/**
 * Performs a merge of all supplied meshes and transforms, combining them into
 * one single skeletal mesh on the owning actor. Also updates the skin color
 * and animation instance internally on success.
 * @return True on success
 */
bool UMeshMergeComponent::PerformMeshMerge()
{
	SetMeshIsHidden(true);
	
	UE_LOG(LogTemp, Warning, TEXT("%s(%s): PerformMeshMerge()"), *GetName(),
		GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetOwner());
	if (!IsValid(CharacterBase))
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner Actor was not a valid CharacterBase!"));
		return false;
	}
	
	// Removes all invalid skeletal meshes from the array copy
	// Invalid mesh assets will return TRUE, which removes it from the array.
	MeshMergeData.RemoveAll([](const FMeshMergeMappings& InternalMeshMergeData)
	{
		// Returns within the lambda
		return (!IsValid(InternalMeshMergeData.SkeletalMesh));  
	});

	// Checks for empty array
	if (MeshMergeData.IsEmpty())
	{
		UE_LOGFMT(LogTemp, Error, "{Name}({Authority}): PerformMeshMerge FAILED - "
			"No valid meshes assigned. Character will be invisible or buggy.", GetName(),
			GetOwner()->HasAuthority()?"SERVER":"CLIENT");
		return false;
	}

	const EMeshBufferAccess BufferAccess = bNeedsCpuAccess ?
			EMeshBufferAccess::ForceCPUAndGPU : EMeshBufferAccess::Default;

	bool bRunDuplicateCheck = false;
	USkeletalMesh* BaseMesh = NewObject<USkeletalMesh>();
	if (IsValid(Skeleton) && bSkeletonBefore)
	{
		BaseMesh->SetSkeleton(Skeleton);
		bRunDuplicateCheck = true;
		for (const USkeletalMeshSocket* Socket : BaseMesh->GetMeshOnlySocketList())
		{
			if (IsValid(Socket))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"),
					*(Socket->SocketName.ToString()) );
			}
		}
		for (const USkeletalMeshSocket* Socket : BaseMesh->GetSkeleton()->Sockets)
		{
			if (IsValid(Socket))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkelSocket: %s"),
					*(Socket->SocketName.ToString()) );
			}
		}
		
	}

	TArray<FSkelMeshMergeSectionMapping> SectionMappingsCopy;
	TArray<FSkelMeshMergeUVTransformMapping> UvTransformsCopy;
	TArray<USkeletalMesh*> FinalMeshList;

	for (FMeshMergeMappings& meshMergeData : MeshMergeData)
	{
		// Add the primary skeletal mesh and any accompanying meshes
		FinalMeshList.Add(meshMergeData.SkeletalMesh);
		for (int i = 0; i < meshMergeData.AccompaniedMeshes.Num(); i++)
		{
			FinalMeshList.Add(meshMergeData.AccompaniedMeshes[i]);
		}

		// Add the section mappings and Uv Transforms
		auto arr = meshMergeData.SectionMappings;
		for (FSkelMeshMergeSectionMapping& sMapping : meshMergeData.SectionMappings)
		{
			SectionMappingsCopy.Add(sMapping);
		}
		for (FSkelMeshMergeUVTransformMapping& uvTransform : meshMergeData.MeshUvTransforms)
		{
			UvTransformsCopy.Add(uvTransform);
		}
	}
	
	FSkeletalMeshMerge MeshMerger(BaseMesh, FinalMeshList, SectionMappingsCopy,
			StripTopLODS, BufferAccess, UvTransformsCopy.GetData());
	
	if (!MeshMerger.DoMerge())
	{
		UE_LOG(LogTemp, Warning, TEXT("Merge failed!"));
		return nullptr;
	}
	if (IsValid(Skeleton) && !bSkeletonBefore)
	{
		BaseMesh->SetSkeleton(Skeleton);
	}
	
	if (bRunDuplicateCheck)
	{
		TArray<FName> SkeletonMeshSockets;
		TArray<FName> SkeletonSockets;
		
		for (const USkeletalMeshSocket* Socket : BaseMesh->GetMeshOnlySocketList())
		{
			if (Socket)
			{
				SkeletonMeshSockets.Add(Socket->GetFName());
				UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"), *(Socket->SocketName.ToString()));
			}
		}
		
		for (const USkeletalMeshSocket* Socket : BaseMesh->GetSkeleton()->Sockets)
		{
			if (Socket)
			{
				SkeletonSockets.Add(Socket->GetFName());
				UE_LOG(LogTemp, Warning, TEXT("SkelSocket: %s"), *(Socket->SocketName.ToString()));
			}
		}
		
		TSet<FName> UniqueSkeletonMeshSockets;
		TSet<FName> UniqueSkeletonSockets;
		UniqueSkeletonMeshSockets.Append(SkeletonMeshSockets);
		UniqueSkeletonSockets.Append(SkeletonSockets);
		const int32 Total = SkeletonSockets.Num() + SkeletonMeshSockets.Num();
		const int32 UniqueTotal = UniqueSkeletonMeshSockets.Num() + UniqueSkeletonSockets.Num();
		UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocketCount: %d | SkelSocketCount: %d | Combined: %d"), SkeletonMeshSockets.Num(), SkeletonSockets.Num(), Total);
		UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocketCount: %d | SkelSocketCount: %d | Combined: %d"), UniqueSkeletonMeshSockets.Num(), UniqueSkeletonSockets.Num(), UniqueTotal);
		UE_LOG(LogTemp, Warning, TEXT("Found Duplicates: %s"), *((Total != UniqueTotal) ? FString("True") : FString("False")));
	}
	
	if (IsValid(BaseMesh))
	{
		CharacterBase->GetMesh()->SetSkeletalMesh(BaseMesh);
		UpdateSkinMaterial(SkinColor_);
		SetAnimBlueprint(AnimBlueprint);
		SetMeshIsHidden(false);
		
		OnMeshMergeCompleted.Broadcast();
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("PerformMeshMerge() failed!"));
	return false;
}


void UMeshMergeComponent::SetupDefaultMeshes(TArray<FBodyPartData> BodyPartDatum)
{
	for (const FBodyPartData& BodyPartData : BodyPartDatum)
	{
		const FMeshBodyMappings BodyMapping = CreateBodyMapping(
			BodyPartData.SkeletalMesh, BodyPartData.BodyPartTag, BodyPartData.BodyTags);
		MeshBodyData.Add(BodyMapping);
	}
}

/**
 * Performs initialization such as restoring saved mesh data, then calls
 * PerformMeshMerge() internally upon success. Does nothing if the save is invalid.
 * @param NewSkeleton The skeleton to use
 * @param NewAnimInstance The anim instance to use
 * @param NewSkinColor The skin color to be used
 * @param MergeMappings The merge data
 */
void UMeshMergeComponent::InitializeMeshMerge(
	USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance,
	FLinearColor NewSkinColor, const TArray<FMeshMergeMappings>& MergeMappings)
{
	if (!GetOwner()->HasAuthority())
	{
		Server_InitializeMeshMerge(NewSkeleton, NewAnimInstance, NewSkinColor, MergeMappings);
		return;
	}
	bHasInitialized = true;
	
	Skeleton      	= NewSkeleton;
	AnimBlueprint	= NewAnimInstance;
	SkinColor_		= NewSkinColor;
	MeshMergeData 	= MergeMappings;
	
	PerformMeshMerge();
}

void UMeshMergeComponent::SetMeshIsHidden(bool bIsHidden)
{
	bHideMesh = bIsHidden;
	ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		CharacterBase->GetMesh()->SetVisibility(!bIsHidden);
	}
}

int UMeshMergeComponent::FindIndexOfMeshByTag(const FGameplayTag& SearchTag)
{
	if (SearchTag.IsValid())
	{
		for (int i = 0; i < MeshMergeData.Num(); i++)
		{
			const FMeshMergeMappings& meshMapping = MeshMergeData[i];
			if (meshMapping.EquipSlotTag == SearchTag)
			{
				return i;
			}
		}
	}
	return -1;
}

/** Creates a new mesh merge mapping.
 * Does not add it to the mappings. Developer
 * must modify the new mapping accordingly (uv transforms, etc) and then add
 * it to the mapping using 'AddToMeshMapping'.
 */
FMeshMergeMappings UMeshMergeComponent::CreateMeshMapping(
	const UEquipmentItemData* NewAsset, const FGameplayTag& EquipmentTag, const bool useFeminineMesh)
{
	// Create a new mesh merge mapping
	FMeshMergeMappings NewMapping(NewAsset, useFeminineMesh);
	NewMapping.EquipSlotTag = EquipmentTag;
	return NewMapping;
}

FMeshBodyMappings UMeshMergeComponent::CreateBodyMapping(USkeletalMesh* UsingMesh,
	const FGameplayTag& BodyTag, FGameplayTagContainer BodyOptionTags)
{
	return FMeshBodyMappings(UsingMesh, BodyTag, BodyOptionTags);
}


/**
 * Adds the given mesh with the specified mappings.
 * Internally removes the old mesh mapping before adding the new one.
 * @param NewMapping The new mesh merge mapping to be added
 */
void UMeshMergeComponent::AddMeshToMerge(const FMeshMergeMappings& NewMapping)
{
	if (IsValid(NewMapping.DataAsset) || NewMapping.EquipSlotTag.IsValid())
	{
		// Remove the existing mesh
		RemoveMeshFromMerge(NewMapping.DataAsset, NewMapping.EquipSlotTag);
		MeshMergeData.Add(NewMapping);
	}
}

/**
 * Removes the given mesh by tag (priority) or data asset.
 * This function needs to be executed on the server.
 * @param NewAsset Pulls the meshes specified in this asset. Optional.
 * @param EquipmentTag Required. The equipment or body slot tag to use.
 */
void UMeshMergeComponent::RemoveMeshFromMerge(
	const UEquipmentItemData* NewAsset, const FGameplayTag& EquipmentTag)
{
	// Always prefer the tag if it is valid. There's only one slot for this tag.
	if (EquipmentTag.IsValid())
	{
		const int idx = FindIndexOfMeshByTag(EquipmentTag);
		if (idx >= 0) { MeshMergeData.RemoveAt(idx); }
	}
	// If the tag isn't valid, remove the equipment that matches
	else
	{
		for (int i = 0; i < MeshMergeData.Num(); i++)
		{
			if (MeshMergeData[i].DataAsset == NewAsset)
			{
				MeshMergeData.RemoveAt(i);
				return;
			}
		}
	}
}

/**
 * Called by the owning client when restoring data from USavedCharacter
 * @param NewSkeleton		The skeleton restored from save
 * @param NewAnimInstance	The anim blueprint restored from save
 * @param NewSkinColor		The skin color restored from save
 * @param MergeMappings		The mesh merge mappings restored from save
 */
void UMeshMergeComponent::Server_InitializeMeshMerge_Implementation(
		USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance,
		FLinearColor NewSkinColor, const TArray<FMeshMergeMappings>& MergeMappings)
{
	if (GetOwner()->HasAuthority())
	{
		InitializeMeshMerge(NewSkeleton, NewAnimInstance, NewSkinColor, MergeMappings);
	}
}

void UMeshMergeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMeshMergeComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	RegisterComponent();
}

void UMeshMergeComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UMeshMergeComponent::OnRep_HideMesh_Implementation()
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		USkeletalMeshComponent* skeletalMeshComponent = CharacterBase->GetMesh();
		skeletalMeshComponent->SetVisibility(!bHideMesh);
	}
}

void UMeshMergeComponent::Server_SetAnimBlueprint_Implementation(TSubclassOf<UAnimInstance> NewAnimInstance)
{
	if (GetOwner()->HasAuthority()) { SetAnimBlueprint(AnimBlueprint); }
}

/**
 * Performs a mesh merge when the authority sends the updated mesh data
 */
void UMeshMergeComponent::OnRep_MeshMergeData_Implementation()
{
	if (!GetOwner()->HasAuthority()) { PerformMeshMerge(); }
}

void UMeshMergeComponent::OnRep_SkinColor_Implementation()
{
	if (!GetOwner()->HasAuthority()) { UpdateSkinMaterial(SkinColor_); }
}

void UMeshMergeComponent::OnRep_MeshBodyData_Implementation()
{
	if (!GetOwner()->HasAuthority()) { PerformMeshMerge(); }
}

void UMeshMergeComponent::OnRep_AnimInstance_Implementation()
{
	if (!GetOwner()->HasAuthority()) { SetAnimBlueprint(nullptr); }
}


void UMeshMergeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Vars replicated to all clients
	DOREPLIFETIME(UMeshMergeComponent, bHideMesh);
	DOREPLIFETIME(UMeshMergeComponent, Skeleton);
	DOREPLIFETIME(UMeshMergeComponent, MeshMergeData);
	DOREPLIFETIME(UMeshMergeComponent, SkinColor_);
	DOREPLIFETIME(UMeshMergeComponent, AnimBlueprint);
	
}
