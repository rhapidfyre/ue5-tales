// Starcache Studios, LLC (c) 2024


#include "Characters/Controllers/PlayerControllerBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Abilities/RsGameplayAbilityBase.h"
#include "Actors/TalesRespawnBase.h"
#include "Characters/CharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APlayerControllerBase::APlayerControllerBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

/**
 * \brief Currently just used for debugging
 * \param InputAction
 * \param Class
 */
void APlayerControllerBase::HotkeyUpdateDelegate(UInputAction* InputAction, const UClass* InClass)
{
	if (HotkeyAbilityMap.Contains(InputAction))
	{
		TSubclassOf<URsGameplayAbilityBase> AbilityReference = const_cast<UClass*>(InClass);
		TSubclassOf<URsGameplayAbilityBase> PreviousAbility = nullptr;

		FAbilityBindingData AbilityBindingData(InputAction);
		const int HotkeyIndex = HotkeyAbilityMap.Find(AbilityBindingData);
		if (HotkeyIndex != INDEX_NONE)
		{
			PreviousAbility = HotkeyAbilityMap[HotkeyIndex].AbilityClass;
		}

		UE_LOGFMT(LogTemp, Display, "{Name}({NetAuthority}): Primary Ability Changed... '{OldAbility}' -> '{NewAbility}'"
			, GetName(), HasAuthority() ? "SERVER" : "CLIENT"
			, IsValid(PreviousAbility) ? PreviousAbility->GetName() : "None"
			, IsValid(AbilityReference) ? AbilityReference->GetName() : "None");
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "{Name}({NetAuthority}): Hotkey Update Failed - Input Action '{InputAction}' is not in the hotkey binding map."
			, GetName(), HasAuthority() ? "SERVER" : "CLIENT", IsValid(InputAction) ? InputAction->GetName() : "None");
	}
}

void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
	if (OnAbilityHotkeyUpdated.IsAlreadyBound(this, &APlayerControllerBase::HotkeyUpdateDelegate))
	{
		OnAbilityHotkeyUpdated.AddDynamic(this, &APlayerControllerBase::HotkeyUpdateDelegate);
	}
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

		// Add the primary and secondary attack input to the hotkey map if they aren't already
		if (!HotkeyAbilityMap.Contains(PrimaryAttackInput) && IsValid(PrimaryAttackInput))
		{
			HotkeyAbilityMap.Add(FAbilityBindingData(PrimaryAttackInput));
		}
		if (!HotkeyAbilityMap.Contains(SecondaryAttackInput) && IsValid(SecondaryAttackInput))
		{
			HotkeyAbilityMap.Add(FAbilityBindingData(SecondaryAttackInput));
		}

		// Bind hotkey assignments, whether they map to a real ability or not
		for (auto& HotkeyData : HotkeyAbilityMap)
		{
			if (IsValid(HotkeyData.InputAction))
			{
				if (HotkeyData.bUseStartEvent)
					EnhancedInputComponent->BindAction(HotkeyData.InputAction, ETriggerEvent::Started,
					this, &APlayerControllerBase::HotkeyActionDelegate, HotkeyData.InputAction, ETriggerEvent::Started);
			}
		}


	}
}

void APlayerControllerBase::HotkeyActionDelegate(
	const FInputActionValue& InputValue, UInputAction* InputAction, ETriggerEvent TriggerEvent)
{
	// TODO - Handle canceled/completed behavior
	if (TriggerEvent != ETriggerEvent::Started) { return; }
	// For now, this is only handling starting new activations

	if (!IsValid(GetPawn()))
	{
		return;
	}

	UActorComponent* ActorComponent = GetPawn()->GetComponentByClass(URsAbilityComponent::StaticClass());
	URsAbilityComponent* AbilityComponent = Cast<URsAbilityComponent>(ActorComponent);
	if (IsValid(AbilityComponent))
	{

		TSubclassOf<URsGameplayAbilityBase> AbilityClass = nullptr;
		FAbilityBindingData AbilityBindingData(InputAction);
		const int HotkeyIndex = HotkeyAbilityMap.Find(AbilityBindingData);
		if (HotkeyIndex != INDEX_NONE)
		{
			AbilityClass = HotkeyAbilityMap[HotkeyIndex].AbilityClass;
		}

		if (IsValid(AbilityClass))
		{
			AbilityComponent->HotkeyAbility(InputValue, AbilityClass, TriggerEvent);
		}
	}
}

void APlayerControllerBase::Server_RequestWeaponReady_Implementation(
	const FInputActionValue& InputValue, UInputAction* InputAction, ETriggerEvent TriggerEvent)
{
	HotkeyActionDelegate(InputValue, InputAction, TriggerEvent);
}

void APlayerControllerBase::RespawnPawn(const FGameplayTag& LocationTag)
{
	if (!HasAuthority())
	{
		return;
	}

	// Get respawn area
	APawn* ControlledPawn = GetPawn();
	ERespawnType RespawnType = ERespawnType::None;
	if (LocationTag.MatchesTag(TAG_Respawners_Graveyard.GetTag()))
	{
		RespawnType = ERespawnType::Graveyard;
	}
	else if (LocationTag.MatchesTag(TAG_Respawners_Entrance.GetTag()))
	{
		RespawnType = ERespawnType::Entrance;
	}
	FVector GraveyardVector = ControlledPawn->GetActorLocation();

	{
		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATalesRespawnBase::StaticClass(), AllActors);
		if (AllActors.Num() > 0)
		{
			for (AActor* RespawnActor : AllActors)
			{
				ATalesRespawnBase* RespawnBase = Cast<ATalesRespawnBase>(RespawnActor);
				if (RespawnBase->RespawnTag.MatchesTag(LocationTag))
				{
					GraveyardVector = RespawnBase->GetActorLocation();
					break;
				}
			}
		}
	}

	// 1. Move the player pawn to the graveyard
	ControlledPawn->SetActorLocation(GraveyardVector);

	// 2. Respawn the character
	ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetCharacter());
	if (IsValid(CharacterBase))
	{
		CharacterBase->Respawn(RespawnType);
	}
}

void APlayerControllerBase::OnRep_HotkeyAbilityMap_Implementation(const TArray<FAbilityBindingData>& OldMappings)
{
	for (const auto& HotkeyMapping : HotkeyAbilityMap)
	{
		// Existing Mapping Updated
		const int OldIndex = OldMappings.Find(HotkeyMapping);
		if (OldIndex != INDEX_NONE)
		{
			FAbilityBindingData OldMapping = OldMappings[OldIndex];
			if (OldMapping.AbilityClass != HotkeyMapping.AbilityClass)
				OnAbilityHotkeyUpdated.Broadcast(HotkeyMapping.InputAction, HotkeyMapping.AbilityClass);
		}

		// New Mapping (if index is invalid)
		else
			OnAbilityHotkeyUpdated.Broadcast(HotkeyMapping.InputAction, HotkeyMapping.AbilityClass);
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

bool APlayerControllerBase::SetHotkeyAbility(
	UInputAction* InputReference, const TSubclassOf<URsGameplayAbilityBase> AbilityReference)
{
	// TODO - See if there's a better way of doing this other than server authoritative keybindings
	if (!HasAuthority())
	{
		Server_SetHotkeyAbility(InputReference, AbilityReference);
		return true;
	}

	if (IsValid(InputReference))
	{
		FAbilityBindingData AbilityBindingData(InputReference);
		const int HotkeyIndex = HotkeyAbilityMap.Find(AbilityBindingData);
		if (HotkeyIndex != INDEX_NONE)
		{
			HotkeyAbilityMap[HotkeyIndex].AbilityClass = AbilityReference;
		}
		else
		{
			AbilityBindingData.AbilityClass = AbilityReference;
			HotkeyAbilityMap.Add(AbilityBindingData);
		}
		OnAbilityHotkeyUpdated.Broadcast(InputReference, AbilityReference);

		return true;
	}
	return false;
}

void APlayerControllerBase::Server_SetHotkeyAbility_Implementation(UInputAction* InputReference, UClass* AbilityReference)
{
	TSubclassOf<URsGameplayAbilityBase> AbilityClass = AbilityReference;
	SetHotkeyAbility(InputReference, AbilityClass);
}

TSubclassOf<URsGameplayAbilityBase> APlayerControllerBase::GetPrimaryAbility()
{
	TSubclassOf<URsGameplayAbilityBase> AbilityMapped;
	if (PrimaryAttackInput)
	{
		for (const auto& HotkeyData : HotkeyAbilityMap)
		{
			if (HotkeyData.InputAction == PrimaryAttackInput)
				return HotkeyData.AbilityClass;
		}
	}
	return {};
}

TSubclassOf<URsGameplayAbilityBase> APlayerControllerBase::GetSecondaryAbility()
{
	TSubclassOf<URsGameplayAbilityBase> AbilityMapped;
	if (SecondaryAttackInput)
	{
		for (const auto& HotkeyData : HotkeyAbilityMap)
		{
			if (HotkeyData.InputAction == SecondaryAttackInput)
				return HotkeyData.AbilityClass;
		}
	}
	return {};
}

void APlayerControllerBase::SetPrimaryActionAbility(TSubclassOf<URsGameplayAbilityBase> AbilityReference)
{
	if (IsValid(PrimaryAttackInput))
	{
		SetHotkeyAbility(PrimaryAttackInput, AbilityReference);
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "{Name}({NetAuthority}): Primary Ability was not changed. Primary Attack Input is not set."
			, GetName(), HasAuthority() ? "SERVER" : "CLIENT");
	}
}

void APlayerControllerBase::SetSecondaryActionAbility(TSubclassOf<URsGameplayAbilityBase> AbilityReference)
{
	if (IsValid(SecondaryAttackInput))
	{
		SetHotkeyAbility(SecondaryAttackInput, AbilityReference);
	}
	else
	{
		UE_LOGFMT(LogTemp, Error, "{Name}({NetAuthority}): Secondary Ability was not changed. Secondary Attack Input is not set."
			, GetName(), HasAuthority() ? "SERVER" : "CLIENT");
	}
}

void APlayerControllerBase::RespawnAtGraveyard()
{
	if (!HasAuthority())
	{
		Server_RespawnAtGraveyard();
		return;
	}
	RespawnPawn(TAG_Respawners_Graveyard.GetTag());
}
void APlayerControllerBase::Server_RespawnAtGraveyard_Implementation()
{
	RespawnAtGraveyard();
}


void APlayerControllerBase::RespawnAtCorpse()
{
	if (!HasAuthority())
	{
		Server_RespawnAtCorpse();
		return;
	}

	ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetCharacter());
	if (IsValid(CharacterBase))
	{
		CharacterBase->Respawn();
	}
}
void APlayerControllerBase::Server_RespawnAtCorpse_Implementation()
{
	RespawnAtCorpse();
}

void APlayerControllerBase::RespawnAtEntrance()
{
	if (!HasAuthority())
	{
		Server_RespawnAtEntrance();
		return;
	}
	RespawnPawn(TAG_Respawners_Entrance.GetTag());
}
void APlayerControllerBase::Server_RespawnAtEntrance_Implementation()
{
	RespawnAtEntrance();
}


void APlayerControllerBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}


void APlayerControllerBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlayerControllerBase, HotkeyAbilityMap, COND_OwnerOnly);
}
