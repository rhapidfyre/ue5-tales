// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "MeshMergeComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"
#include "TalesDungeoneer/lib/GameplayTags.h"
#include "TalesDungeoneer/lib/datastructures/EquipmentWorn.h"


FStMeshMergeData::FStMeshMergeData(FName DataRowName, bool IsMale)
{
	const FStEquipmentWorn EquipmentData = UEquipmentSystem::GetEquipmentWornData(DataRowName);
	if (UEquipmentSystem::GetEquipmentWornDataIsValid(EquipmentData))
	{
		// Set Static Data
		ItemName			= DataRowName;
		MeshAsset			= IsMale ? EquipmentData.MaleMesh : EquipmentData.FemaleMesh;
		
		// Set this equipment mesh to the appropriate slot
		const FGameplayTag DefaultTag = TAG_Character_Body_Default.GetTag();
		for (FGameplayTag BodyPartTag : EquipmentData.BodyPartTags)
		{
			// Ignore the default tag
			if (!BodyPartTag.MatchesTag(DefaultTag))
			{
				AssociatedBodyPart = BodyPartTag;
				break;
			}
		}
		
		// Remember what parts should be hidden
		for (FGameplayTag HiddenBodyPart : EquipmentData.HideBodyParts)
		{
			HidesBodyParts.AddTag(HiddenBodyPart);
		}
	}
}

UMeshMergeComponent::UMeshMergeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool UMeshMergeComponent::PerformMeshMerge()
{
	UE_LOG(LogTemp, Warning, TEXT("%s(%s): PerformMeshMerge()"), *GetName(),
		GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	const ENetMode NetMode = GetNetMode();
	if (MeshesToMerge.IsEmpty())
	{
		if (DefaultMeshes.IsEmpty())
		{
			InitializeDefaultMeshes();
		}
		MeshesToMerge = DefaultMeshes;
	}
	
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetOwner());
	if (!IsValid(CharacterBase))
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner Actor was not a valid CharacterBase!"));
		return false;
	}
	
	// Removes all invalid skeletal meshes from the array copy
	// Invalid mesh assets will return TRUE, which removes it from the array.
	MeshesToMerge.RemoveAll([](FStMeshMergeData InternalMeshMergeData)
	{
		// Returns within the lambda
		return (!IsValid(InternalMeshMergeData.MeshAsset));  
	});

	// Checks for empty array
	if (MeshesToMerge.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Must provide multiple valid Skeletal Meshes in order to perform a merge."));
		return nullptr;
	}

	EMeshBufferAccess BufferAccess = bNeedsCpuAccess ?
		EMeshBufferAccess::ForceCPUAndGPU : EMeshBufferAccess::Default;
	
	TArray<FSkelMeshMergeSectionMapping> SectionMappingsCopy = MeshSectionMappings;
	TArray<FSkelMeshMergeUVTransformMapping> UvTransformsCopy = UvTransformsPerMesh;

	bool bRunDuplicateCheck = false;
	USkeletalMesh* BaseMesh = NewObject<USkeletalMesh>();
	if (IsValid(Skeleton) && bSkeletonBefore)
	{
		BaseMesh->SetSkeleton(Skeleton);
		bRunDuplicateCheck = true;
		for (USkeletalMeshSocket* Socket : BaseMesh->GetMeshOnlySocketList())
		{
			if (IsValid(Socket))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"),
					*(Socket->SocketName.ToString()) );
			}
		}
		for (USkeletalMeshSocket* Socket : BaseMesh->GetSkeleton()->Sockets)
		{
			if (IsValid(Socket))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkelSocket: %s"),
					*(Socket->SocketName.ToString()) );
			}
		}
		
	}

	TArray<USkeletalMesh*> FinalMeshList;
	for (FStMeshMergeData& MeshMergeData : MeshesToMerge)
	{
		FinalMeshList.Add(MeshMergeData.MeshAsset);
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
	if (IsValid(AnimBlueprint))
	{
		//CharacterBase->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		CharacterBase->GetMesh()->SetAnimInstanceClass(AnimBlueprint);
	}
	if (bRunDuplicateCheck)
	{
		TArray<FName> SkelMeshSockets;
		TArray<FName> SkelSockets;
		
		for (USkeletalMeshSocket* Socket : BaseMesh->GetMeshOnlySocketList())
		{
			if (Socket)
			{
				SkelMeshSockets.Add(Socket->GetFName());
				UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"), *(Socket->SocketName.ToString()));
			}
		}
		
		for (USkeletalMeshSocket* Socket : BaseMesh->GetSkeleton()->Sockets)
		{
			if (Socket)
			{
				SkelSockets.Add(Socket->GetFName());
				UE_LOG(LogTemp, Warning, TEXT("SkelSocket: %s"), *(Socket->SocketName.ToString()));
			}
		}
		
		TSet<FName> UniqueSkelMeshSockets;
		TSet<FName> UniqueSkelSockets;
		UniqueSkelMeshSockets.Append(SkelMeshSockets);
		UniqueSkelSockets.Append(SkelSockets);
		int32 Total = SkelSockets.Num() + SkelMeshSockets.Num();
		int32 UniqueTotal = UniqueSkelMeshSockets.Num() + UniqueSkelSockets.Num();
		UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocketCount: %d | SkelSocketCount: %d | Combined: %d"), SkelMeshSockets.Num(), SkelSockets.Num(), Total);
		UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocketCount: %d | SkelSocketCount: %d | Combined: %d"), UniqueSkelMeshSockets.Num(), UniqueSkelSockets.Num(), UniqueTotal);
		UE_LOG(LogTemp, Warning, TEXT("Found Duplicates: %s"), *((Total != UniqueTotal) ? FString("True") : FString("False")));
	}
	
	if (IsValid(BaseMesh))
	{
		CharacterBase->GetMesh()->SetSkeletalMesh(BaseMesh);
		return true;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("PerformMeshMerge() failed!"));
	return false;
}


void UMeshMergeComponent::InitializeMeshMerge(const USavedCharacter* CharacterData)
{
	InitializeDefaultMeshes();

	if (bHasInitialized)
	{
		if (!IsValid(CharacterData))
		{
			UE_LOG(LogTemp, Warning, TEXT("InitializeMeshMerge(%s): Already Initialized!"),
				GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
			return;
		}
		PerformMeshMerge();
		return;
	}

	if (IsValid(CharacterData))
	{
		if (GetOwner()->HasAuthority())
		{
			bHasInitialized = true;
			Skeleton				= CharacterData->Skeleton;			
			MeshSectionMappings		= CharacterData->MeshSectionMappings; 
			UvTransformsPerMesh		= CharacterData->UvTransformsPerMesh; 
			MeshesToMerge			= CharacterData->MeshesToMerge;
			PerformMeshMerge();
		}
		else
		{
			/*
			Server_InitializeMeshMerge(
				CharacterData->Skeleton,
				CharacterData->MeshSectionMappings,
				CharacterData->UvTransformsPerMesh,
				CharacterData->MeshesToMerge );
			bHasInitialized = true;
			*/
		}
		return;
	}
	
	// If save data isn't valid, use the defaults
	if (!GetOwner()->HasAuthority())
	{
		/*
		Server_InitializeMeshMerge(Skeleton,
			MeshSectionMappings, UvTransformsPerMesh, MeshesToMerge);
		*/
		return;
	}
	
	// if the mesh merge array is empty by default, setup the default base
	if (MeshesToMerge.Num() < 1)
	{
		for (FStMeshMergeData MeshMergeData : DefaultMeshes)
		{
			MeshesToMerge.Add(MeshMergeData);
		}
		bHasInitialized = true;
		PerformMeshMerge();
	}
	
}

void UMeshMergeComponent::Server_InitializeMeshMerge_Implementation(USkeleton* NewSkeleton,
	const TArray<FSkelMeshMergeSectionMapping>& NewMeshMaps,
	const TArray<FSkelMeshMergeUVTransformMapping>& NewUvTransforms,
	const TArray<FStMeshMergeData>& NewMeshes)
{
	// The client can only initialize once
	if (!bHasInitialized)
	{
		Skeleton				= NewSkeleton;			
		MeshSectionMappings		= NewMeshMaps; 
		UvTransformsPerMesh		= NewUvTransforms; 
		MeshesToMerge			= NewMeshes;
		InitializeMeshMerge();
	}
}

/**
 * @brief Returns the index of the Mesh Merge array for the given tag, if found.
 * @param SearchTag The FGameplayTag to find in the mesh merge array
 * @return -1 on Failure, otherwise, the array index where tag was found
 */
int UMeshMergeComponent::GetMeshMergeIndexByTag(FGameplayTag SearchTag) const
{
	for (int i = 0; i < MeshesToMerge.Num(); i++)
	{
		const FStMeshMergeData MergeData = MeshesToMerge[i];
		if (MergeData.AssociatedBodyPart.MatchesTag(SearchTag))
			return i;
	}
	return -1;
}

/**
 * @brief Inserts a new entry into the array, returning the index where it was added to.
 *        Does nothing, and returns the index if the game tag already exists in the array.
 * @param GameTag The new tag to add (Character.Body.Torso, etc)
 * @param EquipmentName The equipment to slot into the merge.
 * @param IsMale False if the mesh is female specific. Otherwise, true.
 * @return Returns the index the new data was emplaced at. Negative indicates failure.
 */
int UMeshMergeComponent::AddNewMeshToArrayByTag(FGameplayTag GameTag, FName EquipmentName, bool IsMale)
{
	if (EquipmentName.IsNone())
		return -1;
	
	const int ExistingIndex = GetMeshMergeIndexByTag(GameTag);
	if (ExistingIndex >= 0)
		return ExistingIndex;
	
	// If we still haven't returned, the game tag doesn't exist. Add it.
	const FStMeshMergeData NewMeshData(EquipmentName);
	return MeshesToMerge.Add( NewMeshData );
}

/**
 * @brief Replaces the merge at the given array index with the new mesh
 * @param EquipmentName The new mesh to use. Invalid FName results in default mesh use.
 * @param ArrayIndex The array index to modify
 * @param IsMale False if female body, otherwise true.
 * @param MergeNow If true, performs a mesh merge as soon as the array updates
 */
void UMeshMergeComponent::SetNewMeshByIndex(FName EquipmentName, int ArrayIndex, bool IsMale, bool MergeNow)
{
	const FStEquipmentWorn EquipData = UEquipmentSystem::GetEquipmentWornData(EquipmentName);
		
	if (MeshesToMerge.IsValidIndex(ArrayIndex))
	{
		// Use the mesh given
		USkeletalMesh* UsingMesh = IsMale ? EquipData.MaleMesh : EquipData.FemaleMesh;
		if (IsValid(UsingMesh))
			MeshesToMerge[ArrayIndex].MeshAsset = UsingMesh;

		else
		{
			// Get the default mesh for this slot
			FGameplayTag BodyTag = MeshesToMerge[ArrayIndex].AssociatedBodyPart;
			FStMeshMergeData MeshMergeData = UEquipmentSystem::GetDefaultMeshFromTag(BodyTag, IsMale);
			
			if (IsValid(MeshMergeData.MeshAsset))
			{
				MeshesToMerge[ArrayIndex].MeshAsset = MeshMergeData.MeshAsset;
			}
		}
		
		if (MergeNow)
			PerformMeshMerge();
	}
}

/**
 * @brief Replaces the merge at the given array index with the new mesh.
 *        If no mesh merge exists for the given tag, internally runs AddNewMeshToArrayByTag.
 * @param EquipmentName The new mesh to use. Invalid FName results in default mesh being used.
 * @param GameTag The body part tag to look for when replacing the mesh.
 * @param IsMale False if female body, otherwise true.
 * @param MergeNow If true, performs a mesh merge as soon as the array updates
 */
void UMeshMergeComponent::SetNewMeshByTag(FName EquipmentName, FGameplayTag GameTag, bool IsMale, bool MergeNow)
{
	int ArrayIndex = GetMeshMergeIndexByTag(GameTag);
	if (ArrayIndex < 0)
	{
		ArrayIndex = AddNewMeshToArrayByTag(GameTag, FName());
	}
	if (ArrayIndex >= 0)
	{
		SetNewMeshByIndex(EquipmentName, ArrayIndex, IsMale, MergeNow);
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

void UMeshMergeComponent::InitializeDefaultMeshes()
{
	if (DefaultMeshes.Num() < 1)
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetOwner());
		if (IsValid(CharacterBase))
		{
			DefaultMeshes = UEquipmentSystem::GetAllDefaultMeshes();
		}
	}
}

void UMeshMergeComponent::OnRep_MeshesToMerge_Implementation()
{
	if (GetNetMode() >= NM_Client)
		PerformMeshMerge();
}

void UMeshMergeComponent::OnRep_UvTransformsPerMesh_Implementation()
{
	if (GetNetMode() >= NM_Client)
		PerformMeshMerge();
}

void UMeshMergeComponent::OnRep_MeshSectionMappings_Implementation()
{
	if (GetNetMode() >= NM_Client)
		PerformMeshMerge();
}

void UMeshMergeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMeshMergeComponent, Skeleton);
	DOREPLIFETIME(UMeshMergeComponent, MeshesToMerge);
	DOREPLIFETIME(UMeshMergeComponent, MeshSectionMappings);
	DOREPLIFETIME(UMeshMergeComponent, UvTransformsPerMesh);
}
