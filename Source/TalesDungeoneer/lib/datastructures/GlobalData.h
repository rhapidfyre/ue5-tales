#pragma once

#include "CoreMinimal.h"
#include "lib/VitalityData.h"
#include "lib/EquipmentData.h"

#include "GlobalData.generated.h"

// Combines both the InventorySystem and VitalitySystem
USTRUCT(BlueprintType)
struct FStEquipmentItem
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStCharacterStats StatBonuses = FStCharacterStats();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStEquipmentData EquipmentData = FStEquipmentData();
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
