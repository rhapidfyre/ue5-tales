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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewTargetSet,
ACharacterBase*, NewTarget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityHotkeyChanged,
	UInputAction*, HotkeyAction, FName, AbilityName);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityLearned, FName, AbilityName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityForgotten, FName, AbilityName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilitiesReset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockPointsChanged, int, TotalPoints);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityOnCooldown,
	FName, AbilityName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityReady,
	FName, AbilityName);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAbilityComponent();

	void InitializePoints(int NumberOfPoints = 0);

	UFUNCTION(Server, Reliable) void Server_InitializePoints(int NumberOfPoints = 0);

	UPROPERTY(BlueprintAssignable) FOnAbilityCastStarted		OnAbilityCastStarted;
	UPROPERTY(BlueprintAssignable) FOnAbilityCastComplete		OnAbilityCastComplete;
	UPROPERTY(BlueprintAssignable) FOnAbilityAdded				OnAbilityAdded;
	UPROPERTY(BlueprintAssignable) FOnAbilityRemoved			OnAbilityRemoved;
	UPROPERTY(BlueprintAssignable) FOnActiveEffectsUpdated  	OnActiveEffectsUpdated;
	UPROPERTY(BlueprintAssignable) FOnNewTargetSet				OnNewTargetSet;
	UPROPERTY(BlueprintAssignable) FOnAbilityLearned			OnAbilityLearned;
	UPROPERTY(BlueprintAssignable) FOnAbilityForgotten			OnAbilityForgotten;
	UPROPERTY(BlueprintAssignable) FOnAbilitiesReset			OnAbilitiesReset;
	UPROPERTY(BlueprintAssignable) FOnAbilityHotkeyChanged  	OnAbilityHotkeyChanged;
	UPROPERTY(BlueprintAssignable) FOnUnlockPointsChanged		OnUnlockPointsChanged;
	UPROPERTY(BlueprintAssignable) FOnAbilityOnCooldown  		OnAbilityOnCooldown;
	UPROPERTY(BlueprintAssignable) FOnAbilityReady				OnAbilityReady;
	
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

	UFUNCTION(BlueprintPure)
	TMap<UInputAction*, FName> GetAbilityMappings() const { return AbilityMappings_; };
	
	// The tick rate of the Effects Timer
	// The timer does not run if there are no active effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float TimerRate = 0.1;

	UFUNCTION(BlueprintPure) ACharacterBase* GetTargetedActor() const { return TargetActor_; }

	UFUNCTION(BlueprintCallable)
	void SetTargetedActorByHotkey(UInputAction* TargetHotkey);

	UFUNCTION(BlueprintCallable)
	void SetTargetedActor(ACharacterBase* NewTarget);
	
	// Removes the expired effect from the TArray
	UFUNCTION(BlueprintCallable)
	void RemoveExpiredEffect(UStatusEffect* AbilityEffect, FName AbilityName);

	UFUNCTION(BlueprintPure)
	TArray<UStatusEffect*> GetActiveEffects() { return ActiveEffects_; }

	UFUNCTION(BlueprintCallable) int GetNumStacksActive(FName AbilityName);

	UFUNCTION(BlueprintCallable) bool GetIsEffectActiveByName(FName AbilityName);

	// Returns the ability effect that has the least amount of time left
	UFUNCTION(BlueprintCallable) UStatusEffect* GetEffectWithLowestTimer(FName AbilityName = "None");

	// Returns the ability effect that has the greatest amount of time left
	UFUNCTION(BlueprintCallable) UStatusEffect* GetEffectWithGreatestTimer(FName AbilityName = "None");
	
	// Returns the total time of all effects in the stack
	UFUNCTION(BlueprintCallable) float GetTotalEffectStackTimer(FName AbilityName = "None");

	UFUNCTION(BlueprintPure) bool GetIsAbilityOnCooldown(FName AbilityName) const { return AbilitiesOnCooldown_.Contains(AbilityName);}
	UFUNCTION(BlueprintCallable) void EndAbilityCooldown(FName AbilityName);
	/**
	 * @brief Call to stop the casting of any ability.
	 * @param OnlyFocused If true, interrupt ONLY focused abilities. False interrupts everything.
	 */
	UFUNCTION(BlueprintCallable) void InterruptCasting(bool OnlyFocused = false, bool CanceledIntentionally = false);

	UFUNCTION(BlueprintPure) TArray<FName> GetKnownAbilities() const { return KnownAbilities_; }

	UFUNCTION(Server, Reliable, BlueprintCallable) void Server_RequestAbilityAdd(FName AbilityName);
	UFUNCTION(Server, Reliable, BlueprintCallable) void Server_RequestAbilityRemove(FName AbilityName);
	UFUNCTION(Server, Reliable, BlueprintCallable) void Server_RequestAbilityReset();

	UFUNCTION(BlueprintPure) int GetNumberOfUnlockPoints() const { return UnlockPoints_; }
	UFUNCTION(BlueprintCallable) void AddUnlockPoints(int NumPoints = 1);
	UFUNCTION(BlueprintCallable) void RemoveUnlockPoints(int NumPoints = 1);

	UFUNCTION(BlueprintCallable)
	bool StartCasting(FName AbilityName);
	
	UFUNCTION(BlueprintCallable)
	bool StopCasting(FName AbilityName, bool WasSuccessful = true);

	UFUNCTION(BlueprintCallable)
	bool IsAbilityInProgress(FName AbilityName) const { return AbilitiesInProgress_.Contains(AbilityName); }

	UFUNCTION(BlueprintCallable)
	void SetUnlockPoints(int UnlockPoints = 0);
	
	UFUNCTION(BlueprintCallable)
	void AddKnownAbility(FName AbilityName, int UnlockPoints = 1);
	
	UFUNCTION(BlueprintCallable)
	void RemoveKnownAbility(FName AbilityName);
	
	UFUNCTION(BlueprintCallable)
	void ResetKnownAbilities();
	
protected:
	
	virtual void BeginPlay() override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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
	void Server_InterruptCasting(bool OnlyFocused = false, bool CanceledIntentionally = false);

	UFUNCTION(BlueprintCallable) void CancelCasting(FName AbilityName);
	UFUNCTION(Server, Reliable)	void Server_CancelCasting(FName AbilityName);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<UInputAction*, FName> AbilityMappings_;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<UInputAction*, ETargetingOption> TargetMappings_;

	// Overrideable event that fires everytime the Effects Timer ticks
	UFUNCTION(BlueprintNativeEvent) void OnTickTimer();
	
	UFUNCTION(BlueprintPure) bool GetIsCasting() const { return bIsCasting; }
	UFUNCTION(BlueprintCallable) void SetIsCasting(FName SpellName);

	// Used by the timer delegate to cancel casting
	UFUNCTION()
	void SetNoLongerCasting(FName AbilityName, bool WasSuccessful = false);
	
private:

	bool bHasInitialized = false;
	
	UFUNCTION(Client, Reliable) void OnRep_UnlockPoints(int OldPointsCount);
	UPROPERTY(ReplicatedUsing=OnRep_UnlockPoints) int UnlockPoints_ = 2;

	USkeleton* GetOwnerSkeleton();
	
	UFUNCTION()
	void DelayedAnimation(UAnimMontage* AnimToPlay, bool bIsLooped = false);
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_CastingAnimation(UAnimMontage* AnimToPlay, bool bIsLooped = false);
	
	UFUNCTION(Server, Reliable)
	void Server_RequestTarget(ACharacterBase* NewTarget);

	/**
	 * @brief An ability has failed to cast
	 * @param AbilityName The name of the ability. 'None' cancels ALL abilities.
	 * @param FailureReason The reason for the failure
	 */
	UFUNCTION(Client, Reliable)
	void Client_AbilityFailure(FName AbilityName, const FString& FailureReason);

	/**
	 * @brief Cancels a spell for reason other than a failure
	 * @param AbilityName The name of the ability. 'None' cancels ALL abilities.
	 * @param CancelReason The reason for the cancellation
	 */
	UFUNCTION(Client, Reliable)
	void Client_AbilityCanceled(FName AbilityName, const FString& CancelReason);

	UFUNCTION()	void DestroyAllEffects();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_StopCasting(FName AbilityName, bool WasSuccessful);

	// True if the player is casting any abilities
	UPROPERTY(Replicated) bool bIsCasting = false;

	// If true, the player cannot use any other focused abilities
	UPROPERTY(Replicated) FName FocusedAbility = FName(); 

	UPROPERTY(ReplicatedUsing=OnRep_TargetActor) ACharacterBase* TargetActor_;
	UFUNCTION(Client, Reliable) void OnRep_TargetActor();

	// Fires every time the Effects Timer ticks
	virtual void TickTimer();

	// Active abilities
	//TMap< FName, TArray<FStAbilityEffect> > ActiveEffects_;
	UPROPERTY(ReplicatedUsing=OnRep_ActiveEffectsUpdated)
	TArray<UStatusEffect*> ActiveEffects_;
	UFUNCTION(NetMulticast, Reliable) void OnRep_ActiveEffectsUpdated();

	UFUNCTION(Client, Reliable)
	void OnRep_KnownAbilities(const TArray<FName>& OldAbilityList);
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_KnownAbilities)
	TArray<FName> KnownAbilities_;

	UFUNCTION(Client, Reliable) void Client_AbilityCooldown(FName AbilityName, bool OnCooldown = true);

	// A simple timer for managing effect expiration
	UPROPERTY() FTimerHandle EffectsTimer_;

	FRWLock MutexLock_;

	bool bShowDebug = false;

	FVector OwnerSpeed_ = FVector(0.f);
	UPROPERTY() ACharacterBase* PlayerCharacter_;

	UPROPERTY() TSet<FName> AbilitiesInProgress_;
	UPROPERTY() TSet<FName> AbilitiesOnCooldown_;
	
	UFUNCTION(Client, Reliable) void Client_StartCasting(FName AbilityName);
	UFUNCTION(Client, Reliable) void Client_StopCasting(FName AbilityName, bool WasSuccessful = true);
};
