// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"

#include "PlayerControllerBase.generated.h"

UCLASS()
class TALESDUNGEONEER_API APlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public: // methods
	
	APlayerControllerBase();

	UFUNCTION(BlueprintCallable)
	FKey GetKeyMapping(UInputAction* InputAction);
	
protected: // methods

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual void HotkeyTriggered(UInputAction* HotkeyAction);

	virtual void SetupKeyBindings();

public: //members

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input, meta = (AllowPrivateAccess = "true"))
	TArray<UInputMappingContext*> MappingContexts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UInputAction*> Hotkeys;
	
};
