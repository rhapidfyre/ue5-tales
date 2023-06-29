// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// ReSharper disable once CppUnusedIncludeDirective
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "Delegates/Delegate.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHotkeyTriggered,
		UInputAction*, HotkeyPressed);

UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public: // methods
	
	APlayerControllerBase();

	UFUNCTION(BlueprintCallable)
	FKey GetKeyMapping(UInputAction* InputAction);

	UPROPERTY(BlueprintAssignable) FOnHotkeyTriggered OnHotkeyTriggered;
	
protected: // methods

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void HotkeyTriggered(UInputAction* HotkeyAction);

	virtual void HotkeyTarget(UInputAction* HotkeyAction);

	virtual void SetupInputComponent() override;

public: //members

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	TArray<UInputMappingContext*> MappingContexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UInputAction*> AbilityHotkeys;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UInputAction*> TargetingHotkeys;
	
};
