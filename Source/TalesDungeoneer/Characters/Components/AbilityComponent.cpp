
#include "AbilityComponent.h"

#include "AiController.h"
#include "../CharacterBase.h"
#include "GameFramework/GameSession.h"
#include "Kismet/KismetMathLibrary.h"
#include "TalesDungeoneer/Entities/AbilityEffectBase.h"


UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


void UAbilityComponent::EventOnAbilityAction_Implementation(UInputAction* AbilitySlot)
{
	AbilityAction(AbilitySlot);
}


void UAbilityComponent::AbilityAction(UInputAction* HotkeyAction)
{
	if (IsValid(HotkeyAction))
	{
		for (const FStAbilityHotkey Hotkey : _AbilityActions)
		{
			if (Hotkey.AbilityInput == HotkeyAction)
			{
				if (GetOwner()->HasAuthority())
				{
					ActivateAbility(Hotkey.AbilityName, GetTargetedActor());
				}
				else
				{
					AController* PawnController = GetOwner()->GetInstigatorController();
					if (IsValid(PawnController))
					{
						// Is it a player?
						APlayerController* PlayerController = Cast<APlayerController>(PawnController);
						if (IsValid(PlayerController))
						{
							APlayerCameraManager* CamManager = PlayerController->PlayerCameraManager;
							if (IsValid(CamManager))
							{
								Server_RequestAbility(Hotkey.AbilityName,
									GetTargetedActor(),
									CamManager->GetActorForwardVector());
								return;
							}
						}
						
						// Is it an NPC?
						AAIController* AiController = Cast<AAIController>(PawnController);
						if (IsValid(AiController))
						{
							FVector AiFocalPoint = AiController->GetFocalPoint();
							ActivateAbility(Hotkey.AbilityName, GetTargetedActor(),
								UKismetMathLibrary::GetDirectionUnitVector(
									GetOwner()->GetActorLocation(), AiFocalPoint));
							return;
						}
						
					}
				}
				return;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): Hotkey '%s' Not Found in Ability Actions!"),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *HotkeyAction->GetName());
	}
	UE_LOG(LogTemp, Warning, TEXT("%s(%s): Invalid UInputAction given to AbilityAction()"),
		*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
}

bool UAbilityComponent::SetAbilityInputAction(FName AbilityName, UInputAction* InputAction)
{
	if (IsValid(InputAction))
	{
		if (AbilityName.IsNone())
		{
			for (FStAbilityHotkey Hotkey : _AbilityActions)
			{
				if (Hotkey.AbilityInput == InputAction)
				{
					// Un-assigns the hotkey
					Hotkey.AbilityName = FName();
					return true;
				}
			}
		}
		else
		{
			for (FStAbilityHotkey Hotkey : _AbilityActions)
			{
				if (Hotkey.AbilityInput == InputAction)
				{
					Hotkey.AbilityName = AbilityName;
					return true;
				}
			}
		}
	}
	return false;
}


void UAbilityComponent::ActivateAbility(
		const FName AbilityName, AActor* TargetActor, FVector ForwardVector)
{
	if (GetOwner()->HasAuthority())
	{
		if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
		{
			const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
			ACharacterBase* EffectInstigator = Cast<ACharacterBase>(GetOwner());
			SpawnEffectsActor(EffectInstigator, AbilityName, ForwardVector);
			
		}
	}
	else
	{
		Server_RequestAbility(AbilityName);
	}
}


void UAbilityComponent::ApplyEffect(ACharacterBase* EffectInstigator, FName AbilityName)
{
	if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
	{
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);

		// Only apply the effect if it has a duration
		if (AbilityData.EffectDuration > 0.f)
		{

			// ReSharper disable once CppLocalVariableMayBeConst
			FStAbilityEffect AbilityEffect = FStAbilityEffect(AbilityData);
			
			// Lock against reading from the array
			// Releases lock automatically when scope is lost
			FRWScopeLock WriteLock(_MutexLock, SLT_Write);

			// Add the effect to the appropriate key in the active effects map
			_ActiveEffects[AbilityName].Add(AbilityEffect);
			
			UE_LOG(LogTemp, Warning, TEXT("%s(%s): Added New Effect: %s"),
				*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"),
				*AbilityName.ToString());
		}

		// If the timer isn't valid, initiate it (AKA this is the first effect)
		if (!_EffectsTimer.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s(%s): Starting Effects Timer"),
				*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
			GetWorld()->GetTimerManager().SetTimer(_EffectsTimer, this,
				&UAbilityComponent::OnTickTimer, TimerRate, true);
		}
		
	}
}

void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
#ifdef UE_BUILD_DEBUG
	bShowDebug = true;
#endif
}

void UAbilityComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	
	SetAutoActivate(true);
	SetIsReplicated(true);
	
	RegisterComponent();
}

void UAbilityComponent::SpawnEffectsActor(
	ACharacterBase* EffectInstigator, FName AbilityName, FVector ForwardVector)
{

	if (!UAbilitySystem::GetAbilityNameIsValid(AbilityName) || !IsValid(EffectInstigator))
		return;
	
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	FTransform SpawnTransform(EffectInstigator->GetActorTransform());
	SpawnTransform.SetScale3D(FVector(1.f));
		
	// Spawns the ability effect actor
	AAbilityEffectBase* AbilityEffect = GetWorld()->SpawnActorDeferred<AAbilityEffectBase>(
			AbilityData.SpawnActor, SpawnTransform);
	
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display, TEXT("%s(%s): AbilityEffect->SpawnActorDeferred()"), *GetName(),
			GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	if (IsValid(AbilityEffect))
	{
		AbilityEffect->SetAbilityName(AbilityName);

		if (IsValid(EffectInstigator) && !AbilityData.AttachBoneOnSpawn.IsNone())
		{
			USkeletalMeshComponent* SkeletalMesh = EffectInstigator->GetMesh();
			AbilityEffect->AttachToComponent(SkeletalMesh,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale, AbilityData.AttachBoneOnSpawn);
		}
				
		AbilityEffect->FinishSpawning(SpawnTransform);
		UE_LOG(LogTemp, Display, TEXT("%s(%s): AbilityEffect->FinishSpawning()"), *GetName(),
			GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
}

void UAbilityComponent::TickTimer()
{
	// Lock against reading from the array
	// Releases lock automatically when scope is lost
	FRWScopeLock WriteLock(_MutexLock, SLT_Write);

	// We want to manipulate the existing entries, not copy them.
	// So we will iterate the location in memory, and utilize pointers.
	for (auto &[EffectName, ArrayOfEffects] : _ActiveEffects)
	{
		// Iterate through all active effects of this name
		for (int i = ArrayOfEffects.Num() - 1; i >= 0; i++)
		{
			// Obtain a reference, for cleaner code
			FStAbilityEffect* AbilityData = &ArrayOfEffects[i];
			
			// Only reduce timer if concurrent tick, or is topmost index of array
			if (AbilityData->bTicksConcurrently || i == 0)
			{
				AbilityData->TimeRemaining -= TimerRate;
				if (AbilityData->TimeRemaining < 0.f)
				{
					// Remove this entry if the time has expired
					const FName OldAbilityName = EffectName;
					ArrayOfEffects.RemoveAt(i);
					OnAbilityExpired.Broadcast(OldAbilityName, ArrayOfEffects.Num());
					
					UE_LOG(LogTemp, Display, TEXT("%s(%s): Effect '%s' Expired. There are %d remaining in the stack."),
						*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"),
							*OldAbilityName.ToString(), ArrayOfEffects.Num());
					
				}
			}
		}
	}

	// Save resources by invalidating the timer, if no effects are active
	if (_ActiveEffects.Num() < 1 && _EffectsTimer.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): No effects remaining. Timer invalidated."),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		_EffectsTimer.Invalidate();
	}
	
}

void UAbilityComponent::OnTickTimer_Implementation()
{
	TickTimer();
}

void UAbilityComponent::Client_AbilityAdded_Implementation(FName AbilityName, int StackCount)
{
	// Trigger Delegates, such as the HUD
	OnAbilityActivated.Broadcast(AbilityName, StackCount);
}

void UAbilityComponent::Client_AbilityExpired_Implementation(FName AbilityName, int StackCount)
{
	// Trigger Delegates, such as the HUD
	OnAbilityExpired.Broadcast(AbilityName, StackCount);
}

void UAbilityComponent::Server_RequestAbility_Implementation(
	FName AbilityName, AActor* TargetActor, FVector ForwardVector)
{
	if (GetOwner()->HasAuthority())
	{
		ActivateAbility(AbilityName, TargetActor, ForwardVector);
	}
}

//-------------------------------- REPLICATION
void UAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}