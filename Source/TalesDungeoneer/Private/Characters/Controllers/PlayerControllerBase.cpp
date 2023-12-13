// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"

#include "TalesDungeoneer/Characters/CharacterBase.h"

// Sets default values
APlayerControllerBase::APlayerControllerBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// Set up hotkey action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		for (UInputMappingContext* MappingContext : MappingContexts)
		{
			
			//Add Input Mapping Context
			if (IsValid(MappingContext))
			{
				if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
						ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
				{
					Subsystem->AddMappingContext(MappingContext, 0);
				}
			}
			
		}
			
		// Setup Hotkey Ability bindings 
		for (UInputAction* InputReference : AbilityHotkeys)
		{
			EnhancedInputComponent->BindAction(InputReference, ETriggerEvent::Started,
					this, &APlayerControllerBase::HotkeyTriggered, InputReference);
		}
			
		// Setup Hotkey Target bindings 
		for (UInputAction* InputReference : TargetingHotkeys)
		{
			EnhancedInputComponent->BindAction(InputReference, ETriggerEvent::Started,
					this, &APlayerControllerBase::HotkeyTarget, InputReference);
		}
		
	}
}

FKey APlayerControllerBase::GetKeyMapping(UInputAction* InputAction)
{
	if (IsValid(InputAction))
	{
		for (UInputMappingContext* MappingContext : MappingContexts)
		{
			for (const FEnhancedActionKeyMapping& MappedKey : MappingContext->GetMappings())
			{
				if (MappedKey.Action == InputAction)
					return MappedKey.Key;
			}
		}
	}
	return FKey();
}

void APlayerControllerBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void APlayerControllerBase::HotkeyTriggered(UInputAction* HotkeyAction)
{
	if (IsValid(HotkeyAction))
	{
		const ACharacterBase* ControlledCharacter = Cast<ACharacterBase>( GetCharacter() );
		if (IsValid(ControlledCharacter))
		{
			if (IsValid(ControlledCharacter->AbilityComponent))
			{
				// Send hotkey to ability component for processing
				ControlledCharacter->AbilityComponent->AbilityAction(HotkeyAction);

				// Allow other scripts to hear the request
				OnHotkeyTriggered.Broadcast(HotkeyAction);
				
			}
		}
	}
}

void APlayerControllerBase::HotkeyTarget(UInputAction* HotkeyAction)
{
	if (IsValid(HotkeyAction))
	{
		const ACharacterBase* ControlledCharacter = Cast<ACharacterBase>( GetCharacter() );
		if (IsValid(ControlledCharacter))
		{
			if (IsValid(ControlledCharacter->AbilityComponent))
			{
				ControlledCharacter->AbilityComponent->SetTargetedActorByHotkey(HotkeyAction);
			}
		}
	}
}
