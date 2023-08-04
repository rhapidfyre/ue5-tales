// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "MeshMergeComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"
#include "TalesDungeoneer/lib/datastructures/EquipmentWorn.h"


FStMeshMergeData::FStMeshMergeData(FName DataRowName, bool IsMale)
{
	const FStEquipmentWorn EquipmentData = UEquipmentSystem::GetEquipmentWornData(DataRowName);
	if (UEquipmentSystem::GetEquipmentWornDataIsValid(EquipmentData))
	{
		// Set Static Data
		ItemName			= DataRowName;
		MeshAsset			= IsMale ? EquipmentData.MaleMesh : EquipmentData.FemaleMesh;
		AssociatedBodyPart	= EquipmentData.BodyPartName;

		// Remember what parts should be hidden
		for (FGameplayTag HiddenBodyPart : EquipmentData.HideBodyParts)
			HidesBodyParts.AddTag(HiddenBodyPart);
	}
}

UMeshMergeComponent::UMeshMergeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool UMeshMergeComponent::PerformMeshMerge()
{
	// System is not initialized, OR meshes array is empty
	if (!bHasInitialized && MeshesToMerge.Num() < 1)
	{
		InitializeMeshMerge();
		return true;
	}
	
	if (!IsValid(GetOwner()))
	{
		//UE_LOG(LogTemp, Warning, TEXT("Owner Actor was not a valid Actor!"));
		return false;
	}
	
	ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetOwner());
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
	if (MeshesToMerge.Num() < 1)
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
	if (Skeleton && !bSkeletonBefore)
	{
		BaseMesh->SetSkeleton(Skeleton);
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
		UE_LOG(LogTemp, Display, TEXT("Merge SUCCESS!"));
		bHasInitialized = true;
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("PerformMeshMerge() failed!"));
	return false;
}


void UMeshMergeComponent::InitializeMeshMerge()
{
	if (!bHasInitialized || MeshesToMerge.Num() < 1)
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
		if (IsValid(CharacterBase))
		{
			InitializeDefaultMeshes();

			// if the mesh merge array is empty by default, setup the default base
			if (MeshesToMerge.Num() < 1)
			{
				// Throw Error
				checkf(DefaultMeshes.Num() > 0,
					TEXT("There are no default meshes defined in DT_BodyParts for this race and class combination!!"));

				for (FStMeshMergeData MeshMergeData : DefaultMeshes)
				{
					MeshesToMerge.Add(MeshMergeData);
				}
			}
			
			PerformMeshMerge();
			bHasInitialized = true;
		}
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
	InitializeMeshMerge();
	
}

void UMeshMergeComponent::InitializeComponent()
{
	Super::InitializeComponent();
	InitializeMeshMerge();
}

void UMeshMergeComponent::InitializeDefaultMeshes()
{
	if (DefaultMeshes.Num() < 1)
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetOwner());
		if (IsValid(CharacterBase))
		{
			UDataTable* EquipmentDt = UEquipmentSystem::GetEquipmentWornDataTable();
			if (!IsValid(EquipmentDt))
				return;
			
			TArray<FName> AllRowNames = EquipmentDt->GetRowNames();
			for (FName EquipmentRowName : AllRowNames)
			{
				if (UEquipmentSystem::GetEquipmentWornNameIsValid(EquipmentRowName))
				{
					FStEquipmentWorn WornData = UEquipmentSystem::GetEquipmentWornData(EquipmentRowName);
					FStMeshMergeData MeshMergeData(EquipmentRowName);
					DefaultMeshes.Add(MeshMergeData);
				}
			}
		}
	}
}
