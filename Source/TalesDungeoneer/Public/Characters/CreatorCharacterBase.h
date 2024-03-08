// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "CoreMinimal.h"
#include "PlayerCharacterBase.h" // Includes core and actor files

#include "CreatorCharacterBase.generated.h"


/**
 * Base class for all logic regarding the character creator
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACreatorCharacterBase : public APlayerCharacterBase
{
	GENERATED_BODY()

public: // functions
	
	ACreatorCharacterBase();

	UFUNCTION(BlueprintCallable)
	bool FinishCreation() { return CreateCharacter(); }

	UFUNCTION(BlueprintCallable)
	void SetIsBeingCreated(bool bIsBeingCreated = true);

	UFUNCTION(BlueprintPure)
	bool GetIsBeingCreated() const { return bIsCreating; }
	
	virtual bool CreateCharacter();

	void GetAllBodyPartMeshes(TArray<USkeletalMeshComponent*>& BodyPartMeshes);

	UFUNCTION(BlueprintCallable)
	void ResetMeshSelections();
	
	UFUNCTION(BlueprintCallable)
	void ClearChildrenOfMesh(USkeletalMeshComponent* ReferenceMesh);

	virtual bool LoadCharacter(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame) override;
	virtual USaveGame* SaveCharacter(USaveGame* SaveObject, bool bRunAsync) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartHair		 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartEyebrows  = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartEyes		 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartBeard	 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartHead		 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartNeck		 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartChest	 = nullptr;
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartBra		 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartArms		 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartHands	 = nullptr;
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartUnderwear = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartLegs 	 = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite) USkeletalMeshComponent* MeshPartFeet 	 = nullptr;
	
protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

private:

	USkeletalMeshComponent* InitMeshPart(FName MeshName);

	FString NewSaveSlotName_ = "";
	int32  NewSaveUserIndex_ = 0;

	bool bIsCreating = false;
	
};