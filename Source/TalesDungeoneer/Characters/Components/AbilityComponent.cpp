
#include "AbilityComponent.h"

#include "AiController.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "../CharacterBase.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
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
				const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(Hotkey.AbilityName);
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
							if (GetOwner()->HasAuthority())
							{
								ActivateAbility(Hotkey.AbilityName,
									GetTargetedActor(),
									CamManager->GetActorForwardVector());
							}
							else
							{
								OnAbilityCastStarted.Broadcast(
									Hotkey.AbilityName, AbilityData.ActivationTime);
								Server_RequestAbility(Hotkey.AbilityName,
									GetTargetedActor(),
									CamManager->GetActorForwardVector());
							}
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

					ActivateAbility(Hotkey.AbilityName, GetTargetedActor(),
						GetOwner()->GetActorForwardVector());
					
				}
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
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	if (GetOwner()->HasAuthority())
	{
		if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
		{
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
			FStAbilityEffect AbilityEffect = FStAbilityEffect(AbilityName);
			
			// Lock against reading from the array
			// Releases lock automatically when scope is lost
			FRWScopeLock WriteLock(_MutexLock, SLT_Write);

			// Add the effect to the appropriate key in the active effects map	
			_ActiveEffects[AbilityName].Add(AbilityEffect);
			OnEffectActivated.Broadcast(AbilityName, _ActiveEffects[AbilityName].Num());
			Client_AbilityAdded(AbilityName, AbilityEffect);
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

	const FVector EndPosition = SpawnTransform.GetLocation() + (ForwardVector * AbilityData.MaxReach);

	// Spawns the ability effect actor
	TSubclassOf<AAbilityEffectBase> AbilityBase = AAbilityEffectBase::StaticClass();

	if (IsValid(AbilityData.AbilityBase))
		AbilityBase = AbilityData.AbilityBase;
	
	AAbilityEffectBase* AbilityEffect = GetWorld()->
				SpawnActorDeferred<AAbilityEffectBase>(AbilityBase, SpawnTransform);
	
	if (IsValid(AbilityEffect))
	{
		SetIsCasting(true);
		OnAbilityCastStarted.Broadcast(AbilityName, AbilityData.ActivationTime);
		
		AbilityEffect->SetInstigator( GetOwner()->GetInstigator() );
		AbilityEffect->SetAbilityInstigator( Cast<ACharacterBase>(GetOwner()) );
		AbilityEffect->SetTargetActor( Cast<ACharacterBase>(GetTargetedActor()) );
		AbilityEffect->SetAbilityName(AbilityName);
		
		AbilityEffect->OnAbilityFinished.AddDynamic(this,
			&UAbilityComponent::SetNoLongerCasting);
		
		AbilityEffect->FinishSpawning(SpawnTransform);
		
		if (IsValid(EffectInstigator) && !AbilityData.SpawnBone.IsNone())
		{
			USkeletalMeshComponent* SkeletalMesh = EffectInstigator->GetMesh();
			AbilityEffect->AttachToComponent(SkeletalMesh,
				FAttachmentTransformRules::SnapToTargetIncludingScale, AbilityData.SpawnBone);
		}
		
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

void UAbilityComponent::SetNoLongerCasting(FName AbilityName, bool WasSuccessful)
{
	if (GetOwner()->HasAuthority())
	{
		Multicast_StopCasting(AbilityName, WasSuccessful);
		bIsCasting = false;
		
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
			FStAbilityEffect* AbilityEffect = &ArrayOfEffects[i];
			
			// Only reduce timer if concurrent tick, or is topmost index of array
			if (AbilityEffect->bTicksIndependently || i == 0)
			{
				AbilityEffect->TimeRemaining -= TimerRate;
				if (AbilityEffect->TimeRemaining < 0.f)
				{
					// Remove this entry if the time has expired
					const FName OldAbilityName = EffectName;
					Client_AbilityExpired(OldAbilityName, *AbilityEffect);
					ArrayOfEffects.RemoveAt(i);
					OnEffectExpired.Broadcast(OldAbilityName, ArrayOfEffects.Num());
					
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

void UAbilityComponent::Client_AbilityAdded_Implementation(FName AbilityName, FStAbilityEffect AbilityEffect)
{
	if (AbilityName.IsNone())
		return;
	
	for (FStAbilityEffect& ExistingEffect : _ActiveEffects[AbilityName])
	{
		// If this ability already exists, update it and return
		if (ExistingEffect.UniqueId == AbilityEffect.UniqueId)
		{
			// Sync the time
			ExistingEffect.TimeRemaining = AbilityEffect.TimeRemaining;
			return;
		}
	}
	
	// Otherwise, add it
	_ActiveEffects[AbilityName].Add(AbilityEffect);
	
	// Trigger Delegates, such as the HUD
	OnEffectActivated.Broadcast(AbilityName, _ActiveEffects[AbilityName].Num());
	
}

void UAbilityComponent::Client_AbilityExpired_Implementation(
		FName AbilityName, FStAbilityEffect AbilityEffect)
{
	if (AbilityName.IsNone())
		return;

	if (!_ActiveEffects.Contains(AbilityName))
		return;
	
	int EffectIndex = 0;
	// Exclusive Scope
	{
		FRWScopeLock ReadLock(_MutexLock,SLT_ReadOnly);
	
		for (int i = _ActiveEffects[AbilityName].Num() - 1; i >= 0; i++)
		{
			FStAbilityEffect& ExistingEffect = _ActiveEffects[AbilityName][i];
			if (ExistingEffect.UniqueId == AbilityEffect.UniqueId)
			{
				EffectIndex = i;
			}
		}
	}

	FRWScopeLock WriteLock(_MutexLock, SLT_Write);
	_ActiveEffects[AbilityName].RemoveAt(EffectIndex);
	
	// Trigger Delegates, such as the HUD
	OnEffectExpired.Broadcast(AbilityName, _ActiveEffects[AbilityName].Num());
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
	DOREPLIFETIME(UAbilityComponent, bIsCasting);
}