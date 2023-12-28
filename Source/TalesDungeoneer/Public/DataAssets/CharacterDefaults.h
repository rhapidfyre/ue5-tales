// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "lib/Tags/TalesGlobalTags.h"
//#include "Characters/Components/MeshMergeComponent.h"

#include "CharacterDefaults.generated.h"

USTRUCT(BlueprintType)
struct FMeshOptionsData
{
	GENERATED_BODY();
	
	// If this mesh should be an option for masculine presentation
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsMasculine = true;
	
	// If this mesh should be an option for feminine presentation
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsFeminine = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* SkeletalMesh = nullptr;
};

USTRUCT(BlueprintType)
struct FSkeletonOptionsData
{
	GENERATED_BODY();

	FSkeletonOptionsData() {};
	~FSkeletonOptionsData() {};
	
	// If this these options should be available to masculine presentations
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsMasculine = false;
	
	// If this these options should be available to feminine presentations
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsFeminine = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeleton* Skeleton = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer BodyPartTags = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UAnimInstance> AnimInstance = nullptr;

	bool operator==(const FSkeletonOptionsData& Other) const
	{
		return (Skeleton == Other.Skeleton && AnimInstance == Other.AnimInstance);
	}

	friend uint32 GetTypeHash(const FSkeletonOptionsData& Other)
	{
		return HashCombine(GetTypeHash(Other.Skeleton), GetTypeHash(Other.AnimInstance));
	}
	
};

/** Individual data that each UDataAsset must contain
 * Useful for things like starting abilities, as each
 * race, class and character may have different starting abilities.
 */
UCLASS()
class TALESDUNGEONEER_API UCharacterCommonData : public UDataAsset
{
	GENERATED_BODY()
public:
	
	UCharacterCommonData() {};

	// The name of this character or entity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName = "";

	// Abilities specific to this race (such as racial powers)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf <class UTalesGameplayAbility> > Abilities = {};

	// Effects specific to this race (such as dark vision, slower speed, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf <class UGameplayEffect> > Effects = {};
};

UCLASS(BlueprintType)
class TALESDUNGEONEER_API UCharacterRaceData : public UCharacterCommonData
{
	GENERATED_BODY()
public:
	
	UCharacterRaceData() {};

	// The gameplay tag that denotes this class (equipment, abilities, etc)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag GameplayTag = TAG_Character_Class_Warrior;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DescriptionText = "Unknown";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* RaceIcon = nullptr;
	
	// When spawned, the character will pick randomly from one of these classes.
	// If no classes are selected, all classes will be eligible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer EligibleClasses = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkeletonOptionsData> SkeletonOptions = {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FSkeletonOptionsData> DefaultMeshes = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FLinearColor> SkinColorOptions = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshOptionsData> MeshOptionsForHead = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshOptionsData> MeshOptionsForHairstyle = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshOptionsData> MeshOptionsForFacialHair = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshOptionsData> MeshOptionsForUpperBody = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshOptionsData> MeshOptionsForLowerBody = {};
	
};

UCLASS(BlueprintType)
class TALESDUNGEONEER_API UCharacterClassData : public UCharacterCommonData
{
	GENERATED_BODY()
public:
	
	UCharacterClassData() {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag GameplayTag = TAG_Character_Class_Warrior;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DescriptionText = "Unknown";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ClassIcon = nullptr;
	
};

/**
 * This asset is what is loaded by the save-character method when a new character
 * is created, as well as what is loaded by NPCs first when spawning.
 */
UCLASS(BlueprintType)
class TALESDUNGEONEER_API UCharacterDataAsset : public UCharacterCommonData
{
	GENERATED_BODY()
	
public:

	UCharacterDataAsset() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UInventoryDataAsset* InventoryData = {};

	// When spawned, the character will pick randomly from one of these races.
	// If no races are selected, all races will be eligible.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer EligibleRaces = {};
	
};

UCLASS(BlueprintType)
class TALESDUNGEONEER_API UPrimaryCharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPrimaryCharacterData();

	// The name of this character or entity
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString CharacterName = "";
	
	UFUNCTION(BlueprintPure) FLinearColor GetCharacterSkinColor() const;
	
	UFUNCTION(BlueprintPure, Meta = (Keywords="AnimInstance, Animation, Blueprint"))
	FSkeletonOptionsData GetCharacterSkeleton(
		const bool bForceMasculine = false,
		const bool bForceFeminine = false) const;

	UFUNCTION(BlueprintPure) FGameplayTag 	GetCharacterClass() const;
	UFUNCTION(BlueprintPure) FGameplayTag 	GetCharacterRace() const;
	
	UFUNCTION(BlueprintPure) TArray<TSubclassOf <class UTalesGameplayAbility> > GetAbilities() const;
	UFUNCTION(BlueprintPure) TArray<TSubclassOf <class UGameplayEffect> > GetEffects() const;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UCharacterDataAsset* CharacterDataAsset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UCharacterRaceData*  CharacterRaceData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UCharacterClassData* CharacterClassData;
	 
};