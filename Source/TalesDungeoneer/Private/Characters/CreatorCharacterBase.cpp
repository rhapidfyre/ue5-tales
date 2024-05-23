// Copyright Take Five Games, LLC 2023 - All rights reserved

#include "Characters/CreatorCharacterBase.h"

#include "DataAssets/CharacterDefaults.h"
#include "Gamemode/TitleScreen/CreatorGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"


// Sets default values
ACreatorCharacterBase::ACreatorCharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Each mesh must be declared individually or Blueprint can't differentiate between array indices
	MeshPartHair		= InitMeshPart("MeshPartHair");
	MeshPartEyebrows	= InitMeshPart("MeshPartEyebrows");
	MeshPartEyes		= InitMeshPart("MeshPartEyes");
	MeshPartBeard		= InitMeshPart("MeshPartBeard");
	MeshPartHead		= InitMeshPart("MeshPartHead");
	MeshPartNeck		= InitMeshPart("MeshPartNeck");
	MeshPartChest		= InitMeshPart("MeshPartChest");
	//MeshPartBra		= InitMeshPart("MeshPartBra");
	MeshPartArms		= InitMeshPart("MeshPartArms");
	MeshPartHands		= InitMeshPart("MeshPartHands");
	//MeshPartUnderwear	= InitMeshPart("MeshPartUnderwear");
	MeshPartLegs		= InitMeshPart("MeshPartLegs");
	MeshPartFeet		= InitMeshPart("MeshPartFeet");
}

void ACreatorCharacterBase::SetIsBeingCreated(bool bIsBeingCreated)
{
	//GetMesh()->SetSkeletalMeshAsset(nullptr);
	ResetMeshSelections();
	GetMesh()->SetVisibility(bIsBeingCreated);
	bIsCreating = bIsBeingCreated;
}

bool ACreatorCharacterBase::CreateCharacter()
{
	ACreatorGameStateBase* TalesGameState =
			Cast<ACreatorGameStateBase>(GetWorld()->GetGameState());
	
	if (IsValid(TalesGameState))
	{
		if (!TalesGameState->GetIsCreatingCharacter())
		{
			UE_LOGFMT(LogTemp, Warning, "Creation of Character {NewName} Rejected - Not in Character Creator", GetCharacterName());
			return false;
		}
		
		NewSaveSlotName_	= GetCharacterSafeName();
		NewSaveUserIndex_	= TalesGameState->GetCharacterUserIndex();
	
		// Allow creation as long as this character doesn't already exist
		if (UGameplayStatics::DoesSaveGameExist(NewSaveSlotName_, NewSaveUserIndex_))
		{
			UE_LOGFMT(LogTemp, Warning, "Creation of Character {NewName} Rejected - Already Exists", NewSaveSlotName_);
			NewSaveSlotName_	= "";
			NewSaveUserIndex_	= 0;
			return false;
		}

		// Do Initial Character Setup
		UE_LOGFMT(LogTemp, Display, "Attempting to create new character "
			"'{CharacterName}' in Save Slot: '{SaveName}, {UserIndex}'",
			GetCharacterName(), NewSaveSlotName_, NewSaveUserIndex_);

		FString SaveResponse = "";
		const bool saveSuccess = TalesGameState->CreateNewCharacter(SaveResponse, false);
		UE_LOGFMT(LogTemp, Display, "Creation of Character {NewName} Response: {SaveResponse}", GetCharacterName(), SaveResponse);
		return saveSuccess;
	}
	return false;
}

void ACreatorCharacterBase::GetAllBodyPartMeshes(TArray<USkeletalMeshComponent*>& BodyPartMeshes)
{
	BodyPartMeshes = {
		MeshPartHair,		MeshPartEyebrows,	MeshPartEyes,		MeshPartBeard,
		MeshPartHead,		MeshPartChest,		MeshPartArms,
		MeshPartHands,		MeshPartLegs,		MeshPartFeet,
		MeshPartNeck
	};
}

/**
 * Resets all of the mesh parts to clear/none, and destroys each of their children
 */
void ACreatorCharacterBase::ResetMeshSelections()
{
	TArray<USkeletalMeshComponent*> BodyPartMeshes;
	GetAllBodyPartMeshes(BodyPartMeshes);
	for (USkeletalMeshComponent* MeshPart : BodyPartMeshes)
	{
		if (IsValid(MeshPart))
		{
			ClearChildrenOfMesh(MeshPart);
		}
		MeshPart->SetSkeletalMeshAsset(nullptr);
	}
}

/**
 * Destroys all of the child components of the given mesh part
 * \param ReferenceMesh The mesh part to reference
 */
void ACreatorCharacterBase::ClearChildrenOfMesh(USkeletalMeshComponent* ReferenceMesh)
{
	if (IsValid(ReferenceMesh))
	{
		TArray<USkeletalMeshComponent*> BodyPartMeshes;
		GetAllBodyPartMeshes(BodyPartMeshes);
		if (BodyPartMeshes.Contains(ReferenceMesh))
		{
			TArray<USceneComponent*> MeshChildren;
			ReferenceMesh->GetChildrenComponents(true, MeshChildren);
			for (USceneComponent* MeshChild : MeshChildren)
			{
				MeshChild->DestroyComponent(false);
			}
		}
	}
}

bool ACreatorCharacterBase::LoadCharacter(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame)
{
	if (GetIsBeingCreated()) { return false; }
	return Super::LoadCharacter(SlotName, UserIndex, SaveGame);
}

USaveGame* ACreatorCharacterBase::SaveCharacter(USaveGame* SaveObject, bool bRunAsync)
{
	return Super::SaveCharacter(SaveObject, bRunAsync);
}

void ACreatorCharacterBase::BeginPlay()
{
	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	if (IsValid(SkeletalMesh))
	{
		SkeletalMesh->SetVisibility(false);
	}
	Super::BeginPlay();
}

void ACreatorCharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	USkeletalMeshComponent* SkinnedMesh = GetMesh();

	TArray<USkeletalMeshComponent*> BodyPartMeshes;
	GetAllBodyPartMeshes(BodyPartMeshes);
	for (USkeletalMeshComponent* meshComponent : BodyPartMeshes)
	{
		meshComponent->SetLeaderPoseComponent(SkinnedMesh);
	}
	SkinnedMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	//SkinnedMesh->SetAnimInstanceClass(MeshMergeComponent->AnimBlueprint);
}

USkeletalMeshComponent* ACreatorCharacterBase::InitMeshPart(FName MeshName)
{
	USkeletalMeshComponent* BodyPartMesh = CreateDefaultSubobject<USkeletalMeshComponent>(MeshName);
	if (IsValid(BodyPartMesh)) { BodyPartMesh->SetupAttachment( GetMesh() ); }
	return BodyPartMesh;
}
