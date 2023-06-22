
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../lib/datastructures/AbilityData.h"
#include "../Characters/CharacterBase.h"

#include "AbilityEffectBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityFinished, FName, AbilityName, bool, WasSuccessful);


//
// This is the main parent for all abilities in the game. This is spawned
// whenever an ability is activated, and performs all of the initialization,
// timer, effects and logic of the ability. Once it performs its logic,
// it self-destructs.
//
// The ability is defined within	./TalesContent/DataTables/DT_AbilityData
//
UCLASS(Blueprintable, BlueprintType)
class AAbilityEffectBase : public AActor
{
	GENERATED_BODY()
public:

	AAbilityEffectBase();
	AAbilityEffectBase(ACharacterBase* Instigator, const FName AbilityName, FVector ImpactLocation);
	AAbilityEffectBase(ACharacterBase* Instigator, const FName AbilityName, AActor* TargetActor);

	UPROPERTY(BlueprintAssignable) FOnAbilityFinished OnAbilityFinished;
	
	virtual void SetOwner(AActor* NewOwner) override;
	
	UFUNCTION(BlueprintPure) FStAbilityData GetAbilityData() const
	{
		if (!UAbilitySystem::GetAbilityDataIsValid(_AbilityData))
			return UAbilitySystem::GetAbilityDataFromName(_AbilityName);
		return _AbilityData;
	}

	
	UFUNCTION(BlueprintPure) FStSpellData GetSpellData() const { return _SpellData; }

	// Event triggers when the timer reached zero and the effect wears off
	// Doesn't trigger on instant abilities with no timer or invalid object creation.
	UPROPERTY(BlueprintCallable) FOnEffectExpired OnEffectExpired;

	// Event triggers when the ability has activated and the timer has started
	// Doesn't trigger if the AAbilityEffectBase is invalid or set up improperly.
	UPROPERTY(BlueprintCallable) FOnEffectActivated OnEffectActivated;

	// Event triggers when the timer ticks, including right before expiring.
	UPROPERTY(BlueprintCallable) FOnEffectTick OnEffectTick;
	
	/**
	 * @brief Returns the name of the ability this object is representing
	 * @return The FName (data table row) of this ability
	 */
	UFUNCTION(BlueprintPure) FName GetAbilityName() const { return _AbilityName; };

	/**
	 * @brief Server Only\n Manually sets the instigator of this ability.
	 *						Does nothing if the actor has finished initialization.
	 * @param AbilityInstigator The character who should be the instigator
	 */
	UFUNCTION(BlueprintCallable) void SetAbilityInstigator(ACharacterBase* AbilityInstigator);
	
	/**
	 * @brief Returns the instigator of the ability
	 * @return The ACharacterBase of the actor who initiated this ability
	 */
	UFUNCTION(BlueprintPure) ACharacterBase* GetAbilityInstigator() const { return _Instigator; };
	
	/**
	 * @brief Server Only\n Manually sets the target actor of this ability.
	 *						This ensures the ability will always affect the target.
	 *						If there is no target, SetImpactLocation must be set.
	 * @param TargetActor The character being targeted by this ability (can be self)
	 */
	UFUNCTION(BlueprintCallable) void SetTargetActor(ACharacterBase* TargetActor);

	/**
	 * @brief Server Only\n Manually sets the impact location of this ability.
	 *						Does nothing if Target Actor is set.
	 * @param ImpactLocation The target impact location.
	 */
	UFUNCTION(BlueprintCallable) void SetImpactLocation(FVector ImpactLocation);

	/**
	 * @brief Server Only\n Manually sets the impact rotation of this ability.
	 *						Does nothing if Target Actor is set.
	 * @param ImpactRotation The target impact rotation.
	 */
	UFUNCTION(BlueprintCallable) void SetImpactRotation(FRotator ImpactRotation);

	/**
	 * @brief Server Only\n Manually sets the FName of the ability to inherit.
	 * @param AbilityName The name of the ability to use
	 */
	UFUNCTION(BlueprintCallable) void SetAbilityName(FName AbilityName);

	
	/**
	 * @brief Returns a pointer to the actor this effect originated from
	 * @return The ACharacterBase that instigated this effect
	 */
	UFUNCTION(BlueprintPure) ACharacterBase* GetOriginatingActor() const { return _Instigator; }

	/**
	 * @brief Gets the actor targeted by this ability
	 * @return The actor targeted by this ability, or nullptr
	 */
	UFUNCTION(BlueprintPure) AActor* GetTargetActor() const { return _TargetActor; }

	/**
	 * @brief Gets the vector targeted by this ability
	 * @return The vector targeted by this ability, or zero-vector
	 */
	UFUNCTION(BlueprintPure) FVector GetImpactLocation() const { return _ImpactLocation; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	USceneComponent* SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimerTickRate = 0.05;

protected:
	
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * @brief Called when the ability casting timer completes (ticks < 0)
	 * @param WasSuccessful True if the ability activated successfully. False if it failed.
	 */
	virtual void AbilityComplete(bool WasSuccessful = false);
	
private:

	// Allows tracking of effects/sounds for cleanup if this actor gets destroyed
	TArray<UNiagaraComponent*> LoopingNiagaraEmitters;
	TArray<UAudioComponent*> LoopingSoundEmitters;

	//Helper Functions
	void ExecuteCastingEffects(FStAbilityFx AbilityFx, bool StopOnDestroyed = false);
	UFUNCTION() void PlayDelayedAnimation(ACharacter* PlayTarget, UAnimMontage* AnimMontage);
	UFUNCTION() void PlayDelayedSound(FVector SoundLocation, USoundBase* SoundBase,
		float LoopTime = 0.f, bool StopOnDestroy = false);
	UFUNCTION() void PlayDelayedNiagara(FStAbilityFx VisualEffect);
	
	void InitializeAbility();

	void SetupDefaults();

	// Called everytime the timer ticks
	virtual void EffectTick();
	
	// The data regarding the ability that is in this effect
	UPROPERTY() FStAbilityData	_AbilityData;
	UPROPERTY(Replicated) FName _AbilityName;

	UPROPERTY(Replicated) bool _WasSuccessful = false;
	
	// The data of the spell being used, if any
	UPROPERTY() FStSpellData	_SpellData;

	// The actor of origination for the ability that is in effect
	UPROPERTY(Replicated) ACharacterBase* _Instigator = nullptr;

	// The actor targeted by this ability. If nullptr, it uses _ImpactLocation
	UPROPERTY() AActor* _TargetActor = nullptr;

	// If no target, this is the location the ability will target or impact
	UPROPERTY() FVector _ImpactLocation = FVector(0.f);
	
	// If no target actor, this is the rotation from (0,0,0)
	UPROPERTY() FRotator _ImpactRotation = FRotator(0.f);
	
	// The location of the ability origination
	UPROPERTY() FVector _OriginLocation = FVector(0.f); 
	
	// The rotiation of the ability origination
	UPROPERTY() FVector _OriginRotation = FVector(0.f); 

	// The timer that handles expiration and tick. Destroys object on expiration.
	UPROPERTY() FTimerHandle _EffectTimer = FTimerHandle();

	// Time remaining until the ability expires
	UPROPERTY() float _TimeRemaining = 0;

	// Once the actor has been initialized, the settings cannot be modified
	bool bInitialized = false;

	bool bShowDebug = false;
	
};