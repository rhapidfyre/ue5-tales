// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityEffectBase.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Delegates/Delegate.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Characters/PlayerCharacterBase.h"

AAbilityEffectBase::AAbilityEffectBase()
{
	SetupDefaults();
}

AAbilityEffectBase::AAbilityEffectBase(ACharacterBase* Instigator, const FName AbilityName, FVector ImpactLocation)
{
	SetAbilityInstigator(Instigator);
	SetAbilityName(AbilityName);
	SetImpactLocation(ImpactLocation);
	
	SetupDefaults();
	
}

AAbilityEffectBase::AAbilityEffectBase(ACharacterBase* Instigator, const FName AbilityName, AActor* TargetActor)
{
	SetAbilityInstigator(Instigator);
	SetAbilityName(AbilityName);
	SetTargetActor(Cast<ACharacterBase>(TargetActor));
	
	SetupDefaults();
}

void AAbilityEffectBase::ExecuteCastingEffects(FStAbilityFx AbilityFx, bool StopOnDestroyed)
{
	// Casting Sound Effect
	if (IsValid(AbilityFx.SoundEffect))
	{
		FVector ActorLocation = GetOriginatingActor()->GetActorLocation();
		float loopTime = AbilityFx.NiagaraLoopTime - AbilityFx.DelayEffect;
				
		if (AbilityFx.DelaySound > 0.f)
		{
			FTimerHandle CastTimer;
			FTimerDelegate CastDelegate;
			CastDelegate.BindUObject(this, &AAbilityEffectBase::PlayDelayedSound,
				 ActorLocation, AbilityFx.SoundEffect, loopTime, StopOnDestroyed);
			GetWorld()->GetTimerManager().SetTimer(CastTimer, CastDelegate,
				AbilityFx.DelaySound, false);
		}
		else
		{
			PlayDelayedSound(ActorLocation,
				AbilityFx.SoundEffect, loopTime, StopOnDestroyed);
		}
	}
	// Casting Visual Fx
	if (IsValid(AbilityFx.NiagaraEffect))
	{
		if (AbilityFx.DelayEffect > 0.f)
		{
			FTimerHandle NiagaraTimer;
			FTimerDelegate NiagaraDelegate;
			NiagaraDelegate.BindUObject(this, &AAbilityEffectBase::PlayDelayedNiagara, AbilityFx);
			GetWorld()->GetTimerManager().SetTimer(NiagaraTimer, NiagaraDelegate,
				AbilityFx.DelayEffect, false);
					
		}
		else
		{
			PlayDelayedNiagara(AbilityFx);
		}
	}
}

void AAbilityEffectBase::SetAbilityInstigator(ACharacterBase* AbilityInstigator)
{
	if (HasAuthority())
	{
		_Instigator = AbilityInstigator;
		UE_LOG(LogTemp,Warning, TEXT("%s(%s): SetAbilityInstigator()"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
}

void AAbilityEffectBase::SetTargetActor(ACharacterBase* TargetActor)
{
	if (!bInitialized)
	{
		if (HasAuthority())
		{
			if (!bInitialized && IsValid(TargetActor))
			{
				_TargetActor = TargetActor;
			}
		}
	}
}

void AAbilityEffectBase::SetImpactLocation(FVector ImpactLocation)
{
	if (HasAuthority())
	{
		_ImpactLocation = ImpactLocation;
	}
}

void AAbilityEffectBase::SetImpactRotation(FRotator ImpactRotation)
{
	if (!bInitialized)
	{
		if (HasAuthority())
		{
			if (!bInitialized && !ImpactRotation.IsNearlyZero(0.0001))
			{
				_ImpactRotation = ImpactRotation;
			}
		}
	}
}

void AAbilityEffectBase::SetAbilityName(FName AbilityName)
{
	if (!bInitialized)
	{
		if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
		{
			_AbilityName = AbilityName;
		}
	}
}

void AAbilityEffectBase::CancelCasting(FName AbilityName, FString Reason)
{
	if (HasAuthority())
	{
		OnAbilityFinished.Broadcast(GetAbilityName(), false);
		Destroy();
	}
}

void AAbilityEffectBase::DestroyPrematurely()
{
	bSpellFailed = true;
	OnAbilityFinished.Broadcast(GetAbilityName(), false);
	Destroy();
}

void AAbilityEffectBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeAbility();
	
	if (_TimeRemaining <= 0.f)
	{
		_TimeRemaining = 3.0;
	}

	if (IsValid(_CastingComponent))
	{
		_CastingComponent->OnAbilityCanceled.AddDynamic(this, &AAbilityEffectBase::CancelCasting);
	}
	
	// Setup success timer
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(_EffectTimer,
			this, &AAbilityEffectBase::EffectTick, TimerTickRate, true);
		
	}
	const FStAbilityData AbilityData = GetAbilityData();
		
	if (IsValid(AbilityData.AnimationData.AnimationOnStart))
	{
		if (AbilityData.AnimationData.DelayStartAnim > 0.f)
		{
			FTimerHandle StartAnimTimer;
			FTimerDelegate StartAnimDelegate;
			ACharacter* OriginatingActor;
			StartAnimDelegate.BindUObject(this, &AAbilityEffectBase::PlayDelayedAnimation,
				 OriginatingActor, AbilityData.AnimationData.AnimationOnStart);
			GetWorld()->GetTimerManager().SetTimer(StartAnimTimer, StartAnimDelegate,
				_AbilityData.AnimationData.DelayStartAnim, false);
		}
		else
		{
			PlayDelayedAnimation(GetOriginatingActor(), AbilityData.AnimationData.AnimationOnStart);
		}
	}

	// Dispatch casting effects
	for (auto AbilityFx : AbilityData.EffectCasting)
	{
		ExecuteCastingEffects(AbilityFx);
	}
	for (auto AbilityFx : AbilityData.EffectLooped)
	{
		ExecuteCastingEffects(AbilityFx, true);
	}
}

void AAbilityEffectBase::Destroyed()
{
	for (UNiagaraComponent* NiagaraEmitter : LoopingNiagaraEmitters)
	{
		if (IsValid(NiagaraEmitter))
		{
			NiagaraEmitter->Deactivate();
			NiagaraEmitter->DestroyInstance();
		}
	}
	for (UAudioComponent* SoundEmitter : LoopingSoundEmitters)
	{
		if (IsValid(SoundEmitter))
		{
			SoundEmitter->Stop();
			SoundEmitter->DestroyComponent();
		}
	}
	Super::Destroyed();
}

void AAbilityEffectBase::OnConstruction(const FTransform& Transform)
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s): OnConstruction() - Ability Name: %s"),
	*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *_AbilityName.ToString());
	
	if (IsValid(GetOwner()))
	{
		const ACharacterBase* MyOwner = Cast<ACharacterBase>(GetOwner());
		if (IsValid(MyOwner))
		{
			USkeletalMeshComponent* SkeletalMesh = MyOwner->GetMesh();
			if (IsValid(SkeletalMesh))
			{
				const FStAbilityData AbilityData = GetAbilityData();
				
				AttachToComponent(SkeletalMesh,
					FAttachmentTransformRules::SnapToTargetIncludingScale,
					FName("None"));//AbilityData.AttachBoneOnSpawn);
				
				//this->SetActorRelativeLocation(AbilityData.AttachOffset);
				//this->SetActorRelativeRotation(AbilityData.AttachRotOffset.Rotation());
			}
		}
	}
	
	Super::OnConstruction(Transform);
}

void AAbilityEffectBase::SetOwner(AActor* NewOwner)
{
	if (!bInitialized && HasAuthority())
	{
		Super::SetOwner(NewOwner);
	
		_OriginLocation = GetActorLocation();

		if (!IsValid(_TargetActor))
		{
			// If no target actor and no impact location, target is self
			if (_ImpactLocation.IsNearlyZero(0.0001))
				_TargetActor = _Instigator;
		}
	}
}

void AAbilityEffectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAbilityEffectBase, _AbilityName);
	DOREPLIFETIME(AAbilityEffectBase, _Instigator);
	DOREPLIFETIME(AAbilityEffectBase, _WasSuccessful);
}

/**
 * @brief Applies the ability's effect (if any) to the targeted actor. If no target is selected,
 *			and no optional override character is given, this does nothing.
 * @param OverrideCharacter Optional - Overrides GetTargetActor as the target for this effect
 */
void AAbilityEffectBase::ApplyEffectToTarget(ACharacterBase* OverrideCharacter)
{
	// If the projectile is not targeted
	// Let the child projectile class handle damage
	if (!IsValid(GetTargetActor()) && GetAbilityData().TargetType == EAbilityTarget::PROJECTILE)
		return;
	
	ACharacterBase* InstigatingActor = Cast<ACharacterBase>(GetInstigator());
	if (!IsValid(InstigatingActor))
		return;
	
	ACharacterBase* EffectTarget = OverrideCharacter;
	if (!IsValid(EffectTarget))
	{
		EffectTarget = Cast<ACharacterBase>(GetTargetActor());
		if (!IsValid(EffectTarget))
		{
			return;
		}
	}

	const FStSpellData   SpellData	 = GetSpellData();
	const FStAbilityData AbilityData = GetAbilityData();
	if (AbilityData.AbilityType == EAbilityType::DETRIMENT)
	{
		const bool IsSameTeam = (EffectTarget->GetCharacterTeam() == InstigatingActor->GetCharacterTeam());
		if (EffectTarget == GetInstigator() || IsSameTeam)
		{
			UE_LOG(LogTemp, Display, TEXT("%s(%s): Offensive spell targeting teammember has been ignored"),
				*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
			return;
		}
	}

	UE_LOG(LogTemp, Display, TEXT("%s(%s): Applying Effect to Actor '%s'"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *EffectTarget->GetName());

	// Apply damages
	for (FStAbilityDamageData DamageInfo : SpellData.DamageData)
	{
		// TODO - Damage Resistance & Bonuses
		EffectTarget->VitalityComponent->ModifyVitalityStat(EVitalityCategories::HEALTH,  DamageInfo.ConsumeHealth);
		EffectTarget->VitalityComponent->ModifyVitalityStat(EVitalityCategories::MAGIC,   DamageInfo.ConsumeMagic);
		EffectTarget->VitalityComponent->ModifyVitalityStat(EVitalityCategories::STAMINA, DamageInfo.ConsumeStamina);
	}
	// Apply effect to target actor
	EffectTarget->AbilityComponent->ApplyEffect(InstigatingActor, _AbilityName);
	
}

void AAbilityEffectBase::AbilityComplete(bool WasSuccessful)
{
	if (!HasAuthority())
		return;
	
	const FStAbilityData AbilityData = GetAbilityData();

	// The instigator is always nullptr (world) or ACharacterBase (gamemode character)
	ACharacterBase* InstigatorPawn = Cast<ACharacterBase>( GetInstigator() );

	TArray<ACharacterBase*> AllCharacters; // Filtered array of all characters to be affected

	if (AbilityData.TargetType == EAbilityTarget::TARGET)
	{
		ACharacterBase* TargetCharacter = Cast<ACharacterBase>( GetTargetActor() );
		if (IsValid(TargetCharacter))
		{
			// Target is self, or within range
			if (	TargetCharacter == InstigatorPawn
				||	InstigatorPawn->GetDistanceTo(TargetCharacter) <= AbilityData.MaxRange)
			{
				// Add target to list of affected characters
				AllCharacters.Add(TargetCharacter);
			}
		}
	}
	
	TArray<AActor*> AllTargets; // Used for all actors matching the eligible type(s)
	const ECharacterTeam InstigatorTeam = InstigatorPawn->GetCharacterTeam();
	
	switch(AbilityData.TargetType)
	{
	case EAbilityTarget::NEAR:
		
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterBase::StaticClass(), AllTargets);
		for (int i = AllTargets.Num() - 1; i >= 0; i--)
		{
			ACharacterBase* TargetCharacter = Cast<ACharacterBase>(AllTargets[i]);
			if (IsValid(TargetCharacter) && TargetCharacter != InstigatorPawn)
			{
				// Target actor within tolerance area
				if (InstigatorPawn->GetDistanceTo(TargetCharacter) <= AbilityData.MaxRange)
					AllCharacters.Add(TargetCharacter);
			}
		}
		break;
		
	case EAbilityTarget::GROUP:
		
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterBase::StaticClass(), AllTargets);
		
		for (int i = AllTargets.Num() - 1; i >= 0; i--)
		{
			ACharacterBase* TargetCharacter = Cast<ACharacterBase>(AllTargets[i]);
			if (IsValid(TargetCharacter))
			{
				const bool IsSameTeam = (InstigatorTeam == TargetCharacter->GetCharacterTeam());
				const bool IsBenefitSpell = AbilityData.AbilityType == EAbilityType::BENEFIT;
				if ( (IsSameTeam && IsBenefitSpell) || (!IsSameTeam && !IsBenefitSpell))
				{
					// Target within tolerance area
					if (InstigatorPawn->GetDistanceTo(TargetCharacter) <= AbilityData.MaxRange)
						AllCharacters.Add(TargetCharacter);
				}
			}
		}
		
		break;
		
	case EAbilityTarget::CONE:
		//TODO - Target everything within an angle ahead of the caster
		break;
		
	default:
		break;
		
	}
	//switch
	
	for (ACharacterBase* SoftTarget : AllCharacters)
	{
		ApplyEffectToTarget(SoftTarget);
	}

	// Deductions for a successful cast
	InstigatorPawn->VitalityComponent->DamageHealth(InstigatorPawn, _SpellData.HitpointsAffected);
	InstigatorPawn->VitalityComponent->ConsumeMagic(InstigatorPawn, _SpellData.MagicPointsAffected);
	InstigatorPawn->VitalityComponent->ConsumeStamina(InstigatorPawn, _SpellData.StaminaAffected);
	
	OnAbilityFinished.Broadcast(GetAbilityName(), WasSuccessful);
	Destroy();
}

void AAbilityEffectBase::PlayDelayedAnimation(ACharacter* PlayTarget, UAnimMontage* AnimMontage)
{
	if (IsValid(PlayTarget) && IsValid(AnimMontage))
	{
		PlayTarget->StopAnimMontage(nullptr); // Stop all excuting montages
		PlayTarget->PlayAnimMontage(AnimMontage);
	}
}

void AAbilityEffectBase::PlayDelayedSound(FVector SoundLocation, USoundBase* SoundBase, float LoopTime, bool StopOnDestroy)
{
	UAudioComponent* SoundEffect = UGameplayStatics::SpawnSoundAttached(
		SoundBase, GetRootComponent(), NAME_None, SoundLocation, FRotator(0.f),
		EAttachLocation::SnapToTarget, StopOnDestroy);
	if (IsValid(SoundEffect))
	{
		SoundEffect->bAutoDestroy = true;
		SoundEffect->Play();
		LoopingSoundEmitters.Add(SoundEffect);
		if (LoopTime > 0.f)
		{
			SoundEffect->StopDelayed(LoopTime);
		}
	}
}

void AAbilityEffectBase::PlayDelayedNiagara(FStAbilityFx VisualEffect)
{
	USceneComponent* AttachComponent = GetRootComponent();	
	if (!VisualEffect.NiagaraBone.IsNone() || VisualEffect.NiagaraBone.IsEqual("root"))
	{
		if (VisualEffect.bAttachNiagaraToActor || VisualEffect.bAttachNiagaraToSkeleton)
		{
			if (IsValid( GetOriginatingActor() ))
			{
				if (VisualEffect.bAttachNiagaraToSkeleton)
				{
					USkeletalMeshComponent* SkeletalMesh = GetOriginatingActor()->GetMesh();
					if (IsValid(SkeletalMesh))
					{
						AttachComponent = SkeletalMesh;
					}
				}
				else
				{
					AttachComponent = GetOriginatingActor()->GetRootComponent();
				}
			}
		}
	}
	if (VisualEffect.bAttachNiagaraToSkeleton || VisualEffect.bAttachNiagaraToActor)
	{
		UNiagaraComponent* NiagaraSys = UNiagaraFunctionLibrary::SpawnSystemAttached(
			VisualEffect.NiagaraEffect, AttachComponent, VisualEffect.NiagaraBone,
			VisualEffect.EffectOffset, VisualEffect.EffectRotation,
			EAttachLocation::SnapToTargetIncludingScale, VisualEffect.NiagaraLoopTime > 0.f);
		if (IsValid(NiagaraSys))
		{
			LoopingNiagaraEmitters.Add(NiagaraSys);
			NiagaraSys->SetRelativeScale3D( FVector(VisualEffect.EffectScale) );
			NiagaraSys->Activate();
		}
	}
	else
	{
		UNiagaraComponent* NiagaraSys = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			VisualEffect.NiagaraEffect,
			GetActorLocation() + VisualEffect.EffectOffset,
			FRotator::ZeroRotator + VisualEffect.EffectRotation,
			FVector(VisualEffect.EffectScale), true, true);
		if (IsValid(NiagaraSys))
		{
			LoopingNiagaraEmitters.Add(NiagaraSys);
		}
	}
}

void AAbilityEffectBase::InitializeAbility()
{
	bInitialized = true;
	if (UAbilitySystem::GetAbilityNameIsValid(_AbilityName))
	{
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
		_AbilityData = AbilityData;
		_SpellData   = UAbilitySystem::GetSpellDataFromName(_AbilityName);
		
		_TimeRemaining = FMath::RoundToInt( _AbilityData.ActivationTime );
		if (_TimeRemaining <= 0.f)
		{
			_TimeRemaining = 1.f;
		}

		UE_LOG(LogTemp, Display, TEXT("%s(%s): InitializeAbility() - '%s'"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *_AbilityData.DisplayName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): InitializeAbility() Failed"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
	
}

void AAbilityEffectBase::SetupDefaults()
{ 
#ifdef UE_BUILD_DEBUG
	bShowDebug = true;
#endif
	
	bReplicates = true;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	UE_LOG(LogTemp, Display, TEXT("%s(%s): SetupDefaults()"), *GetName(),
		HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
}

void AAbilityEffectBase::EffectTick()
{
	_TimeRemaining -= TimerTickRate;
	if (_TimeRemaining <= 0.f)
	{
		AbilityComplete(true);
		GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
		if (_EffectTimer.IsValid())
			_EffectTimer.Invalidate();
	}
}