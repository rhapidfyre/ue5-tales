// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityEffectBase.h"

#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Entities/SpellActorBase.h"


AAbilityEffectBase::AAbilityEffectBase()
{
	SetupDefaults();
}

AAbilityEffectBase::AAbilityEffectBase(ACharacterBase* Instigator, const FName AbilityName, FVector ImpactLocation)
{
	
	_Instigator		= Instigator;
	_AbilityName	= AbilityName;
	_ImpactLocation = ImpactLocation;
	
}

AAbilityEffectBase::AAbilityEffectBase(ACharacterBase* Instigator, const FName AbilityName, AActor* TargetActor)
{
	_Instigator  = Instigator;
	_AbilityName = AbilityName;
	_TargetActor		 = TargetActor;
	
	if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
	{
		_AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	}
	
	SetupDefaults();
}

void AAbilityEffectBase::EventOnEffectTick_Implementation()
{
	EffectTick();
}

void AAbilityEffectBase::SetAbilityInstigator(ACharacterBase* AbilityInstigator)
{
	if (HasAuthority())
	{
		if (!bInitialized && IsValid(AbilityInstigator))
		{
			_Instigator = AbilityInstigator;
		}
	}
}

void AAbilityEffectBase::SetTargetActor(ACharacterBase* TargetActor)
{
	if (HasAuthority())
	{
		if (!bInitialized && IsValid(TargetActor))
		{
			_TargetActor = TargetActor;
		}
	}
}

void AAbilityEffectBase::SetImpactLocation(FVector ImpactLocation)
{
	if (HasAuthority())
	{
		if (!bInitialized && !ImpactLocation.IsNearlyZero(0.0001))
		{
			_ImpactLocation = ImpactLocation;
		}
	}
}

void AAbilityEffectBase::SetImpactRotation(FRotator ImpactRotation)
{
	if (HasAuthority())
	{
		if (!bInitialized && !ImpactRotation.IsNearlyZero(0.0001))
		{
			_ImpactRotation = ImpactRotation;
		}
	}
}

void AAbilityEffectBase::SetAbilityName(FName AbilityName)
{
	
	if (UAbilitySystem::GetAbilityNameIsValid(AbilityName))
	{
		_AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
	}
}

void AAbilityEffectBase::BeginPlay()
{
	bInitialized = true;
	if (HasAuthority())
	{
		
		if (!IsValid(_Instigator))
		{
			UE_LOG(LogTemp,Warning,
				TEXT("'%s' was created with no owner... Destroying."), *GetName());
			Destroy();
			return;
		}

		_OriginLocation = GetActorLocation();

		if (!IsValid(_TargetActor))
		{
			// If no target actor and no impact location, target is self
			if (_ImpactLocation.IsNearlyZero(0.0001))
				_TargetActor = _Instigator;
		}
	
		// Determine Ability Data
		_TimeRemaining = FMath::RoundToInt( _AbilityData.EffectDuration );

		if (_TimeRemaining < 1)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("'%s' was created with an invalid duration of %d seconds... Destroying."),
				*GetName(), _TimeRemaining);
		
			this->ConditionalBeginDestroy();
			return;
		
		}

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Display,
				TEXT("'%s' was successfully created with a duration of %d seconds."),
				*GetName(), _TimeRemaining);
		}

		if (IsValid(_AbilityData.SpawnActor))
		{
			FTransform SpawnTransform = _Instigator->GetActorTransform();
			SpawnTransform.SetScale3D( FVector(1.f) );
		
			ASpellActorBase* SpellActor = GetWorld()->SpawnActorDeferred<ASpellActorBase>(
						ASpellActorBase::StaticClass(), SpawnTransform);

			if (IsValid(SpellActor))
			{
				SpellActor->SetOwner(_Instigator);
				
				SpellActor->FinishSpawning(SpawnTransform);
			}
		
		}
	
		// Start the expiration timer
		GetWorld()->GetTimerManager().SetTimer(_EffectTimer, this,
								&AAbilityEffectBase::EffectTick, 1.f, true);

		// Trigger Delegates
		OnEffectActivated.Broadcast();Super::BeginPlay();
	}
}

void AAbilityEffectBase::BeginDestroy()
{
	Super::BeginDestroy();
	UE_LOG(LogTemp,Warning, TEXT("'%s' is being destroyed!"), *GetName());
}

void AAbilityEffectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAbilityEffectBase, _AbilityName);
}

void AAbilityEffectBase::SetupDefaults()
{
#ifdef UE_BUILD_DEBUG
	bShowDebug = true;
#endif
}

void AAbilityEffectBase::EffectTick()
{
	_TimeRemaining -= 1.f;
	OnEffectTick.Broadcast(_TimeRemaining);
	
	if (_TimeRemaining < 0)
	{
		if (bShowDebug)
		{
			UE_LOG(LogTemp, Display, TEXT("'%s' has expired and will be destroyed"), *GetName());
		}
		OnEffectExpired.Broadcast();
	}
}