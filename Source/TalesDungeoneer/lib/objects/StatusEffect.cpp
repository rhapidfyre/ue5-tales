// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "StatusEffect.h"

#include "Net/UnrealNetwork.h"


UStatusEffect::UStatusEffect(FName AbilityName, ACharacterBase* EffectInstigator)
{
	_AbilityName = AbilityName;
	_EffectInstigator = EffectInstigator;
}

void UStatusEffect::SetAbilityName(FName AbilityName)
{
	if (!bInitialized)
		_AbilityName = AbilityName;
}

void UStatusEffect::SetEffectInstigator(ACharacterBase* EffectInstigator)
{
	if (!bInitialized)
		_EffectInstigator = EffectInstigator;
}

void UStatusEffect::InitializeEffect()
{
	if (!HasAuthority())
		return;
	
	if (!bInitialized)
	{
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
		bTicksIndependently	= AbilityData.bTickIndependently;
		_TimeRemaining		= AbilityData.EffectDuration;
		
		UE_LOG(LogTemp, Display, TEXT("%s(%s) has successfully initialized... Owner: %s"),
				*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *(GetOwningActor()->GetName()));
	
		GetWorld()->GetTimerManager().SetTimer(_Timer, this,
				&UStatusEffect::TimerTick, 0.5, true);
		
		bInitialized = true;

		// Only dispatch if anything is listening
		if (OnEffectActivated.IsBound())
				OnEffectActivated.Broadcast(this, _AbilityName);
	}
}

void UStatusEffect::PostInitProperties()
{
	UObject::PostInitProperties();
	// Called when the World exists. Custom 'BeginPlay' function for UObject.
	if (this->GetWorld())
	{
		BeginPlay();
	}
}

void UStatusEffect::BeginDestroy()
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s) has been destroyed"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
	UObject::BeginDestroy();
}

void UStatusEffect::BeginPlay()
{
	if (_TimeRemaining <= 0.f)
	{
		this->ConditionalBeginDestroy();
	}
}

void UStatusEffect::ApplyInitialEffects()
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
	const FStSpellData   SpellData   = UAbilitySystem::GetSpellDataFromName(_AbilityName);
}

void UStatusEffect::TimerTick()
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

UWorld* UStatusEffect::GetWorld() const
{
	if (const UObject* MyOuter = GetOuter())
	{
		return MyOuter->GetWorld();
	}
	return nullptr;
}


void UStatusEffect::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(UStatusEffect, _AbilityName, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UStatusEffect, _TimeRemaining, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UStatusEffect, _TargetActor, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UStatusEffect, _EffectInstigator, COND_OwnerOnly);
}
