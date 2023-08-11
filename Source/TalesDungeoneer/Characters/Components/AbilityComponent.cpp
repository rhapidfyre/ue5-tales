 
#include "AbilityComponent.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "../CharacterBase.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Entities/AbilityEffectBase.h"


UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
	bReplicateUsingRegisteredSubObjectList = true;
}


void UAbilityComponent::EventOnAbilityAction_Implementation(UInputAction* AbilitySlot)
{
	AbilityAction(AbilitySlot);
}

void UAbilityComponent::AbilityAction(UInputAction* HotkeyAction)
{
	if (IsValid(HotkeyAction))
	{
		if (_AbilityMappings.Contains(HotkeyAction))
		{
			const FName AbilityName = _AbilityMappings[HotkeyAction];
			if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
			{
				ActivateAbility(AbilityName, GetTargetedActor());
				return;
			}
		}
		
		else if (_TargetMappings.Contains(HotkeyAction))
		{
			
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
		_AbilityMappings.Add(InputAction, AbilityName);
		OnAbilityHotkeyChanged.Broadcast(InputAction, AbilityName);
		return true;
	}
	return false;
}


void UAbilityComponent::ActivateAbility(
		const FName AbilityName, AActor* TargetActor, FVector ForwardVector)
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	
	if (GetOwner()->HasAuthority())
	{
		if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
		{
			
			if (GetIsAbilityOnCooldown(AbilityName))
			{
				return;
			}
			
			// Caster is focused and can't use any other abilities
			if (!FocusedAbility.IsNone())
			{
				Client_AbilityCanceled(AbilityName, "You are already focusing on something else!");
				return;
			}

			// Actor is casting and can't focus on the new ability
			if (bIsCasting && AbilityData.bRequiresFocus)
			{
				Client_AbilityFailure(AbilityName, "This ability requires focus!");
				return;
			}

			if (AbilityData.TargetType == EAbilityTarget::TARGET && !IsValid(GetTargetedActor()))
			{
				Client_AbilityFailure(AbilityName, "No Target");
				return;
			}
			
			ACharacterBase* EffectInstigator = Cast<ACharacterBase>(GetOwner());
			if (IsValid(TargetActor))
			{
				_TargetActor = Cast<ACharacterBase>( TargetActor );
			}
			SpawnEffectsActor(EffectInstigator, AbilityName, ForwardVector);
		}
	}
	else
	{
			
		if (GetIsAbilityOnCooldown(AbilityName))
		{
			return;
		}

		// Caster is focused and can't use any other abilities
		if (!FocusedAbility.IsNone())
		{
			OnAbilityCanceled.Broadcast(AbilityName, "Already Focusing On " + AbilityData.DisplayName);
			return;
		}

		// Actor is casting and can't focus on the new ability
		if (bIsCasting && AbilityData.bRequiresFocus)
		{
			OnAbilityFailed.Broadcast(AbilityName, "This Ability Requires Focus!");
			return;
		}

		if (AbilityData.TargetType == EAbilityTarget::TARGET && !IsValid(GetTargetedActor()))
		{
			OnAbilityFailed.Broadcast(AbilityName, "No Target");
			return;
		}
		
		StartCasting(AbilityName);
		Server_RequestAbility(AbilityName, TargetActor, ForwardVector);
		
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
			UStatusEffect* AbilityEffect = NewObject<UStatusEffect>(
						GetOwner(), UStatusEffect::StaticClass());
			AddReplicatedSubObject(AbilityEffect);
			AbilityEffect->OnEffectExpired.AddDynamic(this, &UAbilityComponent::RemoveExpiredEffect);
			AbilityEffect->SetAbilityName(AbilityName);
			AbilityEffect->SetEffectInstigator(EffectInstigator);
			AbilityEffect->InitializeEffect();
			
			// Lock against reading from the array
			// Releases lock automatically when scope is lost
			FRWScopeLock WriteLock(_MutexLock, SLT_Write);

			// Add the effect to the appropriate key in the active effects map	
			_ActiveEffects.Add(AbilityEffect);
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

void UAbilityComponent::SetTargetedActorByHotkey(UInputAction* TargetHotkey)
{
	if (IsValid(TargetHotkey))
	{
		if (_TargetMappings.Contains(TargetHotkey))
		{
			
			ACharacterBase* SelfActor = Cast<ACharacterBase>(GetOwner());
			ACharacterBase* NearestActor = nullptr;
			float NearestDistance = -1;
			
			TArray<AActor*> NearbyActors;
			ECharacterTeam MatchingTeam = ECharacterTeam::DUNGEONEER;
			
			switch(_TargetMappings[TargetHotkey])
			{
			case ETargetingOption::ONE:
				SetTargetedActor( SelfActor );
				return;
			case ETargetingOption::TWO:
				//SetTargetedActor(  );
				return;
			case ETargetingOption::THREE:
				//SetTargetedActor(  );
				return;
			case ETargetingOption::FOUR:
				//SetTargetedActor(  );
				return;
			case ETargetingOption::FIVE:
				//SetTargetedActor(  );
				return;
			case ETargetingOption::NEAR_ENEMY:
				MatchingTeam = ECharacterTeam::ENEMY;
				break;
			case ETargetingOption::NEAR_PARTY:
				MatchingTeam = ECharacterTeam::PLAYER;
				break;
			case ETargetingOption::NEAR_NPC:
				MatchingTeam = ECharacterTeam::FRIEND;
				break;
			default:
				return;
			}

			// Get all game mode characters
			UGameplayStatics::GetAllActorsOfClass(GetWorld(),
				ACharacterBase::StaticClass(), NearbyActors);

			// Find the nearest one matching the filter
			for (int i = NearbyActors.Num() - 1; i >= 0; i--)
			{
				ACharacterBase* TargetCharacter = Cast<ACharacterBase>(NearbyActors[i]);
				if (TargetCharacter->GetCharacterTeam() == MatchingTeam)
				{
					float NewDistance = SelfActor->GetDistanceTo(TargetCharacter);
					if (NewDistance < NearestDistance || NearestDistance < 0)
					{
						NearestDistance = NewDistance;
						NearestActor = TargetCharacter;
					}
				}
			}
			
			if (IsValid(NearestActor))
			{
				SetTargetedActor(NearestActor);
			}
			
		}
	}
}

void UAbilityComponent::SetTargetedActor(ACharacterBase* NewTarget)
{
	if (GetOwner()->HasAuthority())
	{
		_TargetActor = NewTarget;
		OnNewTargetSet.Broadcast(NewTarget);
	}
	else
	{
		Server_RequestTarget(NewTarget);
	}
}

/**
 * @brief Called when an effect has expired and needs to be removed from the array.
 * @param AbilityEffect The UObject to remove from the TArray
 * @param AbilityName The name of the ability being removed
 */
void UAbilityComponent::RemoveExpiredEffect(UStatusEffect* AbilityEffect, FName AbilityName)
{
	if ( IsValid(AbilityEffect) && (_ActiveEffects.Contains(AbilityEffect)) )
	{
		RemoveReplicatedSubObject(AbilityEffect);
		_ActiveEffects.RemoveSingle(AbilityEffect);
	}
}

int UAbilityComponent::GetNumStacksActive(FName AbilityName)
{
	int NumStacks = 0;
	FRWScopeLock ReadLock(_MutexLock, SLT_ReadOnly);
	for (UStatusEffect* AbilityEffect : _ActiveEffects)
	{
		if (AbilityEffect->GetAbilityName() == AbilityName)
			NumStacks += 1;
	}
	return NumStacks;
}

bool UAbilityComponent::GetIsEffectActiveByName(FName AbilityName)
{
	for (const UStatusEffect* AbilityEffect : _ActiveEffects)
	{
		if (AbilityEffect->GetAbilityName() == AbilityName)
			return true;
	}
	return false;
}

/**
 * @brief Returns the ability effect that has the least amount of time left
 * @param AbilityName Optional - If valid, returns the effect of this type with the lowest time left
 * @return Object Pointer with the lowest timer, or nullptr if effect is not active
 */
UStatusEffect* UAbilityComponent::GetEffectWithLowestTimer(FName AbilityName)
{
	UStatusEffect* ReturnPointer = nullptr;
	float LowestTimeRemaining = -1.f;
	const bool UseAbilityName = !AbilityName.IsNone();
	FRWScopeLock ReadLock(_MutexLock, SLT_ReadOnly);
	for (UStatusEffect* AbilityEffect : _ActiveEffects)
	{
		// If using ability name, make sure it matches
		if (IsValid(AbilityEffect))
		{
			if (!UseAbilityName || (UseAbilityName && AbilityName == AbilityEffect->GetAbilityName()))
			{
				if (AbilityEffect->GetSecondsRemaining() < LowestTimeRemaining || LowestTimeRemaining < 0.f)
				{
					ReturnPointer		= AbilityEffect;
					LowestTimeRemaining = AbilityEffect->GetSecondsRemaining();
				}
			}
		}
	}
	return ReturnPointer;
}

/**
 * @brief 
 * @param AbilityName Optional - If valid, returns effect of this type with highest time left
 * @return Object with the highest timer, or nullptr if effect is not active
 */
UStatusEffect* UAbilityComponent::GetEffectWithGreatestTimer(FName AbilityName)
{
	UStatusEffect* ReturnPointer = nullptr;
	float GreatestTimeRemaining = -1.f;
	const bool UseAbilityName = !AbilityName.IsNone();
	FRWScopeLock ReadLock(_MutexLock, SLT_ReadOnly);
	for (UStatusEffect* AbilityEffect : _ActiveEffects)
	{
		// If using ability name, make sure it matches
		if (IsValid(AbilityEffect))
		{
			if (!UseAbilityName || (UseAbilityName && AbilityName == AbilityEffect->GetAbilityName()))
			{
				if (AbilityEffect->GetSecondsRemaining() > GreatestTimeRemaining || GreatestTimeRemaining < 0.f)
				{
					ReturnPointer		= AbilityEffect;
					GreatestTimeRemaining = AbilityEffect->GetSecondsRemaining();
				}
			}
		}
	}
	return ReturnPointer;
}

/**
 * @brief Calculates the total seconds remaining, accounting for concurrent timers.
 * @param AbilityName Optional - The ability name to consider when calculating
 * @return 
 */
float UAbilityComponent::GetTotalEffectStackTimer(FName AbilityName)
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	float TotalSecondsRemaining = 0.f;
	const bool UseAbilityName = !AbilityName.IsNone();
	FRWScopeLock ReadLock(_MutexLock, SLT_ReadOnly);

	// If timers run consecutively and not concurrently, return the lowest timer
	if (UseAbilityName && !AbilityData.bTickIndependently)
	{
		const UStatusEffect* AbilityEffect = GetEffectWithLowestTimer(AbilityName);
		if (IsValid(AbilityEffect))
			return AbilityEffect->GetSecondsRemaining();
	}
	
	for (int i = 1; i < _ActiveEffects.Num() ; i++)
	{
		const UStatusEffect* AbilityEffect = _ActiveEffects[i];
		
		// If using ability name, make sure it matches
		if (IsValid(AbilityEffect))
		{
			
			if (!UseAbilityName || (UseAbilityName && AbilityName == AbilityEffect->GetAbilityName()))
			{
				if (AbilityEffect->DoesTimerTickIndependently())
					TotalSecondsRemaining += AbilityEffect->GetSecondsRemaining();
			}
		}
	}
	return TotalSecondsRemaining;
}

void UAbilityComponent::EndAbilityCooldown(FName AbilityName)
{
	if (_AbilitiesOnCooldown.Contains(AbilityName))
		_AbilitiesOnCooldown.Remove(AbilityName);
	
	if (GetOwner()->HasAuthority())
	{
		OnAbilityReady.Broadcast(AbilityName);
		Client_AbilityCooldown(AbilityName, false);
	}
	
}

void UAbilityComponent::Server_InterruptCasting_Implementation(bool OnlyFocused, bool CanceledIntentionally)
{
	InterruptCasting(OnlyFocused, CanceledIntentionally);
}

void UAbilityComponent::InterruptCasting(bool OnlyFocused, bool CanceledIntentionally)
{
	if ( GetIsCasting() )
	{
		// Caster is focused, OR cancel ALL abilities
		if (!FocusedAbility.IsNone() || !OnlyFocused)
		{
			if (GetOwner()->HasAuthority())
			{
				if (CanceledIntentionally)
					Client_AbilityCanceled(FocusedAbility, "Canceled by Player");
				else
					Client_AbilityFailure(FocusedAbility, "Lost Concentration!");
				if (OnlyFocused)
				{
					TArray<AActor*> EffectActors;
					UGameplayStatics::GetAllActorsOfClass(GetWorld(),
						AAbilityEffectBase::StaticClass(), EffectActors);
					for (AActor* EffectActor : EffectActors)
					{
						AAbilityEffectBase* EffectBase = Cast<AAbilityEffectBase>(EffectActor);
						if (IsValid(EffectBase))
						{
							EffectBase->GetAbilityName() == FocusedAbility;
							_AbilitiesInProgress.Remove(FocusedAbility);
							EffectBase->Destroy();
						}
					}
				}
				else
				{
					TArray<AActor*> EffectActors;
					UGameplayStatics::GetAllActorsOfClass(GetWorld(),
						AAbilityEffectBase::StaticClass(), EffectActors);
					for (AActor* EffectActor : EffectActors)
					{
						AAbilityEffectBase* EffectBase = Cast<AAbilityEffectBase>(EffectActor);
						if (IsValid(EffectBase))
						{
							FName AbilityName = EffectBase->GetAbilityName();
							_AbilitiesInProgress.Remove(AbilityName);
							EffectBase->Destroy();
						}
					}
				}
				SetIsCasting(FName());
				FocusedAbility = FName();
			}
			else
			{
				if (CanceledIntentionally)
					OnAbilityCanceled.Broadcast(FocusedAbility, "Canceled by Player");
				else
					OnAbilityFailed.Broadcast(FocusedAbility, "Lost Concentration!");
				Server_InterruptCasting(OnlyFocused, CanceledIntentionally);
			}
		}
	}
}

void UAbilityComponent::Server_RequestAbilityAdd_Implementation(FName AbilityName)
{
	if (GetOwner()->HasAuthority())
	{
		if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
		{
			// TODO - Restrictions, allowed classes, etc.
			const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
			AddKnownAbility(AbilityName, AbilityData.UnlockPoints);
		}
	}
}

void UAbilityComponent::Server_RequestAbilityRemove_Implementation(FName AbilityName)
{
	if (GetOwner()->HasAuthority())
	{
		if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
		{
			RemoveKnownAbility(AbilityName);
		}
	}
}

void UAbilityComponent::Server_RequestAbilityReset_Implementation()
{
	if (GetOwner()->HasAuthority())
	{
		ResetKnownAbilities();
	}
}

void UAbilityComponent::AddUnlockPoints(int NumPoints)
{
	if (GetOwner()->HasAuthority())
	{
		_UnlockPoints += abs(NumPoints);
		OnUnlockPointsChanged.Broadcast(_UnlockPoints);
	}
}

void UAbilityComponent::RemoveUnlockPoints(int NumPoints)
{
	if (GetOwner()->HasAuthority())
	{
		const int NewTotal = _UnlockPoints -= abs(NumPoints);
		_UnlockPoints = NewTotal > 0 ? NewTotal : 0;
		OnUnlockPointsChanged.Broadcast(_UnlockPoints);
	}
}

void UAbilityComponent::BeginPlay()
{
#ifdef UE_BUILD_DEBUG
	bShowDebug = true;
#endif
	Super::BeginPlay();
	_PlayerCharacter = Cast<ACharacterBase>(GetOwner());
	if (IsValid(_PlayerCharacter))
	{
		SetComponentTickEnabled(true);
	}
}

void UAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!FocusedAbility.IsNone())
	{
		FVector PlayerVelocity = _PlayerCharacter->GetMovementComponent()->Velocity;
		if (!PlayerVelocity.Equals(FVector(0.f), 1.f))
		{
			InterruptCasting(true, false);
		}
	}
}

void UAbilityComponent::DestroyAllEffects()
{
	AActor* MyOwner = GetOwner();
	checkf(IsValid(MyOwner), TEXT("DestroySlot:: Invalid Inventory Owner"));
	checkf(MyOwner->HasAuthority(), TEXT("DestroySlot:: Called without Authority!"));
	for (UStatusEffect* AbilityEffect : _ActiveEffects)
	{
		if (IsValid(AbilityEffect))
		{
			AbilityEffect->ConditionalBeginDestroy();
		}
	}
	_ActiveEffects.Empty();
}

void UAbilityComponent::OnUnregister()
{
	const AActor* MyOwner = GetOwner();
	if (IsValid(MyOwner) && MyOwner->HasAuthority())
		DestroyAllEffects();
	Super::OnUnregister();
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
	
	// Is the ability valid
	if (!UAbilitySystem::GetAbilityNameIsValid(AbilityName) || !IsValid(EffectInstigator))
	{
		SetIsCasting(FName());

		if (GetOwner()->HasAuthority())
		{
			if (!IsValid(EffectInstigator))
				Client_AbilityCanceled(AbilityName, "Invalid Ability Name");
			else
				Client_AbilityCanceled(AbilityName, "Invalid Instigator");
		}
		else
			OnAbilityCanceled.Broadcast(AbilityName, "Invalid Ability Name or Instigator");

		return;
		
	}

	// Validate the casting request
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);

	// Targeted spells must have a target
	if (   AbilityData.TargetType == EAbilityTarget::TARGET
		|| AbilityData.TargetType == EAbilityTarget::AOE)
	{
		if (!IsValid( GetTargetedActor() ))
		{
			if (GetOwner()->HasAuthority())
				Client_AbilityCanceled(AbilityName, "No Target Selected");
			else
				OnAbilityCanceled.Broadcast(AbilityName, "No Target Selected");
			return;
		}
	}

	if (_AbilitiesInProgress.Contains(AbilityName))
	{
		if (GetOwner()->HasAuthority())
			Client_AbilityCanceled(AbilityName, "Already Casting");
		else
			OnAbilityCanceled.Broadcast(AbilityName, "Already in Progress");
		return;
	}
	
	FTransform SpawnTransform(EffectInstigator->GetActorTransform());
	SpawnTransform.SetScale3D(FVector(1.f));

	const FVector EndPosition = SpawnTransform.GetLocation() + (ForwardVector * AbilityData.MaxRange);

	// Spawns the ability effect actor
	TSubclassOf<AAbilityEffectBase> AbilityBase = AAbilityEffectBase::StaticClass();

	if (IsValid(AbilityData.AbilityBase))
		AbilityBase = AbilityData.AbilityBase;

	FocusedAbility = AbilityData.bRequiresFocus ? AbilityName : FName();
	
	AAbilityEffectBase* AbilityEffect = GetWorld()->
				SpawnActorDeferred<AAbilityEffectBase>(AbilityBase, SpawnTransform);
	
	if (IsValid(AbilityEffect))
	{
		StartCasting(AbilityName);

		// Deductions for starting to cast
		EffectInstigator->VitalityComponent->DamageHealth(EffectInstigator,   AbilityData.ConsumeHealth);
		EffectInstigator->VitalityComponent->ConsumeMagic(EffectInstigator,   AbilityData.ConsumeMagic);
		EffectInstigator->VitalityComponent->ConsumeStamina(EffectInstigator, AbilityData.ConsumeStamina);
		
		AbilityEffect->SetInstigator( GetOwner()->GetInstigator() );
		AbilityEffect->SetAbilityComponent( this );
		AbilityEffect->SetAbilityInstigator( Cast<ACharacterBase>(GetOwner()) );
		AbilityEffect->SetAbilityName(AbilityName);
		
		AbilityEffect->OnAbilityFinished.AddDynamic(this,
			&UAbilityComponent::SetNoLongerCasting);
		
		AbilityEffect->SetImpactLocation( GetOwner()->GetActorLocation() );

		FHitResult HitResult;
		switch(AbilityData.TargetType)
		{
		case EAbilityTarget::PROJECTILE:
			if (GetWorld()->LineTraceSingleByChannel(HitResult,
				AbilityEffect->GetActorLocation(), EndPosition,ECC_Visibility))
				AbilityEffect->SetImpactLocation(HitResult.ImpactPoint);
			else
				AbilityEffect->SetImpactLocation(EndPosition);
			break;
		case EAbilityTarget::GROUP:
			__fallthrough;
		case EAbilityTarget::NEAR:
			__fallthrough;
		case EAbilityTarget::SELF:
			AbilityEffect->SetTargetActor( EffectInstigator );
			break;
		case EAbilityTarget::AOE:
			__fallthrough;
		case EAbilityTarget::TARGET:
			AbilityEffect->SetTargetActor( Cast<ACharacterBase>(GetTargetedActor()) );
			break;
		default:
			break;
		}

		// Finish spawning and set initialized to true
		AbilityEffect->FinishSpawning(SpawnTransform);
		
		if (IsValid(EffectInstigator) && !AbilityData.SpawnBone.IsNone())
		{
			USkeletalMeshComponent* SkeletalMesh = EffectInstigator->GetMesh();
			AbilityEffect->AttachToComponent(SkeletalMesh,
				FAttachmentTransformRules::SnapToTargetIncludingScale, AbilityData.SpawnBone);
		}
				
		UE_LOG(LogTemp, Display, TEXT("%s(%s): AbilityEffect->FinishSpawning()"), *GetName(),
			GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		
	}
}

void UAbilityComponent::CancelCasting(FName AbilityName)
{
	if (GetOwner()->HasAuthority())
	{
		if (_AbilitiesInProgress.Contains(AbilityName))
		{
			StopCasting(AbilityName, false);
		}
	}
	else
	{
		Server_CancelCasting(AbilityName);
	}
}

void UAbilityComponent::Server_CancelCasting_Implementation(FName AbilityName)
{
	if (GetOwner()->HasAuthority())
		CancelCasting(AbilityName);
}

void UAbilityComponent::Multicast_StopCasting_Implementation(FName AbilityName, bool WasSuccessful)
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);

	if (AbilityData.bRequiresFocus)
	{
		// If this character is us
		if (GetOwner()->GetInstigatorController() == GetWorld()->GetFirstPlayerController())
		{
			FocusedAbility = FName();
		}
	}
	
	// Determine Success/Fail Animation
	UAnimMontage* AnimMontage = AbilityData.AnimationData.AnimationOnFail;
	if (WasSuccessful)
		AnimMontage = AbilityData.AnimationData.AnimationOnSuccess;
	
	if (IsValid(AnimMontage))
	{
		ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
		if (IsValid(CharacterBase))
		{
			CharacterBase->StopAnimMontage(nullptr);
			CharacterBase->PlayAnimMontage(AnimMontage);
		}
	}

	// If effect was successful, run 'EffectsComplete'
	TArray<FStAbilityFx> AbilityEffects = AbilityData.EffectFailed;
	if (WasSuccessful)
		AbilityEffects = AbilityData.EffectComplete;

	// Determine Attachment Requirements
	const ACharacter* ThisCharacter = Cast<ACharacter>(GetOwner());
	if (!IsValid(ThisCharacter))
		return;
	
	USceneComponent* AttachComp = GetOwner()->GetRootComponent();
	USceneComponent* SkelComp = ThisCharacter->GetMesh();
	if (!IsValid(SkelComp))
		SkelComp = AttachComp;
	
	// Apply Visual Effects
	for (FStAbilityFx AbilityFx : AbilityEffects)
	{
		FName AttachBone = AbilityFx.NiagaraBone;
	
		// Dispatch Audio Effect
		if (IsValid(AbilityFx.SoundEffect))
		{
			
			UAudioComponent* SoundEffect = UGameplayStatics::SpawnSoundAttached(
				AbilityFx.SoundEffect, AbilityFx.bAttachSound ? SkelComp : AttachComp, AttachBone,
				AttachComp->GetSocketLocation(AttachBone) + AbilityFx.EffectOffset,
				AttachComp->GetSocketRotation(AttachBone) + AbilityFx.EffectRotation,
				EAttachLocation::SnapToTargetIncludingScale, true);
			if (IsValid(SoundEffect))
			{
				SoundEffect->bAutoDestroy = true;
				SoundEffect->Play();
			}
		}

		// Dispatch Niagara Effect
		if (IsValid(AbilityFx.NiagaraEffect))
		{
			if (AbilityFx.bAttachNiagaraToActor || AbilityFx.bAttachNiagaraToSkeleton)
			{
				UNiagaraComponent* NiagaraSys = UNiagaraFunctionLibrary::SpawnSystemAttached(AbilityFx.NiagaraEffect,
					AbilityFx.bAttachNiagaraToSkeleton ? SkelComp : AttachComp, AttachBone, 
					FVector::ZeroVector + AbilityFx.EffectOffset,
					FRotator::ZeroRotator + AbilityFx.EffectRotation,
					EAttachLocation::SnapToTargetIncludingScale,
					AbilityFx.NiagaraLoopTime > 0.f, true);
		
				if (IsValid(NiagaraSys))
				{
					NiagaraSys->SetRelativeScale3D( FVector(AbilityFx.EffectScale) );
				}
			}
			else
			{
				UNiagaraComponent* NiagaraSys = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
				   AbilityFx.NiagaraEffect,
				   GetOwner()->GetActorLocation() + AbilityFx.EffectOffset,
				   GetOwner()->GetActorRotation() + AbilityFx.EffectRotation,
				   FVector(AbilityFx.EffectScale), true, true);
			}
		}
	}
	
}

void UAbilityComponent::SetIsCasting(FName SpellName)
{
	bIsCasting = !SpellName.IsNone();
}

void UAbilityComponent::SetNoLongerCasting(FName AbilityName, bool WasSuccessful)
{
	_AbilitiesInProgress.Remove(AbilityName);
	if (GetOwner()->HasAuthority())
	{
		
		bIsCasting = false;
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
		
		
		if (AbilityData.bRequiresFocus)
			FocusedAbility = FName();
		
		_AbilitiesOnCooldown.Add(AbilityName);
		Client_AbilityCooldown(AbilityName, true);
		
		FTimerHandle ThrowAwayTimer;
		FTimerDelegate CooldownDelegate;
		CooldownDelegate.BindUObject(this, &UAbilityComponent::EndAbilityCooldown, AbilityName);
		GetWorld()->GetTimerManager().SetTimer(ThrowAwayTimer,
			CooldownDelegate, AbilityData.CooldownSeconds, false);
		
		Multicast_StopCasting(AbilityName, WasSuccessful);
		Client_StopCasting(AbilityName, WasSuccessful);
	}
	
}

void UAbilityComponent::OnRep_UnlockPoints_Implementation()
{
	OnUnlockPointsChanged.Broadcast(_UnlockPoints);
}

void UAbilityComponent::Server_RequestTarget_Implementation(ACharacterBase* NewTarget)
{
	SetTargetedActor(NewTarget);
}

void UAbilityComponent::Client_AbilityFailure_Implementation(
					FName AbilityName, const FString& FailureReason)
{
	OnAbilityFailed.Broadcast(AbilityName, *FailureReason);
}

void UAbilityComponent::Client_AbilityCanceled_Implementation(FName AbilityName, const FString& CancelReason)
{
	FocusedAbility = FName();
	OnAbilityCanceled.Broadcast(AbilityName, *CancelReason);
}

void UAbilityComponent::OnRep_TargetActor_Implementation()
{
	OnNewTargetSet.Broadcast(_TargetActor);
}

void UAbilityComponent::TickTimer()
{
	// Lock against reading from the array
	// Releases lock automatically when scope is lost
	FRWScopeLock WriteLock(_MutexLock, SLT_Write);
	
	// Iterate through all active effects
	TMap<FName, int> AbilitiesRemoved = {}; 
	for (UStatusEffect* AbilityEffect : _ActiveEffects)
	{
		if (AbilityEffect->GetSecondsRemaining() < 0.f)
		{
			const FName OldAbilityName = AbilityEffect->GetAbilityName();
			if (AbilitiesRemoved.Contains(OldAbilityName))
				AbilitiesRemoved[OldAbilityName] += 1;
			else
				AbilitiesRemoved.Add(AbilityEffect->GetAbilityName(), 1);
			
			// Deregister object or garbage collector will null exception
			RemoveReplicatedSubObject(AbilityEffect);
			_ActiveEffects.RemoveSingle(AbilityEffect);
		}
	}

	// For each ability that expired, update
	for (auto &[AbilityName, StackCount] : AbilitiesRemoved)
	{
		OnAbilityRemoved.Broadcast(AbilityName, StackCount);
	}
	
	// Save resources by invalidating the timer, if no effects are active
	if (_ActiveEffects.Num() < 1 && _EffectsTimer.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): No active effects remain."),
			*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
		_EffectsTimer.Invalidate();
	}
}

void UAbilityComponent::AddKnownAbility(FName AbilityName, int UnlockPoints)
{
	// Allow client to run everything so the UI is smooth and fast
	if (_UnlockPoints >= UnlockPoints)
	{
		bool isAlreadySet = false;
		_KnownAbilities.Add(AbilityName, &isAlreadySet);
		if (!isAlreadySet)
		{
			RemoveUnlockPoints(UnlockPoints);
			
			// If this executed on the server, send notification to the owning client
			if (GetOwner()->HasAuthority())
			{
				OnAbilityLearned.Broadcast(AbilityName);
				Client_AddKnownAbility(AbilityName);
			}
			
			// Send request to server to do the actual exchange
			else
				Server_RequestAbilityAdd(AbilityName);
			
		}
	}
}

void UAbilityComponent::RemoveKnownAbility(FName AbilityName)
{
	if (GetOwner()->HasAuthority())
	{
		if (_KnownAbilities.Remove(AbilityName) > 0)
		{
			OnAbilityForgotten.Broadcast(AbilityName);
			Client_RemoveKnownAbility(AbilityName);
		}
	}
}

void UAbilityComponent::ResetKnownAbilities()
{
	if (GetOwner()->HasAuthority())
	{
		if (!_KnownAbilities.IsEmpty())
		{
			OnAbilitiesReset.Broadcast();
			Client_ResetKnownAbilities();
		}
	}
}

void UAbilityComponent::Client_AbilityCooldown_Implementation(FName AbilityName, bool OnCooldown)
{
	if (OnCooldown)
		_AbilitiesOnCooldown.Add(AbilityName);
	else
		EndAbilityCooldown(AbilityName);
}

void UAbilityComponent::Client_StartCasting_Implementation(FName AbilityName)
{
	if (!GetOwner()->HasAuthority())
		StartCasting(AbilityName);
}

void UAbilityComponent::Client_StopCasting_Implementation(FName AbilityName, bool WasSuccessful)
{
	if (!GetOwner()->HasAuthority())
		StopCasting(AbilityName, WasSuccessful);
}

/**
 * @brief Starts an ability casting, performing the appropriate logic.
 *        If called on the client, it just triggers OnAbilityCastStarted
 * @param AbilityName The name of the ability to start casting
 * @return True on success, false if the ability is already casting
 */
bool UAbilityComponent::StartCasting(FName AbilityName)
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);

	bool* isAlreadyInProgress = nullptr;
	_AbilitiesInProgress.Add(AbilityName, isAlreadyInProgress);
	if (isAlreadyInProgress)
		return false;
	
	if (GetOwner()->HasAuthority())
	{
		SetIsCasting(AbilityName);
		Client_StartCasting(AbilityName);
		OnAbilityCastStarted.Broadcast(AbilityName, AbilityData.ActivationTime);
	}
	else
	{
		OnAbilityCastStarted.Broadcast(AbilityName, AbilityData.ActivationTime);
	}
	return true;
}

/**
 * @brief Stops an ability casting, performing the appropriate logic.
 *        If called on the client, it just triggers OnAbilityCastComplete
 * @param AbilityName The name of the ability to start casting
 * @param WasSuccessful Whether the casting was successful or not
 * @return True on success, false if the ability was not being cast
 */
bool UAbilityComponent::StopCasting(FName AbilityName, bool WasSuccessful)
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	
	const int elementsRemoved = _AbilitiesInProgress.Remove(AbilityName);
	if (elementsRemoved < 1)
		return false;
	
	if (GetOwner()->HasAuthority())
	{
		SetIsCasting(AbilityName);
		
		_AbilitiesOnCooldown.Add(AbilityName);
		Client_AbilityCooldown(AbilityName, true);
		
		FTimerHandle ThrowAwayTimer;
		FTimerDelegate CooldownDelegate;
		CooldownDelegate.BindUObject(this, &UAbilityComponent::EndAbilityCooldown, AbilityName);
		GetWorld()->GetTimerManager().SetTimer(ThrowAwayTimer,
			CooldownDelegate, AbilityData.CooldownSeconds, false);
		
		Client_StartCasting(AbilityName);
		OnAbilityCastComplete.Broadcast(AbilityName, WasSuccessful);
	}
	else
	{
		if (WasSuccessful)
			_AbilitiesOnCooldown.Add(AbilityName);
		OnAbilityCastComplete.Broadcast(AbilityName, WasSuccessful);
	}
	return true;
}

void UAbilityComponent::Client_AddKnownAbility_Implementation(FName AbilityName)
{
	bool isAlreadySet = false;
	_KnownAbilities.Add(AbilityName, &isAlreadySet);
	if (!isAlreadySet)
		OnAbilityLearned.Broadcast(AbilityName);
}

void UAbilityComponent::Client_RemoveKnownAbility_Implementation(FName AbilityName)
{
	if (_KnownAbilities.Remove(AbilityName) > 0)
		OnAbilityForgotten.Broadcast(AbilityName);
}

void UAbilityComponent::Client_ResetKnownAbilities_Implementation()
{
	if (!_KnownAbilities.IsEmpty())
	{
		_KnownAbilities.Empty();
		OnAbilitiesReset.Broadcast();
	}
}

void UAbilityComponent::OnRep_ActiveEffectsUpdated_Implementation()
{
	OnActiveEffectsUpdated.Broadcast();
}

void UAbilityComponent::OnTickTimer_Implementation()
{
	TickTimer();
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
	
	DOREPLIFETIME_CONDITION(UAbilityComponent, bIsCasting, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UAbilityComponent, FocusedAbility, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UAbilityComponent, _UnlockPoints, COND_OwnerOnly);
	
	DOREPLIFETIME(UAbilityComponent, _ActiveEffects);
	DOREPLIFETIME(UAbilityComponent, _TargetActor);
	
}