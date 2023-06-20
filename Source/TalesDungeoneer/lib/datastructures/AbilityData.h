
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TalesDungeoneer/lib/enums/AbilityEnums.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Actor.h"
#include "EnhancedInput/Public/InputAction.h"
#include "NiagaraSystem.h"
#include "TalesDungeoneer/lib/enums/GlobalEnums.h"

#include "AbilityData.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectActivated, FName, AbilityName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectExpired, FName, AbilityName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectTick, int, TimeRemaining);

// Fwd Declaration
class AAbilityEffectBase;
class AProjectileBase;
class ASpellActorBase;

// Data relating to hotkeys that activate abilities (such as keys 1-6)
USTRUCT(BlueprintType)
struct FStAbilityHotkey
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UInputAction* AbilityInput = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName	      AbilityName  = FName();
};

USTRUCT(BlueprintType)
struct FStAbilityVfx
{
	GENERATED_BODY()
	// Overrides Cascade Effect if used
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UNiagaraSystem* NiagaraEffect = nullptr;
	// Used if Niagara Effect is invalid
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UParticleSystem* CascadeEffect = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectScale		= 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector EffectOffset	= FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator EffectRotation = FRotator(0.f);
};

USTRUCT(BlueprintType)
struct FStAbilitySoundData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundCasting = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundLooping = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundSuccess = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundFailure = nullptr;
};

USTRUCT(BlueprintType)
struct FStProjectileData
{
	GENERATED_BODY()

	// The speed of the projectile. Leave as 0,0,0 if this isn't a projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector SpeedDirection = FVector(0.f);
	
	// If valid, when the projectile impacts something, this actor will spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<ASpellActorBase> ImpactActor = nullptr;
	
	// The niagara system to play upon impact. Overrides Cascade Effect.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UNiagaraSystem* NiagaraEffect = nullptr;
	
	// Used if NiagaraEffect is not given.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UParticleSystem* CascadeEffect = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectScale = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector EffectOffset = FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator EffectRotation = FRotator(0.f);

	// The sound to loop while the projectile is moving
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundEffect = nullptr;
	
};

USTRUCT(BlueprintType)
struct FStSpellData : public FTableRowBase
{
	GENERATED_BODY()

	// If true, the damage will repeat every tick during the duration of the effect
	// If false, the damage is done once upon application
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsOverTime = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HitpointsAffected = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MagicPointsAffected = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float StaminaAffected = 0.f;

	// The maximum distance, in feet, this spell can go from the origin
	// Ignored for spells that originate from the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxReach = 304.8f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsProjectile = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStProjectileData ImpactData = FStProjectileData();
	
	// If true, projectile will be affected by gravity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseGravity = false;
	
	// If valid, when the ability successfully activates, this actor will be spawned
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AProjectileBase> ProjectileActor = nullptr;
	
	// Spawn Offset from Spawn Location (Parent origin)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SpawnBone = FName();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector SpawnOffset = FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator SpawnRotation = FRotator(0.f);

	// The sound data for when the spell is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStAbilitySoundData SoundData = FStAbilitySoundData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStAbilityVfx VisualEffects = FStAbilityVfx();

	// Classes that can use this ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<ECharacterClass> AllowedClass = {};

	// Minimum level to use this ability
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int   MinimumLevel = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeMagic	 = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeHealth  = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeStamina = 0.f;

	 
};

// Used for looking up ability data. Should not be retained in memory,
// but rather requested as needed.
USTRUCT(BlueprintType)
struct FStAbilityData : public FTableRowBase
{
	GENERATED_BODY()

	// The name to be displayed in the UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName = FString();

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

	// The maximum distance, in feet, this ability can reach
	// For AOEs, this is the max range the origin can be thrown
	// Negative or zero means it goes off centered on the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxReach = 304.8f;

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

	// The sound data for when the ability is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStAbilitySoundData SoundData = FStAbilitySoundData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStAbilityVfx VisualEffects = FStAbilityVfx();
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
	static UDataTable* GetSpellDataTable();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static FStAbilityData GetAbilityDataFromName(FName AbilityName);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static FStSpellData GetSpellDataFromName(FName SpellName);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static bool GetAbilityNameIsValid(FName AbilityName);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static bool GetSpellNameIsValid(FName SpellName);
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Ability Data Table"),
										 Category = "Ability Data")
	static bool GetAbilityDataIsValid(FStAbilityData AbilityData = FStAbilityData());
    
};

