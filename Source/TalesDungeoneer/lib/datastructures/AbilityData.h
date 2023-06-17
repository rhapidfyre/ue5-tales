
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TalesDungeoneer/lib/enums/AbilityEnums.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Actor.h"
#include "EnhancedInput/Public/InputAction.h"
#include "NiagaraSystem.h"

#include "AbilityData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEffectExpired);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectTick, int, TimeRemaining);

// Fwd Declaration
class AAbilityEffectBase;

// Data relating to hotkeys that activate abilities (such as keys 1-6)
USTRUCT(BlueprintType)
struct FStAbilityHotkey
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UInputAction* AbilityInput = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName	      AbilityName  = FName();
};

// Used for looking up ability data. Should not be retained in memory,
// but rather requested as needed.
USTRUCT(BlueprintType)
struct FStAbilityData : public FTableRowBase
{
	GENERATED_BODY()

	// The name to be displayed in the UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName = "";

	// The icon that will show in the ability tree and hot bar
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* DisplayIcon   = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EAbilityType AbilityType  = EAbilityType::NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EAbilityTarget TargetType = EAbilityTarget::SELF;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bCanActivateWhileMoving  = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSprintCancelsActivation = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bJumpCancelsActivation   = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSwimCancelsActivation   = true;

	// The maximum amount of stacks this effect allows before maxing out
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int NumOfStackedEffects = 10;

	// If true, each stack of this effect will multiply the effect
	// If false, each stack just increases the time the effect lasts
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEffectsStack = true;

	// If true, all stacks tick their timers concurrently
	// If false, each stack reduction resets the timer for the next reduction
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bConcurrentStacks = true;
	
	// The amount of time it takes for this ability to activate successfully
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ActivationTime = 3.f;
	
	// The length, in seconds, that the effect from this ability will last.
	// A duration of <= 0 will result in a single tick of the ability before wearing off.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectDuration = 6.f;

	// If valid, when this ability is activated, fires this Niagara Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UNiagaraSystem* NiagaraEffect = nullptr;
	
	// The bone for playing the effect. If invalid, attaches at the actors root
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName NiagaraBone = "root";
	
	// The relative offset from the attachment point at (0,0,0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector NiagaraOffset = FVector(0.f);
	
	// The relative rotational offset from the attachment point at (0,0,0)
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


// Used for tracking active effects. Contains the minimal amount of memory
// necessary for effectively tracking active effects.
USTRUCT(BlueprintType)
struct FStAbilityEffect
{
	GENERATED_BODY()

	FStAbilityEffect() {};
	FStAbilityEffect(FStAbilityData AbilityData)
	{
		AbilityName			= AbilityName;
		EffectInstigator	= EffectInstigator;
		bTicksConcurrently	= AbilityData.bConcurrentStacks;
		TimeRemaining		= AbilityData.EffectDuration;
	}

	// The name of the ability that initiated this effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AbilityName = FName();
	
	// The time remaining on this effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimeRemaining = 0.f;
	
	// If true, this effects timer will reduce even if it isnt the top most entry in the array
	// If false, this effect will not have its time reduced until it is at the top of the array
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTicksConcurrently = true;

	// The actor that instigated this effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite) AActor* EffectInstigator = nullptr;
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

