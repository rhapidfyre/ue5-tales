// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityEffectBase.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Delegates/Delegate.h"
#include "Net/UnrealNetwork.h"


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

void AAbilityEffectBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeAbility();
	
	// Setup success timer
	if (HasAuthority())
	{
		if (_TimeRemaining <= 0.f)
		{
			_TimeRemaining = 3.0;
		}
		GetWorld()->GetTimerManager().SetTimer(_EffectTimer,
			this, &AAbilityEffectBase::EffectTick, TimerTickRate, true);
		
	}
	else
	{
		const FStAbilityData AbilityData = GetAbilityData();
		
		if (IsValid(AbilityData.AnimationData.AnimationOnStart))
		{
			if (AbilityData.AnimationData.DelayStartAnim > 0.f)
			{
				FTimerHandle StartAnimTimer;
				FTimerDelegate StartAnimDelegate;
				StartAnimDelegate.BindUFunction(this, FName("PlayDelayedAnimation"),
					 GetOriginatingActor(), AbilityData.AnimationData.AnimationOnStart);
				GetWorld()->GetTimerManager().SetTimer(StartAnimTimer, StartAnimDelegate,
					_AbilityData.AnimationData.DelayStartAnim, false);
			}
			else
			{
				PlayDelayedAnimation(GetOriginatingActor(), AbilityData.AnimationData.AnimationOnStart);
			}
		}

		if (IsValid(AbilityData.VisualEffects.NiagaraEffect))
		{
			if (AbilityData.VisualEffects.DelayEffect > 0.f)
			{
				FTimerHandle NiagaraTimer;
				FTimerDelegate NiagaraDelegate;
				NiagaraDelegate.BindUFunction(this, FName("PlayDelayedNiagara"),
					 AbilityData.VisualEffects.NiagaraEffect, AbilityData.VisualEffects.EffectScale);
				GetWorld()->GetTimerManager().SetTimer(NiagaraTimer, NiagaraDelegate,
					AbilityData.VisualEffects.DelayEffect, false);
			}
			else
			{
				PlayDelayedNiagara(AbilityData.VisualEffects.NiagaraEffect,
					AbilityData.VisualEffects.EffectScale);
			}
		}
		
	}
	
}

void AAbilityEffectBase::Destroyed()
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Destroyed()"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
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

void AAbilityEffectBase::AbilityComplete(bool WasSuccessful)
{
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

void AAbilityEffectBase::PlayDelayedSound(FVector SoundLocation, USoundBase* SoundBase)
{
	UAudioComponent* CastSound = UGameplayStatics::SpawnSoundAttached(
		SoundBase, GetRootComponent(), NAME_None, SoundLocation, FRotator(0.f),
		EAttachLocation::SnapToTarget, false);
	if (IsValid(CastSound))
	{
		CastSound->bAutoDestroy = true;
		CastSound->Play();
	}
}

void AAbilityEffectBase::PlayDelayedNiagara(UNiagaraSystem* NiagaraEffect, float NiagaraScale)
{
	UNiagaraComponent* NiagaraSys = UNiagaraFunctionLibrary::SpawnSystemAttached(
		NiagaraEffect, GetRootComponent(), FName(), FVector::ZeroVector,
		FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
	if (IsValid(NiagaraSys))
	{
		NiagaraSys->SetRelativeScale3D( FVector(NiagaraScale) );
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

		/*
		if (_AbilityData.VisualEffects.NiagaraEffect)
		{
			NiagaraComponent->SetAsset(_AbilityData.VisualEffects.NiagaraEffect);
			NiagaraComponent->SetWorldScale3D(FVector(_AbilityData.VisualEffects.EffectScale));
			NiagaraComponent->Activate(true);
			NiagaraComponent->SetAutoDestroy(true);
		}
		*/
		
		if (AbilityData.SoundData.SoundCasting)
		{
			if (AbilityData.SoundData.DelaySoundCasting > 0.f)
			{
				FTimerHandle CastTimer;
				FTimerDelegate CastDelegate;
				CastDelegate.BindUFunction(this, FName("PlayDelayedSound"),
					 GetOriginatingActor()->GetActorLocation(), AbilityData.SoundData.SoundCasting);
				GetWorld()->GetTimerManager().SetTimer(CastTimer, CastDelegate,
					AbilityData.SoundData.DelaySoundCasting, false);
			}
			else
			{
				PlayDelayedSound( GetOriginatingActor()->GetActorLocation(), AbilityData.SoundData.SoundCasting );
			}
		}
	
		if (AbilityData.SoundData.SoundLooping)
		{
			LoopSound = UGameplayStatics::SpawnSoundAttached(
				_AbilityData.SoundData.SoundLooping, GetRootComponent(), NAME_None, FVector(0.f), FRotator(0.f),
				EAttachLocation::SnapToTarget, false,
				1.f, 1.f,0.f, nullptr,
				nullptr, true);
			
			if (IsValid(LoopSound))
			{
				LoopSound->StopDelayed(_TimeRemaining);
			}
		}
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

	/*
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(GetRootComponent());
	*/
	
	UE_LOG(LogTemp, Display, TEXT("%s(%s): SetupDefaults()"), *GetName(),
		HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
}

void AAbilityEffectBase::EffectTick()
{
	_TimeRemaining -= TimerTickRate;
	if (_TimeRemaining <= 0.f)
	{
		OnEffectExpired.Broadcast(_AbilityName);
		AbilityComplete(true);
	}
}