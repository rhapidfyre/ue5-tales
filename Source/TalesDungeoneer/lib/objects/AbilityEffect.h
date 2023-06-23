// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Delegates/Delegate.h"
#include "TalesDungeoneer/lib/datastructures/AbilityData.h"

#include "AbilityEffect.generated.h"


// Called when this effect has started play
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectActivated,
	UAbilityEffect*, AbilityEffect, FName, AbilityName);

// Called when this effect wears off
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectExpired,
	UAbilityEffect*, AbilityEffect, FName, AbilityName);

// Called when this effect ticks
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectTick,
UAbilityEffect*, AbilityEffect, FName, AbilityName);


/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API UAbilityEffect : public UObject
{
	GENERATED_BODY()
	
public:
	
	UAbilityEffect() {}
	UAbilityEffect(FName AbilityName, ACharacterBase* EffectInstigator = nullptr);
	
	UPROPERTY(BlueprintAssignable) FOnEffectActivated OnEffectActivated;
	UPROPERTY(BlueprintAssignable) FOnEffectExpired OnEffectExpired;
	UPROPERTY(BlueprintAssignable) FOnEffectTick OnEffectTick;

	UFUNCTION(BlueprintPure) bool DoesTimerTickIndependently() const { return bTicksIndependently; }
	UFUNCTION(BlueprintPure) float GetSecondsRemaining() const { return _TimeRemaining; }

	/**
	 * @brief Assigns the ability name, if the object has not yet been initialized.
	 * @param AbilityName The AbilityName to assign to this effect
	 */
	UFUNCTION(BlueprintCallable) void SetAbilityName(FName AbilityName);
	UFUNCTION(BlueprintPure) FName GetAbilityName() const { return _AbilityName; }
	/**
	 * @brief Assigns the effect instigator, if the object has not yet been initialized.
	 * @param EffectInstigator The character who caused this effect
	 */
	UFUNCTION(BlueprintCallable) void SetEffectInstigator(ACharacterBase* EffectInstigator);

	// Called internally by overload constructors
	// If an overload constructor is not used, this must be called directly
	void InitializeEffect();
	
	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintPure, Category = "Accessors")
	AActor* GetOwningActor() const { return GetTypedOuter<AActor>(); }
	
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override
	{
		check(GetOuter() != nullptr);
		return GetOuter()->GetFunctionCallspace(Function, Stack);
	}
	
	// Call "Remote" (aka, RPC) functions through the actors NetDriver
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override
	{
		check(!HasAnyFlags(RF_ClassDefaultObject));
		AActor* Owner = GetOwningActor();
		UNetDriver* NetDriver = Owner->GetNetDriver();
		if (NetDriver)
		{
			NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
			return true;
		}
		return false;
	}
	
	/*
	* Since this is a replicated object, typically only the Server should create and destroy these
	* Provide a custom destroy function to ensure these conditions are met.
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "My Object")
	void Destroy()
	{
		checkf(GetOwningActor()->HasAuthority() == true,
			TEXT("Destroy:: Object does not have authority to destroy itself!"));
		OnDestroyed();
		MarkAsGarbage();
	}

	UFUNCTION(BlueprintPure)
	bool HasAuthority() const
	{
		if (IsValid(GetOwningActor()))
			return GetOwningActor()->HasAuthority();
		return false;
	}
	
protected:
	
	virtual void PostInitProperties() override;

	virtual void BeginDestroy() override;

	// Called when play for this object starts. Activates the effect timer.
	// If overriden by child classes, the parent (Super) must be called.
	virtual void BeginPlay();

	// Applies the initial effects, such as damage and special fx
	virtual void ApplyInitialEffects();
	
	virtual bool IsSupportedForNetworking() const override { return true; }

	 virtual void GetLifetimeReplicatedProps(
		TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	// Called every half-second to process the effect
	virtual void TimerTick();
	
	virtual void OnDestroyed()
	{
		// Notify Owner etc.
	}
	
private:

	UPROPERTY() FTimerHandle _Timer;

	// The name of the ability that initiated this effect
	UPROPERTY(Replicated) FName _AbilityName = FName();
	
	// The time remaining on this effect
	UPROPERTY(Replicated) float _TimeRemaining = 0.f;
	
	// If true, this effects timer will reduce while active
	// If false, this effects timer will not reduce unless it is at the top of the stack
	UPROPERTY() bool bTicksIndependently = false;

	// The character that instigated this effect
	UPROPERTY() ACharacterBase* _EffectInstigator = nullptr;

	// The actor targeted by the effect
	UPROPERTY() AActor* _TargetActor = nullptr;

	bool bInitialized = false;

};
