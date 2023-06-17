// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterBase.h"

#include "EnhancedInputSubsystems.h"


// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(HotkeySynergy,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::HotkeyTriggered, HotkeySynergy);
		
		EnhancedInputComponent->BindAction(HotkeyOne,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::HotkeyTriggered, HotkeyOne);
		
		EnhancedInputComponent->BindAction(HotkeyTwo,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::HotkeyTriggered, HotkeyTwo);
		
		EnhancedInputComponent->BindAction(HotkeyThree,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::HotkeyTriggered, HotkeyThree);
		
		EnhancedInputComponent->BindAction(HotkeyFour,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::HotkeyTriggered, HotkeyFour);
		
		EnhancedInputComponent->BindAction(HotkeyFive,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::HotkeyTriggered, HotkeyFive);

		EnhancedInputComponent->BindAction(HotkeySix,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::HotkeyTriggered, HotkeySix);
	}
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if ( APlayerController* PlayerController = Cast<APlayerController>(Controller) )
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
					(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(HotkeyMappingContext, 0);
		}
	}

}

void APlayerCharacterBase::HotkeyTriggered(UInputAction* HotkeyAction)
{
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AbilityAction(HotkeyAction);
	}
}
