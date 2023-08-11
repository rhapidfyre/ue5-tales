#pragma once

#include "CoreMinimal.h"
#include "lib/VitalityData.h"
#include "lib/EquipmentData.h"
#include "GameplayTags.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"

#include "GlobalData.generated.h"

// Combines both the InventorySystem and VitalitySystem
USTRUCT(BlueprintType)
struct FStEquipmentItem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStCharacterStats StatBonuses = FStCharacterStats();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStEquipmentData EquipmentData = FStEquipmentData();
};

USTRUCT(BlueprintType)
struct FStCharacterRaces : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterRace RaceEnum = ECharacterRace::HUMAN;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* RaceIcon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Description = "None";
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSinglePlayerOnly = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStCharacterStats StatModifiers = FStCharacterStats();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStDamageIntMap> DamageBonuses;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStDamageIntMap> NaturalResistances;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName GameSafeName = {};
};

USTRUCT(BlueprintType)
struct FStCharacterClasses : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterClass RaceEnum = ECharacterClass::WARRIOR;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* ClassIcon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString Description = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSinglePlayerOnly = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStCharacterStats StatModifiers = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStDamageIntMap> DamageBonuses = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStDamageIntMap> NaturalResistances = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName GameSafeName = {};
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
struct FStCharacterParts : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ECharacterRace EligibleRace = ECharacterRace::HUMAN;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStBodyPartSettings> BodyParts;
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

};
