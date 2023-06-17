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

void AAbilityEffectBase::EventOnEffectTick_Implementation()
{
	EffectTick();
}

void AAbilityEffectBase::SetAbilityInstigator(ACharacterBase* AbilityInstigator)
{
	if (!bInitialized)
	{
		if (HasAuthority())
		{
			if (!bInitialized && IsValid(AbilityInstigator))
			{
				_Instigator = AbilityInstigator;
				UE_LOG(LogTemp,Warning, TEXT("%s(%s): SetAbilityInstigator()"),
					*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
			}
		}
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
	if (!bInitialized)
	{
		if (HasAuthority())
		{
			if (!bInitialized && !ImpactLocation.IsNearlyZero(0.0001))
			{
				_ImpactLocation = ImpactLocation;
			}
		}
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
			_AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
			InitializeAbility();
		}
	}
}

void AAbilityEffectBase::BeginPlay()
{
	if (bShowDebug)
	{
		UE_LOG(LogTemp, Display,
			TEXT("%s(%s): Successfully created. Duration: %d"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), _TimeRemaining);
	}
	Super::BeginPlay();
}

void AAbilityEffectBase::BeginDestroy()
{
	Super::BeginDestroy();
	UE_LOG(LogTemp,Warning, TEXT("%s(%s): BeginDestroy()"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
}

void AAbilityEffectBase::SetOwner(AActor* NewOwner)
{
	if (!bInitialized)
	{
		Super::SetOwner(NewOwner);
	
		_OriginLocation = GetActorLocation();

		if (!IsValid(_TargetActor))
		{
			// If no target actor and no impact location, target is self
			if (_ImpactLocation.IsNearlyZero(0.0001))
				_TargetActor = _Instigator;
		}
		UE_LOG(LogTemp, Display, TEXT("%s(%s): New Owner Set"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	}
}

void AAbilityEffectBase::SetAbilityReady()
{
	if (!bInitialized)
	{
		// Perform setup verification steps
		

		bInitialized = true;
		
		// Activate Ability Actor
		if (HasAuthority())
		{
			// Start the expiration timer
			GetWorld()->GetTimerManager().SetTimer(_EffectTimer, this,
									&AAbilityEffectBase::EffectTick, 1.f, true);

			// Trigger Delegates
			OnEffectActivated.Broadcast();
		}
	}
}

void AAbilityEffectBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AAbilityEffectBase, _AbilityName);
}

void AAbilityEffectBase::InitializeAbility()
{
	
	if (UAbilitySystem::GetAbilityNameIsValid(_AbilityName))
	{
		_AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
	}
	
	// Determine Ability Data
	_TimeRemaining = FMath::RoundToInt( _AbilityData.EffectDuration );

	if (_TimeRemaining < 1)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("'%s' was created with an invalid duration of %d seconds... Destroying."),
			*GetName(), _TimeRemaining);
		return;
	}
	
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