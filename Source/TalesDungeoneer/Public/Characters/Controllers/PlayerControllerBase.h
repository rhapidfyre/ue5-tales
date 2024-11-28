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
	UInputAction*, InputAction, const UClass*, AbilityReference);


USTRUCT(BlueprintType)
struct TALESDUNGEONEER_API FAbilityBindingData
{
	GENERATED_BODY()
	FAbilityBindingData() {}

	explicit FAbilityBindingData(const TSubclassOf<URsGameplayAbilityBase>& InAbility) : AbilityClass(InAbility) {}

	explicit FAbilityBindingData(UInputAction* InInputAction) : InputAction(InInputAction) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite) UInputAction* InputAction = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<URsGameplayAbilityBase> AbilityClass = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseTriggerEvent  = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseStartEvent    = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseOngoingEvent  = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseCancelEvent   = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bUseCompleteEvent = false;

	bool operator==(const FAbilityBindingData& rhs) const { return InputAction == rhs.InputAction; }
	bool operator==(const UInputAction* CompareAction) const { return InputAction == CompareAction; }
};


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

	UFUNCTION(BlueprintCallable) TSubclassOf<URsGameplayAbilityBase> GetPrimaryAbility();
	UFUNCTION(BlueprintCallable) TSubclassOf<URsGameplayAbilityBase> GetSecondaryAbility();

	// Allows easy access to changing out the ability bound to the primary attack action
	UFUNCTION(BlueprintCallable)
	void SetPrimaryActionAbility(TSubclassOf<URsGameplayAbilityBase> AbilityReference);

	// Allows easy access to changing out the ability bound to the secondary attack action
	UFUNCTION(BlueprintCallable)
	void SetSecondaryActionAbility(TSubclassOf<URsGameplayAbilityBase> AbilityReference);

	UFUNCTION(BlueprintCallable) void RespawnAtGraveyard();

	UFUNCTION(BlueprintCallable) void RespawnAtCorpse();

	UFUNCTION(BlueprintCallable) void RespawnAtEntrance();

	UPROPERTY(BlueprintAssignable) FOnHotkeyTriggered OnHotkeyTriggered;

protected: // methods

	UFUNCTION() // For Debugging
	void HotkeyUpdateDelegate(UInputAction* InputAction, const UClass* InClass);

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void SetupInputComponent() override;

	virtual void GetLifetimeReplicatedProps(
		TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:

	void HotkeyActionDelegate(const FInputActionValue& InputValue, UInputAction* InputAction, ETriggerEvent TriggerEvent);

	UFUNCTION(Server, Reliable)
	void Server_RespawnAtGraveyard();

	UFUNCTION(Server, Reliable)
	void Server_RespawnAtCorpse();

	UFUNCTION(Server, Reliable)
	void Server_RespawnAtEntrance();

	UFUNCTION(Server, Reliable)
	void Server_RequestWeaponReady(const FInputActionValue& InputValue, UInputAction* InputAction, ETriggerEvent TriggerEvent);

	UFUNCTION(Server, Reliable)
	void Server_SetHotkeyAbility(UInputAction* InputReference, UClass* AbilityReference);

	void RespawnPawn(const FGameplayTag& LocationTag);

	UFUNCTION(Client, Reliable)
	void OnRep_HotkeyAbilityMap(const TArray<FAbilityBindingData>& OldMappings);

public: //members

	UPROPERTY(BlueprintAssignable) FOnAbilityHotkeyUpdated OnAbilityHotkeyUpdated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")
	TArray<UInputMappingContext*> MappingContexts;

	// Maps input actions to specific abilities (hotkey 1, etc.)
	UPROPERTY(ReplicatedUsing=OnRep_HotkeyAbilityMap, EditAnywhere, BlueprintReadWrite, Category = "Input Actions")
	TArray<FAbilityBindingData> HotkeyAbilityMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions") UInputAction* PrimaryAttackInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions") UInputAction* SecondaryAttackInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")	UInputAction* ActionTargetHostile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")	UInputAction* ActionTargetFriend;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Actions")	UInputAction* ActionTargetSelf;

protected:


};
