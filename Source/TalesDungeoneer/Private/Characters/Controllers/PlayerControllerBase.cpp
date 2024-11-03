// Starcache Studios, LLC (c) 2024


#include "Characters/Controllers/PlayerControllerBase.h"

#include "Abilities/RsGameplayAbilityBase.h"
#include "Actors/TalesRespawnBase.h"
#include "Characters/CharacterBase.h"
#include "Kismet/GameplayStatics.h"

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

		// Bind hotkey assignments, whether they map to a real ability or not
		for (auto& HotkeyPair : HotkeyAbilityMap)
		{
			if (IsValid(HotkeyPair.Key))
			{
				EnhancedInputComponent->BindAction(
					HotkeyPair.Key, ETriggerEvent::Started, this, &APlayerControllerBase::HotkeyActionDelegate, HotkeyPair.Key, true, false);
				EnhancedInputComponent->BindAction(
					HotkeyPair.Key, ETriggerEvent::Canceled, this, &APlayerControllerBase::HotkeyActionDelegate, HotkeyPair.Key, false, true);
				EnhancedInputComponent->BindAction(
					HotkeyPair.Key, ETriggerEvent::Completed, this, &APlayerControllerBase::HotkeyActionDelegate, HotkeyPair.Key, false, false);
			}
		}


	}
}

void APlayerControllerBase::HotkeyActionDelegate(
	const FInputActionValue& InputValue, UInputAction* InputAction, const bool bStarted, const bool bCanceled)
{
	if (!IsValid(GetPawn()))
	{
		return;
	}

	UActorComponent* ActorComponent = GetPawn()->GetComponentByClass(URsAbilityComponent::StaticClass());
	URsAbilityComponent* AbilityComponent = Cast<URsAbilityComponent>(ActorComponent);
	if (IsValid(AbilityComponent))
	{
		const TSubclassOf<URsGameplayAbilityBase> AbilityClass = *HotkeyAbilityMap.Find(InputAction);
		if (IsValid(AbilityClass))
		{
			AbilityComponent->HotkeyAbility(InputValue, AbilityClass, bStarted, bCanceled);
		}
	}
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

/*
void APlayerControllerBase::PrimaryHotkeyDelegate(const FInputActionValue& InputValue)
{
	if (!IsValid(GetPawn()))
	{
		return;
	}
	URsAbilityComponent* AbilityComponent = Cast<URsAbilityComponent>
		(GetPawn()->GetComponentByClass(URsAbilityComponent::StaticClass()));
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->PrimaryAbility(InputValue);
	}
}

void APlayerControllerBase::SecondaryHotkeyDelegate(const FInputActionValue& InputValue, const bool bStarted,
	const bool bCanceled)
{
	if (!IsValid(GetPawn()))
	{
		return;
	}
	URsAbilityComponent* AbilityComponent = Cast<URsAbilityComponent>
		(GetPawn()->GetComponentByClass(URsAbilityComponent::StaticClass()));
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->SecondaryAbility(bStarted, bCanceled);
	}
}
*/

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

void APlayerControllerBase::HotkeyTarget(UInputAction* HotkeyAction)
{
	if (IsValid(HotkeyAction))
	{
		const ACharacterBase* ControlledCharacter = Cast<ACharacterBase>( GetCharacter() );
		if (IsValid(ControlledCharacter))
		{
		}
	}
}
