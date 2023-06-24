
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

// Called when an ability has started casting
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCastStarted,
	FName, AbilityName, float, CastTime);

// Called when an ability has finished casting
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCastComplete,
	FName, AbilityName, bool, WasSuccessful);

// Fwd Declaration
class AAbilityEffectBase;
class AProjectileBase;
class ASpellActorBase;
class ACharacterBase;

// Data relating to hotkeys that activate abilities (such as keys 1-6)
USTRUCT(BlueprintType)
struct FStAbilityHotkey
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UInputAction* AbilityInput = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName	      AbilityName  = FName();
};

USTRUCT(BlueprintType)
struct FStAbilityActorSpawner
{
	GENERATED_BODY()
	// The actor to spawn when this ability has completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AActor> SpawnedActor = nullptr;
	// When this spawns, should it be owned by the actor who activated this ability?
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bOwnedOnSpawn				= false;
	// Should this actor be attached to the owner who activated it?
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAttachOnSpawn				= true;
	// The bone that the spawned actor should attach to
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName AttachBone				= FName("root");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector AttachOffset			= FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator AttachRotation 		= FRotator(0.f);
};

USTRUCT(BlueprintType)
struct FStAbilityFx
{
	GENERATED_BODY()

	// The effect will loop for this amount of time. Does not loop for values <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float NiagaraLoopTime			= 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelaySound				= 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelayEffect				= 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectScale				= 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SoundVolume				= 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAttachNiagaraToActor   	= true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAttachNiagaraToSkeleton   = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAttachSound				= true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName NiagaraBone				= FName("root");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SoundBone					= FName("root");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundEffect			= nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UNiagaraSystem* NiagaraEffect	= nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector EffectOffset			= FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator EffectRotation 		= FRotator(0.f);
};

USTRUCT(BlueprintType)
struct FStAbilitySoundData
{
	GENERATED_BODY()
	// The effect will loop for this amount of time. Does not loop for values <= 0
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SoundLoopTime = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelaySound = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundReference = nullptr;
};

USTRUCT(BlueprintType)
struct FStCastingSoundData : public FStAbilitySoundData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelaySoundSuccess = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundSuccess = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelaySoundFailure = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* SoundFailure = nullptr;
};

USTRUCT(BlueprintType)
struct FStAbilityAnimData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bHoldStartAnim  = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelayStartAnim = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* AnimationOnStart		= nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelayFailAnim = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* AnimationOnFail		= nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelaySuccessAnim = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAnimMontage* AnimationOnSuccess	= nullptr;
};

USTRUCT(BlueprintType)
struct FStProjectileData
{
	GENERATED_BODY()

	// The speed of the projectile
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ProjectileSpeed = 0.f;
	
	// If true, projectile will be affected by gravity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseGravity = false;
	
	// The projectile actor to use. If invalid, the projectile is invisible and has no sfx or vfx
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AProjectileBase> Projectile = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector SpawnOffset = FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator SpawnRotation = FRotator(0.f);
	
	// If valid, when the projectile begins play, these actors will spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityActorSpawner> SpawnsOnFire = {};
	
	// If valid, when the projectile impacts something, these actors will spawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityActorSpawner> SpawnsOnImpact = {};
	
	// The effects that play when the projectile comes to life
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectsOnSpawn = {};
	
	// The effects to play when the projectile is active/idle
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectsLooped = {};
	
	// The effects to play when the projectile is destroyed
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectsFinal = {};
	
};

USTRUCT(BlueprintType)
struct FStSpellData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName = FString();

	// If true, the ability effects will spawn attached to the actor who cast this ability
	// If false, the ability effects will spawn at the actor's root and remain there.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAttachToCaster = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SpawnBone = FName();
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector SpawnOffset = FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator SpawnRotation = FRotator(0.f);
	
	// If true, the affects (hp, magic, stamina, etc) will be applied each tick
	// If false, the affects are only applied once and then finished
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsOverTime = false;
	
	// The number of hit points to deduct when the ability triggers
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HitpointsAffected = 0.f;
	
	// The number of magic points to deduct when the ability triggers
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MagicPointsAffected = 0.f;
	
	// The number of stamina points to deduct when the ability triggers
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float StaminaAffected = 0.f;

	// The maximum distance from the impact point this spell can affect
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxRadius = 304.8f;

	// Data for when the project hits something. No entries means this is NOT a projectile.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStProjectileData> ImpactData = {};

	// The sound this spell makes while it's active (such as a projectile traveling)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStAbilitySoundData SoundData = FStAbilitySoundData();

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeMagic	 = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeHealth  = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeStamina = 0.f;

	// Classes that can use this spell
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<ECharacterClass, int> AllowedClass = {};
	
	// The effects that play when the spell comes to life
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectsOnSpawn = {};

	// The effects that play while the spell is active/ticking
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectsLooped = {};

	// The effects that play when the spell completes
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectsFinal = {};

	 
};

// Used for looking up ability data. Should not be retained in memory,
// but rather requested as needed.
USTRUCT(BlueprintType)
struct FStAbilityData : public FTableRowBase
{
	GENERATED_BODY()

	// The name to be displayed in the UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName = FString();

	// If true, the ability actor will spawn attached to the actor who cast this ability
	// If false, the ability actor will spawn at the actor's root and remain there.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bAttachToCaster = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName SpawnBone = FName("root");
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector SpawnOffset = FVector(0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator SpawnRotation = FRotator(0.f);
	
	// If valid, when the ability successfully activates, these actors will be spawned
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AAbilityEffectBase> AbilityBase = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeMagic	 = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeHealth  = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ConsumeStamina = 0.f;

	// The icon that will show in the ability tree and hot bar
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UTexture2D* DisplayIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSprintCancels = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bJumpCancels   = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bSwimCancels   = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bMoveCancels   = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EAbilityType AbilityType  		= EAbilityType::NONE;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EAbilityTarget TargetType 		= EAbilityTarget::SELF;
	
	// If true, each stack of this effect will multiply the effect
	// If false, the effect will be the same regardless of number of stacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bEffectsStack = true;

	// If true, all stacks tick their timers concurrently, independent of one another
	// If false, only the top-most effect in the stack will reduce their timer
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bTickIndependently = true;
	
	// The maximum amount of stacks this effect allows before maxing out
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int NumOfStackedEffects = 1;

	// The maximum distance from the casting actor that the effect can be applied
	// For non-projectiles, this is the max distance from the casting actor
	// For projectiles, this is how far the projectile can go without hitting anything
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxRange = 2048.f;
	
	// The amount of time it takes for this ability to activate successfully
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float ActivationTime = 3.f;
	
	// The length, in seconds, that the effect from this ability will last.
	// A duration of <= 0 will apply the effects once without ticking
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float EffectDuration = 0.f;
	
	// The minimum length of time until this ability can be re-activated
	// This timer starts as soon as the ability succeeds
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float CooldownSeconds = 4.f;

	// The animation data for the ability procedure
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStAbilityAnimData AnimationData	= FStAbilityAnimData();
	
	// The sound data for when the ability is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStCastingSoundData SoundData		= FStCastingSoundData();

	// The visual effects played when the ability is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectCasting		= {};
	
	// The visual effects played when the ability is activated, and looped until completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectLooped		= {};
	
	// The visual effect played when the ability has completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectComplete		= {};
	
	// The visual effect played when the ability has completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStAbilityFx> EffectFailed		= {};
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
