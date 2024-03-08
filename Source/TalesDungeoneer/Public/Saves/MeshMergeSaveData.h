// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Characters/Components/MeshMergeComponent.h"
#include "GameFramework/SaveGame.h"
#include "MeshMergeSaveData.generated.h"


UCLASS(BlueprintType)
class TALESDUNGEONEER_API UMeshMergeSaveData : public USaveGame
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeleton* UsingSkeleton = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UAnimInstance> UsingAnimInstance = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor EyeColor   = FLinearColor();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor SkinColor  = FLinearColor();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor HairColor  = FLinearColor();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor BeardColor = FLinearColor();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FMeshMergeMappings> MeshMergeMappings = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FMeshBodyMappings>  MeshBodyMappings  = {};
	
};
