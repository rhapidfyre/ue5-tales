// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"

#include "EquipmentWorn.generated.h"

struct FStMeshMergeData;
/**
 * Struct that holds all of the data for individual pieces of equipment
 * The name of the data table row should be identical to the proper inventory item name
 */
USTRUCT(BlueprintType)
struct FStEquipmentWorn : public FTableRowBase
{
	GENERATED_BODY()
	
	// Associated Body Part - i.e. equipment slot (torso, neck, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer BodyPartTags = {};

	// Races that should be allowed to use this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<ECharacterRace> AllowedRaces = { ECharacterRace::ANY };

	// Classes that should be allowed to use this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<ECharacterClass> AllowedClasses = { ECharacterClass::ANY };

	// The mesh that will be used if this equipment is worn by a male body of the above race and class
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* MaleMesh = nullptr;
	
	// The mesh that will be used if this equipment is worn by a female body of the above race and class
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* FemaleMesh = nullptr;

	// Any body parts in this list will be made invisible if this equipment is worn
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FGameplayTag> HideBodyParts = {};
	
};


UCLASS(Blueprintable)
class TALESDUNGEONEER_API UEquipmentSystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static UDataTable* GetEquipmentWornDataTable();
	
	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static void GetAllHeadBodyTags(FGameplayTagContainer& TagContainer);

	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static void GetAllUpperBodyTags(FGameplayTagContainer& TagContainer);

	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static void GetAllLowerBodyTags(FGameplayTagContainer& TagContainer);

	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static TArray<FStMeshMergeData> GetAllDefaultMeshes(bool IsMale = true);

	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static FStMeshMergeData GetDefaultMeshFromTag(FGameplayTag SearchTag, bool IsMale = true);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static TArray<FStMeshMergeData> GetDefaultMeshMergeHeadBody(bool IsSquareFace = false);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static TArray<FStMeshMergeData> GetDefaultMeshMergeUpperBody(bool IsBusty = false);

	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static TArray<FStMeshMergeData> GetDefaultMeshMergeLowerBody(bool IsWideHip = false);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static FStEquipmentWorn GetEquipmentWornData(FName EquipmentName);

	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static bool GetEquipmentWornDataIsValid(const FStEquipmentWorn& EquipmentData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment System Globals")
	static bool GetEquipmentWornNameIsValid(FName EquipmentName);

};