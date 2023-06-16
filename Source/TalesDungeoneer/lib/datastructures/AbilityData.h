
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TalesDungeoneer/lib/enums/AbilityEnums.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Actor.h"
#include "TalesDungeoneer/Entities/AbilityEffectBase.h"

#include "AbilityData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectExpired);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectTick, int, TimeRemaining);


USTRUCT(BlueprintType)
struct FStAbilityData : public FTableRowBase
{
	GENERATED_BODY()

	// The name to be displayed in the UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName = "";

	// The icon that will show in the ability tree and hot bar
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* DisplayIcon = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EAbilityType AbilityType = EAbilityType::NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EAbilityTarget TargetType = EAbilityTarget::SELF;
	
	// The length, in seconds, that the effect from this ability will last.
	// A duration of <= 0 will result in a single tick of the ability before wearing off.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectDuration = 6.f;

	// If valid, plays the given NiagaraEffect at the given bone and offsets
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UNiagaraSystem* NiagaraEffect = nullptr;
	// If valid, the Niagara System will attach to this bone. If not valid, plays at mesh root.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName NiagaraBone = "root";
	// The offset from the attachment point at (0,0,0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector NiagaraOffset = FVector(0.f);
	// The rotational offset from the attachment point at (0,0,0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector NiagaraRotOffset = FVector(0.f);

	// Spawns an actor of this type for more effects. Ignored if unset.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AAbilityEffectBase> SpawnActor = nullptr;

	/**
	 * @brief Only works if SpawnActor is set. If other than None, the actor will
	 * attach to this bone with the given offsets. If None, it is ignored.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AttachBoneOnSpawn = FName();
	// The offset from the actors attachment position at (0,0,0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector AttachOffset	= FVector(0.f);
	// The rotational offset from the actors attachment position at (0,0,0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector AttachRotOffset = FVector(0.f);
};

UCLASS()
class UAbilitySystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static UDataTable* GetAbilityDataTable();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static FStAbilityData GetAbilityDataFromName(FName AbilityName);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static bool GetAbilityNameIsValid(FName AbilityName);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static bool GetAbilityDataIsValid(FStAbilityData AbilityData = FStAbilityData());
    
};

