 
#include "AbilityComponent.h"

#include "AiController.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "../CharacterBase.h"
#include "Components/AudioComponent.h"
#include "Engine/ActorChannel.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
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
				const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);

				if (GetOwner()->HasAuthority())
				{
					ActivateAbility(AbilityName, GetTargetedActor());
				}
				else
				{
					OnAbilityCastStarted.Broadcast(AbilityName, AbilityData.ActivationTime);
					Server_RequestAbility(AbilityName, GetTargetedActor());
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
		_AbilityMappings.Add(InputAction, AbilityName);
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

			// Caster is focused and can't use any other abilities
			if (bIsFocused)
			{
				Client_AbilityFailure(AbilityName, "You are already focusing on something else!");
				return;
			}

			// Actor is casting and can't focus on the new ability
			if (bIsCasting && AbilityData.bRequiresFocus)
			{
				Client_AbilityFailure(AbilityName, "This ability requires focus!");
				return;
			}
			
			ACharacterBase* EffectInstigator = Cast<ACharacterBase>(GetOwner());
			if (IsValid(TargetActor))
			{
				_TargetActor = TargetActor;
			}
			SpawnEffectsActor(EffectInstigator, AbilityName, ForwardVector);
		}
	}
	else
	{

		// Caster is focused and can't use any other abilities
		if (bIsFocused)
		{
			OnAbilityFailed.Broadcast(AbilityName, "You are already focusing on something else!");
			return;
		}

		// Actor is casting and can't focus on the new ability
		if (bIsCasting && AbilityData.bRequiresFocus)
		{
			OnAbilityFailed.Broadcast(AbilityName, "This ability requires focus!");
			return;
		}
		
		OnAbilityCastStarted.Broadcast(AbilityName, AbilityData.ActivationTime);
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
			//OnEffectActivated.Broadcast(AbilityName, _ActiveEffects[AbilityName].Num());
			//Client_AbilityAdded(AbilityName, AbilityEffect);
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

void UAbilityComponent::Server_InterruptCasting_Implementation(bool OnlyFocused)
{
	InterruptCasting(OnlyFocused);
}

void UAbilityComponent::InterruptCasting(bool OnlyFocused)
{
	if ( GetIsCasting() )
	{
		// Caster is focused, OR cancel ALL abilities
		if (bIsFocused || !OnlyFocused)
		{
			if (GetOwner()->HasAuthority())
			{
				OnAbilityCanceled.Broadcast(FName(), "Canceled by Player");
				SetIsCasting(FName());
				bIsFocused = false;
			}
			else
			{
				OnAbilityCanceled.Broadcast(FName(), "Canceled by Player");
				Server_InterruptCasting(OnlyFocused);
			}
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
	
	FTransform SpawnTransform(EffectInstigator->GetActorTransform());
	SpawnTransform.SetScale3D(FVector(1.f));

	const FVector EndPosition = SpawnTransform.GetLocation() + (ForwardVector * AbilityData.MaxRange);

	// Spawns the ability effect actor
	TSubclassOf<AAbilityEffectBase> AbilityBase = AAbilityEffectBase::StaticClass();

	if (IsValid(AbilityData.AbilityBase))
		AbilityBase = AbilityData.AbilityBase;

	bIsFocused = AbilityData.bRequiresFocus;
	
	AAbilityEffectBase* AbilityEffect = GetWorld()->
				SpawnActorDeferred<AAbilityEffectBase>(AbilityBase, SpawnTransform);
	
	if (IsValid(AbilityEffect))
	{
		SetIsCasting(AbilityName);
		OnAbilityCastStarted.Broadcast(AbilityName, AbilityData.ActivationTime);
		
		AbilityEffect->SetInstigator( GetOwner()->GetInstigator() );
		AbilityEffect->SetAbilityComponent( this );
		AbilityEffect->SetAbilityInstigator( Cast<ACharacterBase>(GetOwner()) );
		AbilityEffect->SetTargetActor( Cast<ACharacterBase>(GetTargetedActor()) );
		AbilityEffect->SetAbilityName(AbilityName);
		
		AbilityEffect->OnAbilityFinished.AddDynamic(this,
			&UAbilityComponent::SetNoLongerCasting);
		
		AbilityEffect->SetImpactLocation( GetOwner()->GetActorLocation() );

		if (AbilityData.TargetType == EAbilityTarget::PROJECTILE)
		{
			FHitResult HitResult;
			// ReSharper disable once CppTooWideScope
			const bool TraceHit = GetWorld()->LineTraceSingleByChannel(HitResult,
				AbilityEffect->GetActorLocation(), EndPosition,ECC_Visibility);

			if (TraceHit)
				AbilityEffect->SetImpactLocation(HitResult.ImpactPoint);
			else
				AbilityEffect->SetImpactLocation(EndPosition);

		}
		
		else if (AbilityData.TargetType == EAbilityTarget::SELF)
		{
			AbilityEffect->SetTargetActor( EffectInstigator );
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

void UAbilityComponent::Multicast_StopCasting_Implementation(FName AbilityName, bool WasSuccessful)
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	OnAbilityCastComplete.Broadcast(AbilityName, WasSuccessful);
	
	// Disallow on dedicated server, playable clients only
	if (GetOwner()->HasAuthority())
	{
		if (IsRunningDedicatedServer())
			return;
	}

	if (AbilityData.bRequiresFocus)
	{
		// If this character is us
		if (GetOwner()->GetInstigatorController() == GetWorld()->GetFirstPlayerController())
		{
			bIsFocused = false;
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
	if (bIsCasting)
	{
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(SpellName);
		OnAbilityCastStarted.Broadcast(SpellName, AbilityData.ActivationTime);
	}
}

void UAbilityComponent::SetNoLongerCasting(FName AbilityName, bool WasSuccessful)
{
	if (GetOwner()->HasAuthority())
	{
		Multicast_StopCasting(AbilityName, WasSuccessful);
		bIsCasting = false;
	}
}

void UAbilityComponent::Client_AbilityFailure_Implementation(
					FName AbilityName, const FString& FailureReason)
{
	OnAbilityFailed.Broadcast(AbilityName, *FailureReason);
}

void UAbilityComponent::Client_AbilityCanceled_Implementation(FName AbilityName, const FString& FailureReason)
{
	bIsFocused = false;
	OnAbilityCanceled.Broadcast(AbilityName, *FailureReason);
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

void UAbilityComponent::OnRep_ActiveEffectsUpdated_Implementation()
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Active Effects has been Updated"),
		*GetName(), GetOwner()->HasAuthority()? TEXT("SERVER") : TEXT("CLIENT") );
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
	DOREPLIFETIME_CONDITION(UAbilityComponent, bIsFocused, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UAbilityComponent, _ActiveEffects, COND_OwnerOnly);
}