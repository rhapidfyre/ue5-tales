// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TalesDungeoneer/lib/datastructures/AbilityData.h"
#include "Delegates/Delegate.h"

#include "AbilityComponent.generated.h"


// Called when an ability has been added
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAbilityActivated, FName, AbilityName, int, StackCount);

// Called everytime an ability expires
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAbilityExpired, FName, AbilityName, int, StackCount);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAbilityComponent();

	UPROPERTY(BlueprintCallable) FOnAbilityActivated OnAbilityActivated;
	UPROPERTY(BlueprintCallable) FOnAbilityExpired	 OnAbilityExpired;

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
	
protected:
	
	virtual void BeginPlay() override;

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


	// Mappings for abilities, i.e: Hotkeys
	UPROPERTY(Replicated, EditAnywhere, BlueprintReadWrite)
	TArray<FStAbilityHotkey> _AbilityActions;

	// Overrideable event that fires everytime the Effects Timer ticks
	UFUNCTION(BlueprintNativeEvent) void OnTickTimer();
	
private:

	// Called when an ability has been added and the current stack count
	UFUNCTION(Client, Reliable)
	void Client_AbilityAdded(FName AbilityName, int StackCount);

	// Called when an ability has expired and the current stack count
	// Stack count is zero if the effect has fully worn off
	UFUNCTION(Client, Reliable)
	void Client_AbilityExpired(FName AbilityName, int StackCount);

	UPROPERTY() AActor* _TargetActor;

	// Fires every time the Effects Timer ticks
	virtual void TickTimer();

	// Active abilities
	TMap< FName, TArray<FStAbilityEffect> > _ActiveEffects;

	// A simple timer for managing effect expiration
	UPROPERTY() FTimerHandle _EffectsTimer;

	FRWLock _MutexLock;

	bool bShowDebug = false;
	
};
