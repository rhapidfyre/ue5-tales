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
 * Called after setting one (or all) of the hair/beard/skin/etc mesh materials.
 * Does NOT perform a Mesh Merge. This is called after the mesh merge has been performed.
 * @param SpecificMaterial  Optional.
 *							If provided, updates the specific material instead of all of them.
 */
void UMeshMergeComponent::UpdateMeshMaterials(
	const FGameplayTag& BodyPartTag, UMaterialInstance* SpecificMaterial, FLinearColor MaterialColor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	
	USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
	if (!IsValid(CharacterMesh)) { return; }
	
	TArray<UMaterialInterface*> meshMats = CharacterMesh->GetMaterials();
	
	for (UMaterialInterface* meshMaterial : meshMats)
	{
		if (!IsValid(meshMaterial)) {continue;}
		
		int matIndex = -1;
		for (int i = 0; i < meshMats.Num(); i++)
		{
			UMaterialInterface* matInterface	 = meshMats[i];
			const UMaterialInstance* matInstance = Cast<UMaterialInstance>(matInterface);
			UMaterialInterface* matParent		 = Cast<UMaterialInterface>(matInstance->Parent);
			if (matParent == meshMaterial || matInterface == meshMaterial)
			{
				matIndex = i;
				break;
			}
		}
		if (matIndex < 0) { continue; }

		FLinearColor meshColor = FLinearColor();
		
		// Use the existing dynamic material, or create a new one
		UMaterialInstanceDynamic* dynMaterial = Cast<UMaterialInstanceDynamic>( CharacterMesh->GetMaterial(matIndex) );
		if (!IsValid(dynMaterial)) { dynMaterial = UMaterialInstanceDynamic::Create(meshMaterial, this); }
		if (IsValid(dynMaterial))
		{
			dynMaterial->SetVectorParameterValue("Param", meshColor);
			CharacterMesh->SetMaterial(matIndex, dynMaterial);
			return;
		}
	}
}

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
			// Only update if the new animation is not the old one
			if (NewAnimInstance != AnimBlueprint)
			{
				USkeletalMeshComponent* BaseSkeleton = CharacterBase->GetMesh();
				if (BaseSkeleton->GetAnimClass() != NewAnimInstance)
				{
					BaseSkeleton->SetAnimInstanceClass(NewAnimInstance);
					AnimBlueprint = NewAnimInstance;
				}
			}
		}
	}
}

/**
 * Sets the owner actor's skin color WITHOUT performing the mesh merge.
 * If executed on the server, it will be replicated to all clients internally.
 * @param OptionalColor Optional skin color. If unset, will be ignored.
 */
void UMeshMergeComponent::SetSkinMaterial(const FLinearColor OptionalColor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
	if (!IsValid(CharacterMesh)) { return; }
	const FLinearColor BaseColor = FLinearColor();
			
	// If optional color or current skin color is not valid, generate one
	if (OptionalColor == BaseColor)
	{
		TArray<FLinearColor> SkinColors		= CharacterBase->GetCharacterRaceData()->SkinColorOptions;
		const int 			 lastSkinIndex 	= SkinColors.Num() - 1;
		const int 			 idx    		= lastSkinIndex < 0 ? -1 : FMath::RandRange(0,lastSkinIndex);
		SkinColor_ = SkinColors.IsValidIndex(idx) ? SkinColors[idx] : FLinearColor(242, 239, 238, 255);
	}

	// If optional color has been provided, use it
}

/**
 * Sets the owner actor's hair color WITHOUT performing the mesh merge.
 * If executed on the server, it will be replicated to all clients internally.
 * @param OptionalColor Optional hair color. If unset, will be ignored.
 */
void UMeshMergeComponent::SetHairMaterial(const FLinearColor OptionalColor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
	if (!IsValid(CharacterMesh)) { return; }
	const FLinearColor BaseColor = FLinearColor();
	if (OptionalColor == BaseColor)
	{
		TArray<FLinearColor> HairColors		= CharacterBase->GetCharacterRaceData()->HairColorOptions;
		const int 			 lastSkinIndex 	= HairColors.Num() - 1;
		const int 			 idx    		= lastSkinIndex < 0 ? -1 : FMath::RandRange(0,lastSkinIndex);
		HairColor_ = HairColors.IsValidIndex(idx) ? HairColors[idx] : FLinearColor(0, 0, 0, 255);
	}

	else { HairColor_ = OptionalColor; }
}

/**
 * Sets the owner actor's beard color WITHOUT performing the mesh merge.
 * If executed on the server, it will be replicated to all clients internally.
 * @param OptionalColor Optional beard color. If unset, will be ignored.
 */
void UMeshMergeComponent::SetBeardMaterial(const FLinearColor OptionalColor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
	if (!IsValid(CharacterMesh)) { return; }
	const FLinearColor BaseColor = FLinearColor();
	if (OptionalColor == BaseColor)
	{
		TArray<FLinearColor> BeardColors	= CharacterBase->GetCharacterRaceData()->BeardColorOptions;
		if (BeardColors.Num() < 1)
		{
			BeardColors = CharacterBase->GetCharacterRaceData()->HairColorOptions;
		}
		const int 			 lastSkinIndex 	= BeardColors.Num() - 1;
		const int 			 idx    		= lastSkinIndex < 0 ? -1 : FMath::RandRange(0,lastSkinIndex);
		BeardColor_ = BeardColors.IsValidIndex(idx) ? BeardColors[idx] : FLinearColor(0, 0, 0, 255);
	}
	else { BeardColor_ = OptionalColor; }
}

void UMeshMergeComponent::SetEyeMaterial(const FLinearColor OptionalColor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
	if (!IsValid(CharacterMesh)) { return; }
	const FLinearColor BaseColor = FLinearColor();
	if (OptionalColor == BaseColor)
	{
		TArray<FLinearColor> EyeColors		= CharacterBase->GetCharacterRaceData()->EyeColorOptions;
		const int 			 lastSkinIndex 	= EyeColors.Num() - 1;
		const int 			 idx    		= lastSkinIndex < 0 ? -1 : FMath::RandRange(0,lastSkinIndex);
		EyeColor_ = EyeColors.IsValidIndex(idx) ? EyeColors[idx] : FLinearColor(0, 0, 0, 255);
	}

	else { EyeColor_ = OptionalColor; }
}

/**
 * Performs a merge of all supplied meshes and transforms, combining them into
 * one single skeletal mesh on the owning actor. Also updates the skin color
 * and animation instance internally on success.
 * @param bMergeMeshesOnly If true, no material or animation updates will occur.
 * @return True on success
 */
bool UMeshMergeComponent::PerformMeshMerge(bool bMergeMeshesOnly)
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s): PerformMeshMerge()"), *GetName(),
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
		if (!bMergeMeshesOnly)
		{
			SetAnimBlueprint(AnimBlueprint);
		}
		//UpdateMeshMaterials();
		OnMeshMergeCompleted.Broadcast();
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("PerformMeshMerge() failed!"));
	return false;
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

void UMeshMergeComponent::BeginPlay()
{
	ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	CharacterBase_ = CharacterBase;
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

void UMeshMergeComponent::Server_SetAnimBlueprint_Implementation(TSubclassOf<UAnimInstance> NewAnimInstance)
{
	if (GetOwner()->HasAuthority())
	{
		SetAnimBlueprint(AnimBlueprint);
	}
}

/**
 * Performs a mesh merge when the authority sends the updated mesh data
 */
void UMeshMergeComponent::OnRep_MeshMergeData_Implementation()
{
	if (!GetOwner()->HasAuthority())
	{
		PerformMeshMerge(true);
	}
}

void UMeshMergeComponent::OnRep_SkinColor_Implementation()
{
	//if (!GetOwner()->HasAuthority()) { UpdateMeshMaterials(SkinMaterial); }
}

void UMeshMergeComponent::OnRep_BeardColor_Implementation()
{
	//if (!GetOwner()->HasAuthority()) { UpdateMeshMaterials(BeardMaterial); }
}

void UMeshMergeComponent::OnRep_HairColor_Implementation()
{
	//if (!GetOwner()->HasAuthority()) { UpdateMeshMaterials(HairMaterial); }
}

void UMeshMergeComponent::OnRep_EyeColor_Implementation()
{
	//if (!GetOwner()->HasAuthority()) { UpdateMeshMaterials(EyeMaterial); }
}

void UMeshMergeComponent::OnRep_MeshBodyData_Implementation()
{
	//if (!GetOwner()->HasAuthority()) { PerformMeshMerge(true); }
}

void UMeshMergeComponent::OnRep_AnimInstance_Implementation()
{
	//if (!GetOwner()->HasAuthority()) { SetAnimBlueprint(nullptr); }
}


void UMeshMergeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// Vars replicated to all clients
	DOREPLIFETIME(UMeshMergeComponent, Skeleton);
	DOREPLIFETIME(UMeshMergeComponent, MeshMergeData);
	DOREPLIFETIME(UMeshMergeComponent, MeshBodyData);
	DOREPLIFETIME(UMeshMergeComponent, EyeColor_);
	DOREPLIFETIME(UMeshMergeComponent, SkinColor_);
	DOREPLIFETIME(UMeshMergeComponent, HairColor_);
	DOREPLIFETIME(UMeshMergeComponent, BeardColor_);
	DOREPLIFETIME(UMeshMergeComponent, AnimBlueprint);
}
