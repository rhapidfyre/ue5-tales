// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayTagContainer.h"
#include "Characters/Components/MeshMergeComponent.h"
#include "Engine/DataAsset.h"
#include "lib/Tags/TalesGlobalTags.h"

#include "CharacterDefaults.generated.h"


UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_Sex_Female);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_Sex_Male);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Character_Sex_Nonbinary);


struct FGameplayAttributeData;


USTRUCT(BlueprintType)
struct FSkeletonOptionsData
{
	GENERATED_BODY();

	FSkeletonOptionsData() {};
	~FSkeletonOptionsData() {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTagContainer OptionTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeleton* Skeleton = nullptr;
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayAttribute, int> CoreStatsModifiers = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayAttribute, int> DamageResistModifiers = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayAttribute, int> DamageBonusModifiers = {};

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

	// The default, no-equipment mesh options for this race
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FBodyPartData> DefaultMeshes = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FLinearColor> SkinColorOptions = {FLinearColor(255, 206, 180)};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshMergeMappings> MeshOptionsForHairstyle	= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshMergeMappings> MeshOptionsForHead		= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshMergeMappings> MeshOptionsForEyebrows	= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshMergeMappings> MeshOptionsForEyes		= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshMergeMappings> MeshOptionsForFacialHair = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshMergeMappings> MeshOptionsForTorso		= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	TArray<FMeshMergeMappings> MeshOptionsForLegs		= {};
	
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