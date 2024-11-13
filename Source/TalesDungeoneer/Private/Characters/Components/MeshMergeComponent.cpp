// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Characters/Components/MeshMergeComponent.h"

#include "Characters/CharacterBase.h"
#include "Characters/PlayerCharacterBase.h"
#include "DataAssets/CharacterDefaults.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "lib/ItemData.h"
#include "Logging/StructuredLog.h"						// for UE_LOGFMT
#include "Net/UnrealNetwork.h"
#include "Saves/MeshMergeSaveData.h"			// For loading/saving mesh merges

UMeshMergeComponent::UMeshMergeComponent()
		: StripTopLODS(0), bNeedsCpuAccess(false), bSkeletonBefore(false)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	SexSkeleton	 = static_cast<ECharacterSex>(FMath::RandRange(0,2) );
}


FMeshMergeMappings::FMeshMergeMappings(const UEquipmentDataAsset* NewAsset, bool isFeminine)
	: DataAsset(NewAsset)
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


FMeshBodyMappings::FMeshBodyMappings(USkeletalMesh* UsingMesh,
									 const FGameplayTag& BodyTag, const FGameplayTagContainer& NewOptions)
{
	if (IsValid(UsingMesh))
		{ SkeletalMesh = UsingMesh; }
	BodyPartTag		= BodyTag;
	OptionTags		= NewOptions;
};

/**
 * Performs a merge of all supplied meshes and transforms, combining them into
 * one single skeletal mesh on the owning actor. Also updates the skin color
 * and animation instance internally on success.
 * \param bMergeMeshesOnly If true, no material or animation updates will occur.
 * \return True on success
 */
bool UMeshMergeComponent::PerformMeshMerge(bool bMergeMeshesOnly)
{
	if (!bUseMeshMerge)
	{
		return true;
	}

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
		UE_LOGFMT(LogTemp, Log, "{Name}({Authority}): PerformMeshMerge FAILED - "
			"No valid meshes assigned. Character will be invisible or buggy.", GetName(),
			GetOwner()->HasAuthority()?"SERVER":"CLIENT");
		return false;
	}

	const EMeshBufferAccess BufferAccess = bNeedsCpuAccess ?
			EMeshBufferAccess::ForceCPUAndGPU : EMeshBufferAccess::Default;

	bool bRunDuplicateCheck = false;
	USkeletalMesh* NewBaseMesh = NewObject<USkeletalMesh>();
	if (IsValid(Skeleton) && bSkeletonBefore)
	{
		NewBaseMesh->SetSkeleton(Skeleton);
		bRunDuplicateCheck = true;
		for (const USkeletalMeshSocket* Socket : NewBaseMesh->GetMeshOnlySocketList())
		{
			if (IsValid(Socket))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"),
					*(Socket->SocketName.ToString()) );
			}
		}
		for (const USkeletalMeshSocket* Socket : NewBaseMesh->GetSkeleton()->Sockets)
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

	// Populate the mesh merge data
	for (FMeshMergeMappings& meshMergeData : MeshMergeData)
	{
		// Only show meshes that are not suppressed
		if (meshMergeData.numSuperiorMeshes < 1)
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
	}

	FSkeletalMeshMerge MeshMerger(NewBaseMesh, FinalMeshList, SectionMappingsCopy,
			StripTopLODS, BufferAccess, UvTransformsCopy.GetData());

	if (!MeshMerger.DoMerge())
	{
		UE_LOG(LogTemp, Warning, TEXT("Merge failed!"));
		return false;
	}
	if (IsValid(Skeleton) && !bSkeletonBefore)
	{
		NewBaseMesh->SetSkeleton(Skeleton);
	}

	if (bRunDuplicateCheck)
	{
		TArray<FName> SkeletonMeshSockets;
		TArray<FName> SkeletonSockets;

		for (const USkeletalMeshSocket* Socket : NewBaseMesh->GetMeshOnlySocketList())
		{
			if (Socket)
			{
				SkeletonMeshSockets.Add(Socket->GetFName());
				UE_LOG(LogTemp, Warning, TEXT("SkelMeshSocket: %s"), *(Socket->SocketName.ToString()));
			}
		}

		for (const USkeletalMeshSocket* Socket : NewBaseMesh->GetSkeleton()->Sockets)
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

	if (IsValid(NewBaseMesh))
	{
		USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
		CharacterMesh->SetSkeletalMesh(NewBaseMesh);

		UpdateMeshMaterials();

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
	const UEquipmentDataAsset* NewAsset, const FGameplayTag& EquipmentTag, const bool useFeminineMesh)
{
	// Create a new mesh merge mapping
	FMeshMergeMappings NewMapping(NewAsset, useFeminineMesh);
	NewMapping.EquipSlotTag = EquipmentTag;
	NewMapping.DataAsset	= NewAsset;
	NewMapping.SkeletalMesh = useFeminineMesh ? NewAsset->MeshFeminine : NewAsset->MeshMasculine;
	//NewMapping.AccompaniedMeshes;
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
 * \param NewMapping The new mesh merge mapping to be added
 */
void UMeshMergeComponent::AddMeshToMerge(const FMeshMergeMappings& NewMapping)
{
	if (IsValid(NewMapping.DataAsset) || NewMapping.EquipSlotTag.IsValid())
	{
		// Remove the existing mesh or conflicting meshes of the same type
		RemoveMeshFromMerge(NewMapping.DataAsset, NewMapping.EquipSlotTag);

		// Add the new mesh
		MeshMergeData.Add(NewMapping);
		ValidateMeshMergeMappings(NewMapping);
	}
}

/**
 * Removes the given mesh by tag (priority) or data asset.
 * This function needs to be executed on the server.
 * \param NewAsset Pulls the meshes specified in this asset. Optional.
 * \param EquipmentTag Required. The equipment or body slot tag to use.
 */
void UMeshMergeComponent::RemoveMeshFromMerge(
	const UEquipmentDataAsset* NewAsset, const FGameplayTag& EquipmentTag)
{
	// Always prefer the tag if it is valid. There's only one slot for this tag.
	if (EquipmentTag.IsValid())
	{
		int idx = FindIndexOfMeshByTag(EquipmentTag);
		if (idx >= 0 && MeshMergeData.IsValidIndex(idx))
		{
			// Do not allow the removal of body pieces
			if (!EquipmentTag.MatchesTag(TAG_Character_Body))
			{
				FMeshMergeMappings CopyOfMapping = MeshMergeData[idx];
				MeshMergeData.RemoveAt(idx);
				ValidateMeshMergeMappings(CopyOfMapping, true);
			}
		}
	}

	// If the tag isn't valid, remove the equipment that matches
	else
	{
		for (int i = 0; i < MeshMergeData.Num(); i++)
		{
			if (MeshMergeData[i].DataAsset == NewAsset)
			{
				// Do not allow the removal of body pieces
				if (!EquipmentTag.MatchesTag(TAG_Character_Body))
				{
					FMeshMergeMappings CopyOfMapping = MeshMergeData[i];
					MeshMergeData.RemoveAt(i);
					ValidateMeshMergeMappings(CopyOfMapping, true);
				}
				return;
			}
		}
	}
}

/**
 * \param searchTag The equipment slot tag to search for
 * \return The index of MeshMergeData where the mesh tag was found. -1 on failure.
 */
int UMeshMergeComponent::FindMeshMappingByTag(const FGameplayTag& searchTag)
{
	if (searchTag.IsValid())
	{
		for (int i = 0; i < MeshMergeData.Num(); i++)
		{
			if (MeshMergeData[i].EquipSlotTag == searchTag)
			{
				return i;
			}
		}
	}
	return -1;
}

FMeshMergeMappings UMeshMergeComponent::GetMeshMappingFromIndex(int index)
{
	if (MeshMergeData.IsValidIndex(index))
	{
		return MeshMergeData[index];
	}
	return FMeshMergeMappings();
}

FGameplayTag UMeshMergeComponent::GetBodyPartFromEquipmentSlot(const FGameplayTag& EquipmentSlotTag)
{
	if (EquipmentSlotTag == TAG_Equipment_Slot_Ammunition.GetTag())		return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Anklet.GetTag())			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Arms.GetTag())			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Face.GetTag()) 			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Feet.GetTag()) 			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Head.GetTag()) 			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Legs.GetTag()) 			return TAG_Character_Body_Lower_Legs.GetTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Neck.GetTag()) 			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Primary.GetTag())			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Ranged.GetTag())			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Secondary.GetTag()) 		return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Shoulders.GetTag()) 		return TAG_Character_Body_Armor_Shoulders.GetTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Torso.GetTag()) 			return TAG_Character_Body_Upper_Torso.GetTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Waist.GetTag()) 			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Wrists.GetTag())			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Wrists_Left.GetTag()) 	return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Wrists_Right.GetTag())	return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Ring.GetTag())			return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Ring_Left.GetTag())		return FGameplayTag();
	if (EquipmentSlotTag == TAG_Equipment_Slot_Ring_Right.GetTag())		return FGameplayTag();
	return TAG_Character_Body_Default;
}

void UMeshMergeComponent::SetEyeColor(const FLinearColor NewColor)
{
	EyeColor_ = NewColor;
}

void UMeshMergeComponent::SetSkinColor(const FLinearColor NewColor)
{
	SkinColor_ = NewColor;
}

void UMeshMergeComponent::SetHairColor(const FLinearColor NewColor)
{
	HairColor_ = NewColor;
}

void UMeshMergeComponent::SetBeardColor(const FLinearColor NewColor)
{
	BeardColor_ = NewColor;
}

void UMeshMergeComponent::LoadMeshMerge(
	FString& LoadResponse, FString& SaveSlotName, int32 SaveUserIndex, bool bIsAsync)
{
	LoadResponse = "Failed to Load (Unknown Error)";
	if (!UGameplayStatics::DoesSaveGameExist(SaveFolder + SaveSlotName, SaveUserIndex))
	{
		LoadResponse = "No SaveSlotName Exists";
		return;
	}

	SaveSlotName_  = SaveSlotName;
	SaveUserIndex_ = SaveUserIndex;

	if (bIsAsync)
	{
		FAsyncLoadGameFromSlotDelegate LoadDelegate;
		LoadDelegate.BindUObject(this, &UMeshMergeComponent::LoadDataDelegate);
		UGameplayStatics::AsyncLoadGameFromSlot(SaveFolder + SaveSlotName_, SaveUserIndex_, LoadDelegate);
		LoadResponse = "Sent Async Load Request";
		return;
	}

	LoadDataDelegate(SaveSlotName_, SaveUserIndex_, nullptr);
}

FString UMeshMergeComponent::SaveMeshMerge(FString& responseStr, bool isAsync)
{
	responseStr = "Failed to Save (System Not Ready)";
	const bool doServerSave =	HasAuthority() &&   bSavesOnServer;
	const bool doClientSave = ! HasAuthority() && ! bSavesOnServer;
	if ( !doServerSave || !doClientSave || !GetIsMeshMergeSystemReady())
	{
		// Always allow save if this client is the listen server or standalone
		const ENetMode netMode = GetNetMode();
		if (netMode != NM_ListenServer && netMode != NM_Standalone)
		{
			responseStr = "Authority Violation when Saving MeshMerge!";
			return FString();
		}
	}

	// The inventory save file does not exist, if the string is empty.
	// If so, the script will try to generate one.
	SaveUserIndex_ = 0;

	const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>( GetWorld()->GetGameState() );
	if (IsValid(TalesGameState)) { SaveUserIndex_ = TalesGameState->GetCharacterUserIndex(); }

	if (SaveSlotName_.IsEmpty())
	{
		FString TempSaveName;
		do
		{
			for (int i = 0; i < 18; i++)
			{
				TArray<int> RandValues = {
					FMath::RandRange(48,57), // Numbers 0-9
					FMath::RandRange(65,90) // Uppercase A-Z
				};
				const char RandChar = static_cast<char>(RandValues[FMath::RandRange(0,RandValues.Num()-1)]);
				TempSaveName.AppendChar(RandChar);
			}
		}
		// Loop until a unique save string has been created
		while (UGameplayStatics::DoesSaveGameExist(SaveFolder + TempSaveName, SaveUserIndex_));
		SaveSlotName_ = TempSaveName;

	}

	// If this is a brand new inventory save, issue the starting items.
	UMeshMergeSaveData* SaveData = Cast<UMeshMergeSaveData>
		(UGameplayStatics::CreateSaveGameObject( UMeshMergeSaveData::StaticClass() ));
	if (!IsValid(SaveData))
	{
		responseStr = "Failed to Create New Save Object";
		return FString();
	}

	SaveData->EyeColor		= EyeColor_;
	SaveData->SkinColor		= SkinColor_;
	SaveData->BeardColor	= BeardColor_;
	SaveData->HairColor		= HairColor_;

	SaveData->MeshMergeMappings = MeshMergeData;
	SaveData->MeshBodyMappings  = MeshBodyData;
	SaveData->UsingSkeleton		= Skeleton;
	SaveData->UsingAnimInstance = AnimBlueprint;

	if (isAsync)
	{
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &UMeshMergeComponent::SaveDataDelegate);
		UGameplayStatics::AsyncSaveGameToSlot(SaveData, SaveFolder + SaveSlotName_, SaveUserIndex_, SaveDelegate);
		responseStr = "Successful Asynchronous Save";
		return SaveSlotName_;
	}

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveData, SaveFolder + SaveSlotName_, SaveUserIndex_);
	responseStr = bSuccess ? "Failed to Save" : "Successful Synchronous Save";
	return SaveSlotName_;
}

bool UMeshMergeComponent::HasAuthority() const
{
	if (IsValid(GetOwner()))
		{return GetOwner()->HasAuthority();}
	return false;
}

void UMeshMergeComponent::BeginPlay()
{
	Super::BeginPlay();

	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		CharacterBase->GetMesh()->SetHiddenInGame(false);
	}

	bMeshMergeReady = true;
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

/**
 * Performs a mesh merge when the authority sends the updated mesh data
 */
void UMeshMergeComponent::OnRep_MeshMergeData_Implementation()
{
	UE_LOGFMT(LogTemp, Log, "{Name}({NetAuthority}): OnRep_MeshMergeData() - Mesh Merge Data has been updated"
		, GetOwner()->GetName(), HasAuthority() ? "SRV" : "CLI");
	PerformMeshMerge(true);
}

void UMeshMergeComponent::OnRep_SkinColor_Implementation()
{
	UE_LOGFMT(LogTemp, Log, "{Name}({NetAuthority}): OnRep_SkinColor() - Skin Color has been updated"
		, GetOwner()->GetName(), HasAuthority() ? "SRV" : "CLI");
	ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		UCharacterRaceData* RaceData = CharacterBase->GetCharacterRaceData();
		UpdateMeshMaterials(IsValid(RaceData) ? RaceData->SkinMaterial : nullptr);
	}
}

void UMeshMergeComponent::OnRep_BeardColor_Implementation()
{
	UE_LOGFMT(LogTemp, Log, "{Name}({NetAuthority}): OnRep_BeardColor() - Beard Color has been updated"
		, GetOwner()->GetName(), HasAuthority() ? "SRV" : "CLI");
	ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		UCharacterRaceData* RaceData = CharacterBase->GetCharacterRaceData();
		UpdateMeshMaterials(IsValid(RaceData) ? RaceData->BeardMaterial : nullptr);
	}
}

void UMeshMergeComponent::OnRep_HairColor_Implementation()
{
	UE_LOGFMT(LogTemp, Log, "{Name}({NetAuthority}): OnRep_HairColor() - Hair Color has been updated"
		, GetOwner()->GetName(), HasAuthority() ? "SRV" : "CLI");
	ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		UCharacterRaceData* RaceData = CharacterBase->GetCharacterRaceData();
		UpdateMeshMaterials(IsValid(RaceData) ? RaceData->HairMaterial : nullptr);
	}
}

void UMeshMergeComponent::OnRep_EyeColor_Implementation()
{
	UE_LOGFMT(LogTemp, Log, "{Name}({NetAuthority}): OnRep_EyeColor() - Eye Color has been updated"
		, GetOwner()->GetName(), HasAuthority() ? "SRV" : "CLI");
	ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		UCharacterRaceData* RaceData = CharacterBase->GetCharacterRaceData();
		UpdateMeshMaterials(IsValid(RaceData) ? RaceData->EyeMaterial : nullptr);
	}
}

void UMeshMergeComponent::OnRep_MeshBodyData_Implementation()
{
	UE_LOGFMT(LogTemp, Log, "{Name}({NetAuthority}): 'MeshBodyData' has been updated"
		, GetOwner()->GetName(), HasAuthority() ? "SRV" : "CLI");
	PerformMeshMerge(true);
}

void UMeshMergeComponent::OnRep_AnimInstance_Implementation()
{
	ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))
	{
		CharacterBase->GetMesh()->SetAnimInstanceClass(AnimBlueprint);
	}
}

/**
 * Goes through the meshes, hiding meshes and showing meshes as appropriate.
 * \param MapReference The mesh mapping data
 * \param bWasRemoved True flips the hide/show mutators
 */
void UMeshMergeComponent::ValidateMeshMergeMappings(
		const FMeshMergeMappings& MapReference, bool bWasRemoved)
{
	if (IsValid(MapReference.DataAsset))
	{
		for (FMeshMergeMappings& meshMapping : MeshMergeData)
		{
			// If the new mesh hides the existing meshes, set appropriately.
			if (MapReference.DataAsset->HidesBodyParts.HasTagExact(meshMapping.EquipSlotTag))
			{
				// Show if bWasRemoved, otherwise hide.
				meshMapping.numSuperiorMeshes += bWasRemoved ? -1 :  1;
			}

			// Override hidden meshes by allowing shown meshes
			if (MapReference.DataAsset->ShowsBodyParts.HasTagExact(meshMapping.EquipSlotTag))
			{
				// Hide if bWasRemoved, otherwise show.
				meshMapping.numSuperiorMeshes -= bWasRemoved ?  1 : -1;
			}
		}
	}
}

void UMeshMergeComponent::UpdateMeshMaterials(const UMaterialInterface* MaterialInterface)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase))		{ return; }

	USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
	if (!IsValid(CharacterMesh))		{ return; }

	// Get all materials from the character's mesh component
	TArray<UMaterialInterface*> meshMats = CharacterMesh->GetMaterials();

	for (UMaterialInterface* meshMaterial : meshMats)
	{
		if (!IsValid(meshMaterial)) {continue;}

		int matIndex = -1;
		// Find the index where the specified mesh is found
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
		// Continue to the next material if this material doesn't match the search term
		if (matIndex < 0)
		{
			continue;
		}

		FLinearColor meshColor;
		const UCharacterRaceData* RaceData = CharacterBase->GetCharacterRaceData();
		if (IsValid(RaceData))
		{
			if(meshMaterial == RaceData->EyeMaterial)		 { meshColor = EyeColor_;   }
			else if(meshMaterial == RaceData->SkinMaterial)  { meshColor = SkinColor_;  }
			else if(meshMaterial == RaceData->HairMaterial)  { meshColor = HairColor_;  }
			else if(meshMaterial == RaceData->BeardMaterial) { meshColor = BeardColor_; }
			else { return; }
		}

		// Use the existing dynamic material, or create a new one
		UMaterialInstanceDynamic* dynMaterial = Cast<UMaterialInstanceDynamic>( CharacterMesh->GetMaterial(matIndex) );
		if (!IsValid(dynMaterial)) { dynMaterial = UMaterialInstanceDynamic::Create(meshMaterial, this); }

		if (IsValid(dynMaterial))
		{
			dynMaterial->SetVectorParameterValue("Param", meshColor);
			CharacterMesh->SetMaterial(matIndex, dynMaterial);
		}
	}
}

void UMeshMergeComponent::LoadDataDelegate(const FString& SaveName, const int32 UserIndex, USaveGame* SaveGameData)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): LoadDataDelegate()"
		, HasAuthority() ? "SRV" : "CLI");
	UMeshMergeSaveData* MeshMergeSave = Cast<UMeshMergeSaveData>( SaveGameData );
	if (!IsValid(MeshMergeSave))
	{
		MeshMergeSave = Cast<UMeshMergeSaveData>
			( UGameplayStatics::LoadGameFromSlot(SaveFolder + SaveSlotName_, SaveUserIndex_) );
		if (!IsValid(MeshMergeSave))
		{
			OnMeshMergeRestored.Broadcast(false);
			UE_LOGFMT(LogTemp, Error, "MeshMerge({NetAuthority}): LoadDataDelegate() FAILURE - MeshMergeSave does not exist"
				, HasAuthority() ? "SRV" : "CLI");
			return;
		}
	}

	RestoreSkeleton(MeshMergeSave->UsingSkeleton,
						   MeshMergeSave->UsingAnimInstance);

	RestoreMappings(MeshMergeSave->MeshMergeMappings,
						   MeshMergeSave->MeshBodyMappings);

	RestoreMaterials(MeshMergeSave->EyeColor, MeshMergeSave->SkinColor,
							MeshMergeSave->HairColor, MeshMergeSave->BeardColor);

	OnMeshMergeRestored.Broadcast(true);
}

void UMeshMergeComponent::SaveDataDelegate(const FString& SaveName, const int32 UserIndex, bool bSuccess)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): SaveDataDelegate({SaveName}, {UserIndex}, {bSuccess})"
		, HasAuthority() ? "SRV" : "CLI", SaveName, UserIndex, bSuccess);
	OnMeshMergeSaved.Broadcast(bSuccess);
}


void UMeshMergeComponent::Server_RestoreSkeleton_Implementation(
	USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): Server_RestoreSkeleton()"
		, HasAuthority() ? "SRV" : "CLI");
	Skeleton = NewSkeleton;
	AnimBlueprint = NewAnimInstance;
}

void UMeshMergeComponent::RestoreSkeleton(
	USkeleton* NewSkeleton, TSubclassOf<UAnimInstance> NewAnimInstance)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): RestoreSkeleton()"
		, HasAuthority() ? "SRV" : "CLI");
	if (!IsValid(NewAnimInstance))
	{
		UE_LOGFMT(LogTemp, Error, "MeshMerge({NetAuthority}): RestoreSkeleton() FAILURE - Invalid AnimInstance"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase))
	{
		UE_LOGFMT(LogTemp, Error, "MeshMerge({NetAuthority}): RestoreSkeleton() FAILURE - Invalid CharacterBase Reference"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	USkeletalMeshComponent* CharacterMesh = CharacterBase->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		UE_LOGFMT(LogTemp, Error, "MeshMerge({NetAuthority}): RestoreSkeleton() FAILURE - Invalid CharacterMesh Component"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	Skeleton		= NewSkeleton;
	AnimBlueprint	= NewAnimInstance;
	CharacterMesh->SetAnimInstanceClass(NewAnimInstance);

	if (IsValid(Cast<APlayerCharacterBase>(GetOwner())))
		Server_RestoreSkeleton(NewSkeleton, NewAnimInstance);
}


void UMeshMergeComponent::Server_RestoreMappings_Implementation(
	const TArray<FMeshMergeMappings>& NewMeshMappings,
	const TArray<FMeshBodyMappings>& NewBodyMappings)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): Server_RestoreMappings()"
		, HasAuthority() ? "SRV" : "CLI");
	MeshBodyData  = NewBodyMappings;
	MeshMergeData = NewMeshMappings;
	PerformMeshMerge();
}

void UMeshMergeComponent::RestoreMappings(
	const TArray<FMeshMergeMappings>& NewMeshMappings,
	const TArray<FMeshBodyMappings>& NewBodyMappings)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): RestoreMappings()"
		, HasAuthority() ? "SRV" : "CLI");

	if (IsValid(Cast<APlayerCharacterBase>(GetOwner())))
		Server_RestoreMappings(NewMeshMappings, NewBodyMappings);
}


void UMeshMergeComponent::Server_RestoreMaterials_Implementation(FLinearColor NewEyeColor, FLinearColor NewSkinColor,
	FLinearColor NewHairColor, FLinearColor NewBeardColor)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): Server_RestoreMaterials()"
		, HasAuthority() ? "SRV" : "CLI");
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase))
	{
		UE_LOGFMT(LogTemp, Error, "MeshMerge({NetAuthority}): Server_RestoreMaterials() - Invalid CharacterBase Reference"
			, HasAuthority() ? "SRV" : "CLI");
		return;
	}

	EyeColor_ = NewEyeColor;	SkinColor_ = NewSkinColor;
	HairColor_ = NewHairColor;	BeardColor_ = NewBeardColor;

	const UCharacterRaceData* RaceData = CharacterBase->GetCharacterRaceData();
	if (IsValid(RaceData))
	{
		UpdateMeshMaterials(RaceData->EyeMaterial);
		UpdateMeshMaterials(RaceData->SkinMaterial);
		UpdateMeshMaterials(RaceData->HairMaterial);
		UpdateMeshMaterials(RaceData->BeardMaterial);
	}
}

void UMeshMergeComponent::RestoreMaterials(FLinearColor NewEyeColor, FLinearColor NewSkinColor,
                                           FLinearColor NewHairColor, FLinearColor NewBeardColor)
{
	UE_LOGFMT(LogTemp, Display, "MeshMerge({NetAuthority}): RestoreMaterials()"
		, HasAuthority() ? "SRV" : "CLI");

	if (IsValid(Cast<APlayerCharacterBase>(GetOwner())))
		Server_RestoreMaterials(NewEyeColor, NewSkinColor, NewHairColor, NewBeardColor);
}

// ////////////////////////////////////////////////////
// REPLICATION

void UMeshMergeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Vars replicated to all clients
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, Skeleton,		COND_None);
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, MeshMergeData, COND_None);
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, MeshBodyData,  COND_None);
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, EyeColor_,		COND_None);
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, SkinColor_,	COND_None);
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, HairColor_,	COND_None);
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, BeardColor_,	COND_None);
	DOREPLIFETIME_CONDITION(UMeshMergeComponent, AnimBlueprint, COND_None);
}
