// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TalesDungeoneer/lib/datastructures/AbilityData.h"
#include "Delegates/Delegate.h"

#include "AbilityComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityActivated, AAbilityEffectBase*, AbilityEffect);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityReady, FName, AbilityName);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAbilityComponent();

	UPROPERTY(BlueprintCallable) FOnAbilityActivated OnAbilityActivated;
	UPROPERTY(BlueprintCallable) FOnAbilityReady	 OnAbilityReady;

	/**
	 * @brief Called to activate the requested ability 
	 * @param AbilityName The FName of the ability to request
	 */
	UFUNCTION(BlueprintCallable)
	void ActivateAbility(const FName AbilityName);

protected:
	
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnComponentCreated() override;

	/**
	 * @brief Sends the ability activation request to the server in case
	 * ActivateAbility() was called on the client.
	 * @param AbilityName The ability FName to request
	 */
	UFUNCTION(Server, Reliable)
	void Server_RequestAbility(FName AbilityName = FName());

private:

	// Called when an effect is added
	// The effect expiration will be handled internally by AAbilityEffectBase
	UFUNCTION(Client, Reliable)
	void Client_AbilityAdded(AAbilityEffectBase* NewEffect);

	bool bShowDebug = false;
	
};
