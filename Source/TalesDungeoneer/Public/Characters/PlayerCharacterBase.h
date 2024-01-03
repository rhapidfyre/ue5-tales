// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

#include "PlayerCharacterBase.generated.h"


class USavedCharacter;

// Called when this player has fully spawned into the world
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerJoined);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCoreStatUpdated,
	const FGameplayAttribute&, AttributeData, const float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageStatUpdated,
	const FGameplayAttribute&, AttributeData, const float, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeArmorClassUpdated,
	const float&, OldValue, const float&, NewValue);

/**
 * Player Character Base is the base C++ class for all logic, methods and members that affect all
 * PLAYER based characters, prior to handling by child classes or dependent blueprint classes.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API APlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public: // functions

	APlayerCharacterBase();
	
	UPROPERTY(BlueprintAssignable) FOnPlayerJoined OnPlayerJoined;
	UPROPERTY(BlueprintAssignable) FOnCoreStatUpdated OnCoreStatsUpdated;
	UPROPERTY(BlueprintAssignable) FOnDamageStatUpdated OnDamageStatsUpdated;
	UPROPERTY(BlueprintAssignable) FOnAttributeArmorClassUpdated OnAttributeArmorClassUpdated;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputMappingContext* DefaultMappingContext = nullptr;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* JumpInputAction = nullptr;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* MoveInputAction = nullptr;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* LookInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* PrimaryAttackInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* SecondaryAttackInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* PrimaryInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* SecondaryInputAction = nullptr;
	
protected:

	UFUNCTION()
	virtual void HotkeyTriggered(UInputAction* HotkeyAction);

	// Called for movement input
	void Move(const FInputActionValue& Value);

	// Called for looking input
	void Look(const FInputActionValue& Value);
	
	virtual void BeginPlay() override;

	virtual void BindListeners() override;

	virtual void BindInput();

	virtual void OnVitalityAttributeChanged(const FOnAttributeChangeData& Data) override;
	
	virtual void OnCoreStatsChanged(const FOnAttributeChangeData& Data) override;
	
	virtual void OnDamageStatsChanged(const FOnAttributeChangeData& Data) override;
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintImplementableEvent) void EventArmorClassChanged(float OldValue, float NewValue);
	
};
