// Copyright Take Five Games, LLC 2023 - All rights reserved


#include "Characters/PlayerCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Logging/StructuredLog.h"
#include "Gas/AttributeSets/TalesAttributes.h"
#include "Saves/SavedCharacters.h"
#include "TalesDungeoneer/TalesDungeoneer.h"


// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

void APlayerCharacterBase::HotkeyTriggered(UInputAction* HotkeyAction)
{
	UE_LOGFMT(LogTemp, Display,
		"{CharacterName}({Sv}): Hotkey '{KeyName}' Triggered",
		GetName(), HasAuthority()?"SV":"CL", HotkeyAction->GetName());
}

void APlayerCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
					(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
}

void APlayerCharacterBase::BindListeners()
{
	// Setup Vitality Listeners
	Super::BindListeners();

	// We only need to know the rest of the attributes if they belong to
	//  the person playing this character.
	if (!IsValid(GetController()))
	{
		return;
	}
	
	if (GetController()->IsLocalPlayerController())
	{
		TArray CoreStatAttributes = {
			AttributeCoreStatsSet->GetStrengthAttribute(),
			AttributeCoreStatsSet->GetDexterityAttribute(),
			AttributeCoreStatsSet->GetFortitudeAttribute(),
			AttributeCoreStatsSet->GetAstutenessAttribute(),
			AttributeCoreStatsSet->GetIntellectAttribute(),
			AttributeCoreStatsSet->GetCharismaAttribute()
		};

		// bind to core stat changes
		for (const FGameplayAttribute& coreAttribute : CoreStatAttributes)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				coreAttribute).AddUObject(this, &APlayerCharacterBase::OnCoreStatsChanged);
			UE_LOGFMT(LogTemp, Display, "{CharacterName}({Sv}): "
				"Successfully initialized delegate for CoreStat Attribute '{vAttribute}'",
				GetCharacterName(), HasAuthority()?"SRV":"CLI", coreAttribute.AttributeName);
		}
	
		TArray DamageAttributes = {
			AttributeDamageSet->GetBluntBonusAttribute(), 	AttributeDamageSet->GetBluntResistanceAttribute(),
			AttributeDamageSet->GetSlashBonusAttribute(), 	AttributeDamageSet->GetSlashResistanceAttribute(),
			AttributeDamageSet->GetPierceBonusAttribute(),	AttributeDamageSet->GetPierceResistanceAttribute(),
			AttributeDamageSet->GetBiteBonusAttribute(), 	AttributeDamageSet->GetBiteResistanceAttribute(),
			AttributeDamageSet->GetClawBonusAttribute(), 	AttributeDamageSet->GetClawResistanceAttribute(),
			AttributeDamageSet->GetKickBonusAttribute(), 	AttributeDamageSet->GetKickResistanceAttribute(),
			AttributeDamageSet->GetStingBonusAttribute(),	AttributeDamageSet->GetStingResistanceAttribute(),
			AttributeDamageSet->GetFireBonusAttribute(),	AttributeDamageSet->GetFireResistanceAttribute(),
			AttributeDamageSet->GetFrostBonusAttribute(),	AttributeDamageSet->GetFrostResistanceAttribute(),
			AttributeDamageSet->GetAcidBonusAttribute(),	AttributeDamageSet->GetAcidResistanceAttribute(),
			AttributeDamageSet->GetShockBonusAttribute(), 	AttributeDamageSet->GetShockResistanceAttribute(),
			AttributeDamageSet->GetRadioBonusAttribute(), 	AttributeDamageSet->GetRadioResistanceAttribute(),
			AttributeDamageSet->GetSonicBonusAttribute(), 	AttributeDamageSet->GetSonicResistanceAttribute(),
			AttributeDamageSet->GetHolyBonusAttribute(), 	AttributeDamageSet->GetHolyResistanceAttribute(),
			AttributeDamageSet->GetDarkBonusAttribute(), 	AttributeDamageSet->GetDarkResistanceAttribute(),
		};

		// bind to damage resistance/bonus changes
		for (const FGameplayAttribute& damageAttribute : DamageAttributes)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				damageAttribute).AddUObject(this, &APlayerCharacterBase::OnDamageStatsChanged);
			UE_LOGFMT(LogTemp, Display, "{CharacterName}({Sv}): "
				"Successfully initialized delegate for Damage Attribute '{vAttribute}'",
				GetCharacterName(), HasAuthority()?"SRV":"CLI", damageAttribute.AttributeName);
		}
	}
}

void APlayerCharacterBase::BindInput()
{
	if (bIsInputBound || !IsValid(AbilitySystemComponent) || !IsValid(InputComponent))
	{
		return;
	}

	FTopLevelAssetPath EnumAssetPath = FTopLevelAssetPath(
		FName("/Script/TalesDungeoneer"), FName("EAbilityInputID"));
	
	GetAbilitySystemComponent()->BindAbilityActivationToInputComponent(InputComponent,
		FGameplayAbilityInputBinds(
			FString("Confirm"),
			FString("Cancel"),
			EnumAssetPath,
		static_cast<int32>(EAbilityInputID::Confirm),
		static_cast<int32>(EAbilityInputID::Cancel)));
	bIsInputBound = true;
}

void APlayerCharacterBase::OnCoreStatsChanged(const FOnAttributeChangeData& Data)
{
	Super::OnCoreStatsChanged(Data);
	OnCoreStatsUpdated.Broadcast(Data.Attribute, Data.NewValue);
}

void APlayerCharacterBase::OnDamageStatsChanged(const FOnAttributeChangeData& Data)
{
	Super::OnDamageStatsChanged(Data);
	OnDamageStatsUpdated.Broadcast(Data.Attribute, Data.NewValue);
}

void APlayerCharacterBase::OnVitalityAttributeChanged(const FOnAttributeChangeData& Data)
{
	if (Data.Attribute == AttributeVitalitySet->GetCurrentHungerAttribute())
	{
		OnAttributeUpdated.Broadcast(Data.Attribute, Data.NewValue);
		OnAttributeHungerUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventHungerChanged(Data.OldValue, Data.NewValue);
	}
	else if (Data.Attribute == AttributeVitalitySet->GetCurrentHydrationAttribute())
	{
		OnAttributeHydrationUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventHydration(Data.OldValue, Data.NewValue);
	}
	else if (Data.Attribute == AttributeVitalitySet->GetCurrentArmorClassAttribute())
	{
		OnAttributeArmorClassUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventArmorClassChanged(Data.OldValue, Data.NewValue);
	}
	else
	{
		Super::OnVitalityAttributeChanged(Data);
		return;
	}
	OnAttributeUpdated.Broadcast(Data.Attribute, Data.NewValue);
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpInputAction,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::Jump);
		
		EnhancedInputComponent->BindAction(JumpInputAction,
			ETriggerEvent::Completed, this, &APlayerCharacterBase::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveInputAction,
		ETriggerEvent::Triggered, this, &APlayerCharacterBase::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookInputAction,
		ETriggerEvent::Triggered, this, &APlayerCharacterBase::Look);

	}
	
	// Call the function that handles ability system bindings
	BindInput();
}

void APlayerCharacterBase::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacterBase::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}