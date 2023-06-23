// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "AbilityEffect.h"

#include "Net/UnrealNetwork.h"


UAbilityEffect::UAbilityEffect(FName AbilityName, ACharacterBase* EffectInstigator)
{
	_AbilityName = AbilityName;
	_EffectInstigator = EffectInstigator;
	InitializeEffect();
}

void UAbilityEffect::SetAbilityName(FName AbilityName)
{
	if (!bInitialized)
		_AbilityName = AbilityName;
}

void UAbilityEffect::SetEffectInstigator(ACharacterBase* EffectInstigator)
{
	if (!bInitialized)
		_EffectInstigator = EffectInstigator;
}

void UAbilityEffect::InitializeEffect()
{
	if (!bInitialized)
	{
		bInitialized = true;
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
		bTicksIndependently	= AbilityData.bTickIndependently;
		_TimeRemaining		= AbilityData.EffectDuration;
		UE_LOG(LogTemp, Display, TEXT("%s(%s) has successfully initialized... Owner: %s"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *(GetOwningActor()->GetName()));
	}
}

void UAbilityEffect::PostInitProperties()
{
	UObject::PostInitProperties();
	// Called when the World exists. Custom 'BeginPlay' function for UObject.
	if (this->GetWorld())
	{
		BeginPlay();
	}
}

void UAbilityEffect::BeginDestroy()
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s) has been destroyed"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	UObject::BeginDestroy();
}

void UAbilityEffect::BeginPlay()
{
	if (_TimeRemaining <= 0.f)
	{
		this->ConditionalBeginDestroy();
	}
	
	GetWorld()->GetTimerManager().SetTimer(_Timer, this,
		&UAbilityEffect::TimerTick, 0.5, true);

	// Only dispatch if anything is listening
	if (OnEffectActivated.IsBound())
		OnEffectActivated.Broadcast(this, _AbilityName);
	
	
}

void UAbilityEffect::ApplyInitialEffects()
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
	const FStSpellData   SpellData   = UAbilitySystem::GetSpellDataFromName(_AbilityName);
}

void UAbilityEffect::TimerTick()
{
	if (bInitialized)
	{
		_TimeRemaining -= 0.5;
		
		// Only dispatch if anything is listening
		if (OnEffectTick.IsBound())
			OnEffectTick.Broadcast(this, _AbilityName);
		
		if (_TimeRemaining <= 0.f)
		{
			// Invalidate this timer
			_TimeRemaining = 0.f;
			if (_Timer.IsValid())
				_Timer.Invalidate();
			
			UE_LOG(LogTemp, Display, TEXT("%s(%s) has expired and should get destroyed"),
				*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
			
			// Only dispatch if anything is listening
			if (OnEffectExpired.IsBound())
				OnEffectExpired.Broadcast(this, _AbilityName);

		}
	}
}

UWorld* UAbilityEffect::GetWorld() const
{
	if (const UObject* MyOuter = GetOuter())
	{
		return MyOuter->GetWorld();
	}
	return nullptr;
}


void UAbilityEffect::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UAbilityEffect, _AbilityName, COND_OwnerOnly);
}
