#pragma once

#include "CoreMinimal.h"
#include "lib/EquipmentData.h"
#include "lib/InventoryData.h"
#include "GameplayTags.h"
#include "lib/enums/GlobalEnums.h"

#include "GlobalData.generated.h"

struct FStMeshMergeData;

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
struct FTalesVersion
{
	GENERATED_BODY()
	FTalesVersion() : VersionMajor(0), VersionMinor(0), VersionPatch(0), VersionBranch("x") {};
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) int VersionMajor;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) int VersionMinor;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) int VersionPatch;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) FString VersionBranch;
	FString ToString(const bool bShowBranch = false, const bool bSafeString = false) const;
};

UCLASS()
class UGlobalData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Project Version"), Category = "Game Config")
		static FString GetAppVersion(bool bShowBranch = false, bool bSafeString = false);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Get Full Project Version"), Category = "Game Config")
		static FTalesVersion GetFullAppVersion();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Maximum Character Level"), Category = "Game Config")
		static int GetGameMaxCharacterLevel();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAppVersion"), Category = "Game Config")
	static FString CharacterSaveFolder() { return "Characters/"; }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAppVersion"), Category = "Game Config")
	static FString InventorySaveFolder() { return "Inventory/"; }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAppVersion"), Category = "Game Config")
	static FString MeshMergeSaveFolder() { return "MeshMerge/"; }
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Debug Build"), Category = "Game Config")
		static bool GetGameIsInDebugMode()
	{
		return (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT);
	};

};
