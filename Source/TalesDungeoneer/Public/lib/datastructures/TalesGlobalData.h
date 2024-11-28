#pragma once

#include "CoreMinimal.h"
#include "lib/InventoryData.h"
#include "lib/enums/GlobalEnums.h"

#include "TalesGlobalData.generated.h"

class URsAbilityDataAsset;
struct FStMeshMergeData;

USTRUCT(BlueprintType)
struct FStFactionData
{
	GENERATED_BODY()

	FStFactionData();
	FStFactionData(const FStFactionData& InFactionData);
	explicit FStFactionData(const EFaction InFaction, const float InValue = 0.f);
	explicit FStFactionData(const EFaction InFaction, const EFactionState InState);

	void SetFactionValue(const float InValue);
	void SetFactionState(const EFactionState InState);

	float		  GetFactionValue() const;
	EFactionState GetFactionState() const;

	const float MinFactionValue = -1000.f;
	const float MaxFactionValue = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) EFaction FactionEnum = EFaction::ANY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float FactionValue = 0.f;

	FStFactionData& operator=(const FStFactionData& InFactionData);
	bool operator==(const FStFactionData& RHS) const { return FactionEnum == RHS.FactionEnum; }
	bool operator==(const EFaction RHS) const { return FactionEnum == RHS; }
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
class UTalesGlobalData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Project Version"), Category = "Global Tales Functions")
		static FString GetAppVersion(bool bShowBranch = false, bool bSafeString = false);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Full Project Version"), Category = "Global Tales Functions")
		static FTalesVersion GetFullAppVersion();

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Maximum Character Level"), Category = "Global Tales Functions")
		static int GetGameMaxCharacterLevel();

	UFUNCTION(BlueprintPure, Category = "Global Tales Functions")
	static FString CharacterSaveFolder() { return "Characters/"; }

	UFUNCTION(BlueprintPure, Category = "Global Tales Functions")
	static FString InventorySaveFolder() { return "Inventory/"; }

	UFUNCTION(BlueprintPure, Category = "Global Tales Functions")
	static FString MeshMergeSaveFolder() { return "MeshMerge/"; }

	UFUNCTION(BlueprintPure, Category = "Global Tales Functions")
	static FString GetStringFromAttribute(const FGameplayAttribute& GameplayAttribute);

	UFUNCTION(BlueprintPure, Category = "Global Tales Functions")
	static TArray<URsAbilityDataAsset*> GetAllGameplayAbilities(const FGameplayTag& RequiredClass, const FGameplayTag& OptionalSchool);


	UFUNCTION(BlueprintPure, Category = "Global Tales Functions")
		static bool GetGameIsInDebugMode()
	{
		return (UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT);
	};

};
