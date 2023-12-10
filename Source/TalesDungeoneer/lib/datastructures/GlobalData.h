#pragma once

#include "CoreMinimal.h"
#include "lib/VitalityData.h"
#include "lib/EquipmentData.h"
#include "lib/InventoryData.h"
#include "GameplayTags.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"

#include "GlobalData.generated.h"

struct FStMeshMergeData;

// Combines both the InventorySystem and VitalitySystem
USTRUCT(BlueprintType)
struct FStEquipmentItem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStEquipmentData EquipmentData = {};
	
	// Adds or removes core stat points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EVitalityStat, float> CoreStats = {};
	// Adds or removes damage bonus points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageBonuses = {};
	// Adds or removes damage resistance points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageResists = {};
};

USTRUCT(BlueprintType)
struct FStFactionDataMap
{
	GENERATED_BODY()
	FStFactionDataMap() {};
	FStFactionDataMap(EFaction NewEnum, float NewValue)
	{
		FactionEnum = NewEnum;
		FactionValue = NewValue;
	}
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EFaction FactionEnum = EFaction::ANY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FactionValue = 0.f;
};

USTRUCT(BlueprintType)
struct FStFactionData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStFactionDataMap> DataMap = {};
	float GetFactionValue(EFaction FactionToCheck) const
	{
		for (const FStFactionDataMap TempMap : DataMap)
		{
			if (TempMap.FactionEnum == FactionToCheck)
				return TempMap.FactionValue;
		}
		return 0.f;
	}
	EFactionState GetFactionState(EFaction FactionToCheck) const
	{
		const float FactionValue = GetFactionValue(FactionToCheck);
		if (FactionValue < -100.f) return EFactionState::HATE;
		if (FactionValue < 100.f)  return EFactionState::NONE;
		if (FactionValue  < 500.f) return EFactionState::LIKE;
		return EFactionState::ALLY;
	}
};

USTRUCT(BlueprintType)
struct FStBodyPartSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FGameplayTag BodyPartName = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText BodyPartDisplayName = FText();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* BodyPartIcon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* BodyPartMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int NumVariations = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int CurrentSelection = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FSlateColor BodyPartColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString OptionalDescription = "";
};

USTRUCT(BlueprintType)
struct FStCharacterRaces : public FTableRowBase
{
	GENERATED_BODY()
	
	// The enum display name is the GameSafeName / Proper Name (i.e. "Human")
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterRace RaceEnum = ECharacterRace::HUMAN;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName		= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* RaceIcon	= nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Description		= "None";
	
	// Starting Faction Considerations (<-100 = HATE, < 0 = NONE, < 100 = LIKE, >= 500 = ALLY)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStFactionData StartingRelations = {};
	// The body parts used by this model
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStBodyPartSettings> BodyParts = {};
	// Adds or removes core stat points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EVitalityStat, float> CoreStats = {};
	// Adds or removes damage bonus points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageBonuses = {};
	// Adds or removes damage resistance points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageResists = {};
};

USTRUCT(BlueprintType)
struct FStCharacterClasses : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterClass ClassEnum = ECharacterClass::WARRIOR;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName		= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* ClassIcon	= nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Description		= {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSinglePlayerOnly	= false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStFactionData StartingRelations = {};
	
	// Adds or removes core stat points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EVitalityStat, float> CoreStats = {};
	// Adds or removes damage bonus points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageBonuses = {};
	// Adds or removes damage resistance points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageResists = {};
};


USTRUCT(BlueprintType)
struct FStNpcData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterRace CharacterRace = ECharacterRace::HUMAN;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterClass CharacterClass	= ECharacterClass::WARRIOR;
	
	// The name of the NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString CharacterName = "";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeleton* MaleSkeleton = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UAnimInstance> MaleAnimationBp = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStMeshMergeData> MaleMeshes = {};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeleton* FemaleSkeleton = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UAnimInstance> FemaleAnimationBp = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStMeshMergeData> FemaleMeshes = {};

	// Factions that this NPC is a member of
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<EFaction> FactionMemberships = {};
	
	// Faction states for this NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStFactionData FactionData = FStFactionData();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStStartingItem> StartingItems;
	
	
	// Adds or removes core stat points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EVitalityStat, float> CoreStats = {};
	// Adds or removes damage bonus points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageBonuses = {};
	// Adds or removes damage resistance points. Access directly to set to a specific value.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<EDamageType, float> DamageResists = {};
	 
};

UCLASS()
class UGlobalData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAppVersion"), Category = "Game Config")
		static FString GetAppVersion();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Maximum Character Level"), Category = "Game Config")
		static int GetGameMaxCharacterLevel();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Debug Build"), Category = "Game Config")
		static bool GetGameIsInDebugMode()
	{
		return (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT);
	};

};
