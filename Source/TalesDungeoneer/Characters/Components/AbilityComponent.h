// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
//#include "TalesDungeoneer/lib/datastructures/AbilityData.h"
#include "TalesDungeoneer/lib/objects/StatusEffect.h"
#include "Delegates/Delegate.h"

#include "AbilityComponent.generated.h"


// Called when this effect has started play
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityAdded,
FName, AbilityName, int, NumStacksActive);

// Called when this effect wears off
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityRemoved,
FName, AbilityName, int, NumStacksActive);

// Called when this effect wears off
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActiveEffectsUpdated);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityFailed,
FName, AbilityName, FString, FailureMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCanceled,
	FName, AbilityName, FString, CancelReason);



UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAbilityComponent();

	UPROPERTY(BlueprintAssignable) FOnAbilityCastStarted	OnAbilityCastStarted;
	UPROPERTY(BlueprintAssignable) FOnAbilityCastComplete	OnAbilityCastComplete;
	UPROPERTY(BlueprintAssignable) FOnAbilityAdded			OnAbilityAdded;
	UPROPERTY(BlueprintAssignable) FOnAbilityRemoved		OnAbilityRemoved;
	UPROPERTY(BlueprintAssignable) FOnActiveEffectsUpdated  OnActiveEffectsUpdated;

	// Called whenever casting/activation is denied
	UPROPERTY(BlueprintAssignable) FOnAbilityFailed			OnAbilityFailed;

	// Called when the currently casting ability is canceled
	UPROPERTY(BlueprintAssignable) FOnAbilityCanceled		OnAbilityCanceled;


	// Blueprint overridable version of AbilityAction (C++)
	UFUNCTION(BlueprintNativeEvent)
	void EventOnAbilityAction(UInputAction* AbilitySlot);

	/**
	 * @brief Activates a set ability, mapped to a specific AbilityName
	 *			Can be called on the client or server.
	 *			Useful for making dynamic hotkeys.
	 * @param AbilitySlot The ability slot to activate
	 */
	virtual void AbilityAction(UInputAction* AbilitySlot);

	/**
	 * @brief Updates the input actions to trigger a specific ability
	 *		  Setting AbilityName to 'None' will un-assign the input.
	 * @param AbilityName The ability name to map. 'None' to un-assign.
	 * @param InputAction The input action that triggers the ability to fire
	 * @return True if successful, false if the input action failed to map to the ability
	 */
	UFUNCTION(BlueprintCallable)
	bool SetAbilityInputAction(FName AbilityName, UInputAction* InputAction);
	
	/**
	 * @brief Called to activate the requested ability directly.
	 * @param AbilityName The FName of the ability to request
	 * @param TargetActor An optional actor as the target of the ability
	 * @param ForwardVector The direction to fire the spell
	 */
	UFUNCTION(BlueprintCallable)
	void ActivateAbility(const FName AbilityName,
		AActor* TargetActor = nullptr, FVector ForwardVector = FVector(0.f));

	/**
	 * @brief Applies an effect to the component, so other components (UI) can track it.
	 * @param EffectInstigator The actor who instigated the effect
	 * @param AbilityName The Effect to apply
	 */
	UFUNCTION(BlueprintCallable)
	void ApplyEffect(ACharacterBase* EffectInstigator, FName AbilityName);

	// The tick rate of the Effects Timer
	// The timer does not run if there are no active effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimerRate = 0.1;

	UFUNCTION(BlueprintPure) AActor* GetTargetedActor() const { return _TargetActor; }

	// Removes the expired effect from the TArray
	UFUNCTION(BlueprintCallable)
	void RemoveExpiredEffect(UStatusEffect* AbilityEffect, FName AbilityName);

	UFUNCTION(BlueprintPure)
	TArray<UStatusEffect*> GetActiveEffects() { return _ActiveEffects; }

	UFUNCTION(BlueprintCallable) int GetNumStacksActive(FName AbilityName);

	UFUNCTION(BlueprintCallable) bool GetIsEffectActiveByName(FName AbilityName);

	// Returns the ability effect that has the least amount of time left
	UFUNCTION(BlueprintCallable) UStatusEffect* GetEffectWithLowestTimer(FName AbilityName = "None");

	// Returns the ability effect that has the greatest amount of time left
	UFUNCTION(BlueprintCallable) UStatusEffect* GetEffectWithGreatestTimer(FName AbilityName = "None");
	
	// Returns the total time of all effects in the stack
	UFUNCTION(BlueprintCallable) float GetTotalEffectStackTimer(FName AbilityName = "None");

	/**
	 * @brief Call to stop the casting of any ability.
	 * @param OnlyFocused If true, interrupt ONLY focused abilities. False interrupts everything.
	 */
	UFUNCTION(BlueprintCallable) void InterruptCasting(bool OnlyFocused = false);
	
protected:
	
	virtual void BeginPlay() override;

	virtual void OnUnregister() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnComponentCreated() override;

	virtual void SpawnEffectsActor(
		ACharacterBase* EffectInstigator, FName AbilityName, FVector ForwardVector = FVector(0.f));

	/**
	 * @brief Sends the ability activation request to the server in case
	 * ActivateAbility() was called on the client.
	 * @param AbilityName The ability FName to request
	 * @param TargetActor An optional actor as the target of the ability
	 * @param ForwardVector The direction the spell is going if aimed/fired
	 */
	UFUNCTION(Server, Reliable)
	void Server_RequestAbility(FName AbilityName,
		AActor* TargetActor = nullptr, FVector ForwardVector = FVector(0.f));

	UFUNCTION(Server, Reliable)
	void Server_InterruptCasting(bool OnlyFocused = false);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<UInputAction*, FName> _AbilityMappings;

	// Overrideable event that fires everytime the Effects Timer ticks
	UFUNCTION(BlueprintNativeEvent) void OnTickTimer();
	
	UFUNCTION(BlueprintPure) bool GetIsCasting() const { return bIsCasting; }
	UFUNCTION(BlueprintCallable) void SetIsCasting(FName SpellName);

	// Used by the timer delegate to cancel casting
	UFUNCTION()
	void SetNoLongerCasting(FName AbilityName, bool WasSuccessful = false);
	
private:

	UFUNCTION(Client, Reliable)
	void Client_AbilityFailure(FName AbilityName, const FString& FailureReason);


	UFUNCTION(Client, Reliable)
	void Client_AbilityCanceled(FName AbilityName, const FString& FailureReason);

	UFUNCTION()	void DestroyAllEffects();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StopCasting(FName AbilityName, bool WasSuccessful);

	// True if the player is casting any abilities
	UPROPERTY(Replicated) bool bIsCasting = false;

	// If true, the player cannot use any other focused abilities
	UPROPERTY(Replicated) bool bIsFocused = false; 

	UPROPERTY() AActor* _TargetActor;

	// Fires every time the Effects Timer ticks
	virtual void TickTimer();

	// Active abilities
	//TMap< FName, TArray<FStAbilityEffect> > _ActiveEffects;
	UPROPERTY(ReplicatedUsing=OnRep_ActiveEffectsUpdated)
	TArray<UStatusEffect*> _ActiveEffects;
	UFUNCTION(Client, Reliable) void OnRep_ActiveEffectsUpdated();

	// A simple timer for managing effect expiration
	UPROPERTY() FTimerHandle _EffectsTimer;

	FRWLock _MutexLock;

	bool bShowDebug = false;
	
};
