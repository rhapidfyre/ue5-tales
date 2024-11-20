// Starcache Studios, LLC (c) 2024

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputMappingContext.h"
#include "Delegates/Delegate.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBase.generated.h"

class URsGameplayAbilityBase;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotkeyTriggered,
	UInputAction*, HotkeyPressed);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityHotkeyUpdated,
	const UInputAction*, InputAction, const UClass*, AbilityReference);

UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public: // methods

	APlayerControllerBase();

	UFUNCTION(BlueprintCallable)
	FKey GetKeyMapping(UInputAction* InputAction);

	UFUNCTION(BlueprintCallable)
	bool SetHotkeyAbility(UInputAction* InputReference, TSubclassOf<URsGameplayAbilityBase> AbilityReference);

	UFUNCTION(BlueprintCallable) void RespawnAtGraveyard();

	UFUNCTION(BlueprintCallable) void RespawnAtCorpse();

	UFUNCTION(BlueprintCallable) void RespawnAtEntrance();

	UPROPERTY(BlueprintAssignable) FOnHotkeyTriggered OnHotkeyTriggered;

protected: // methods

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void HotkeyTarget(UInputAction* HotkeyAction);

	virtual void SetupInputComponent() override;

private:

	void HotkeyActionDelegate(const FInputActionValue& InputValue, UInputAction* InputAction, const bool bStarted, const bool bCanceled);

	//void PrimaryHotkeyDelegate(const FInputActionValue& InputValue);

	//void SecondaryHotkeyDelegate(const FInputActionValue& InputValue, const bool bStarted, const bool bCanceled);

	UFUNCTION(Server, Reliable)
	void Server_RespawnAtGraveyard();

	UFUNCTION(Server, Reliable)
	void Server_RespawnAtCorpse();

	UFUNCTION(Server, Reliable)
	void Server_RespawnAtEntrance();

	void RespawnPawn(const FGameplayTag& LocationTag);


public: //members

	UPROPERTY(BlueprintAssignable) FOnAbilityHotkeyUpdated OnAbilityHotkeyUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")
	TArray<UInputMappingContext*> MappingContexts;

	// Maps input actions to specific abilities (primary, secondary, hotkey 1, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")
	TMap<UInputAction*, TSubclassOf<URsGameplayAbilityBase>> HotkeyAbilityMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")	UInputAction* ActionTargetHostile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")	UInputAction* ActionTargetFriend;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")	UInputAction* ActionTargetSelf;

};
