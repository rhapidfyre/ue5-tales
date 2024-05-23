// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Characters/Controllers/CombatAiControllerBase.h"


float ACombatAiControllerBase::AddHateTowardsTarget(ACharacterBase* HateTarget, float HatePoints)
{
	if (!FMath::IsNearlyZero(HatePoints, 0.001f))
	{
		const float* OldValue = _HateList.Find(HateTarget);
		if (OldValue == nullptr)
			_HateList.Add(HateTarget, HatePoints);
		else
			_HateList.Add(HateTarget, *OldValue + HatePoints);
		
		SortHateList();
		
		const float* NewValue = _HateList.Find(HateTarget);
		return NewValue != nullptr ? *NewValue : 0.f;
	}
	return 0.f;
}

void ACombatAiControllerBase::SortHateList()
{
	// TODO - We need a scope lock, as this hate list can change during sort
	// Sorts the hate list by highest hate value first
	_HateList.ValueSort([](const float A, const float B){return A > B;});
	OnHateListUpdated.Broadcast();
}

float ACombatAiControllerBase::RemoveHateFromTarget(ACharacterBase* HateTarget, float HatePoints)
{
	if (!FMath::IsNearlyZero(HatePoints, 0.001f))
	{
		const float newValue = _HateList.Add(HateTarget, _HateList[HateTarget] - abs(HatePoints));
		if (FMath::IsNearlyZero(newValue, 0.001f))
			ClearTargetFromHateList(HateTarget);
		
		SortHateList();
		
		const float* NewValue = _HateList.Find(HateTarget);
		return NewValue != nullptr ? *NewValue : 0.f;
	}
	return 0.f;
}

bool ACombatAiControllerBase::ClearTargetFromHateList(ACharacterBase* HateTarget)
{
	if (IsValid(HateTarget))
		return _HateList.Remove(HateTarget) > 0;
	_HateList.Empty();
	return true;
}

bool ACombatAiControllerBase::WipeHateListMemory()
{
	_HateList.Empty();
	return true;
}

ACharacterBase* ACombatAiControllerBase::GetMostHatedTarget(float& HatePoints) const
{
	TArray<ACharacterBase*> HatedTargets;
	_HateList.GetKeys(HatedTargets);
	ACharacterBase* HighestHatedTarget = nullptr;
	for (ACharacterBase* HatedTarget : HatedTargets)
	{
		const float* HateValue = _HateList.Find(HatedTarget);
		if (HateValue != nullptr)
		{
			if (*HateValue > HatePoints)
			{
				HighestHatedTarget = HatedTarget;
				HatePoints = *HateValue;
			}
		}
	}
	return HighestHatedTarget;
}

void ACombatAiControllerBase::RememberDamage(AActor* DamagingActor, float DamageValue)
{
	if (IsValid(DamagingActor))
	{
		if (!FMath::IsNearlyZero(DamageValue, 0.001f))
		{
			ACharacterBase* CharacterBase = Cast<ACharacterBase>(DamagingActor);
			if (IsValid(CharacterBase))
			{
				// Remember the damage that has occurred
				const float* OldValue = _DamageList.Find(CharacterBase);
				if (OldValue == nullptr)
					_DamageList.Add(CharacterBase, DamageValue);
				else
					_DamageList.Add(CharacterBase, *OldValue + DamageValue);

				// Add the damage dealer to the hate list
				AddHateTowardsTarget(CharacterBase, abs(DamageValue));
			}
		}
	}
}

/**
 * \brief Checks if the supplied target is on this actors hate list
 * \param TargetActor The actor being tested
 * \return True if the target is on the hate list (has aggro)
 */
bool ACombatAiControllerBase::IsTargetOnHateList(AActor* TargetActor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>(TargetActor);
	if (IsValid(CharacterBase))
		return _HateList.Find(CharacterBase) != nullptr;
	return false;
}

/**
 * \brief Checks if the supplied target is being perceived by this ai
 * \param TargetActor The actor being tested
 * \return True if the target is currently being detected by this ai
 */
bool ACombatAiControllerBase::IsTargetValid(AActor* TargetActor)
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>(TargetActor);
	if (IsValid(CharacterBase))
		return _ValidTargets.Find(CharacterBase) != nullptr;
	return false;
}

void ACombatAiControllerBase::BeginPlay()
{
	Super::BeginPlay();
	
	CharacterReference = Cast<ANpcCharacterBase>(GetPawn());
	if (IsValid(CharacterReference))
	{

	}

	if (IsValid(AiPerception))
	{
		if (!AiPerception->OnTargetPerceptionUpdated.IsAlreadyBound(this, &ACombatAiControllerBase::TargetPerception))
		{
			AiPerception->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAiControllerBase::TargetPerception);
		}
	}
}

void ACombatAiControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ACombatAiControllerBase::TargetPerception(AActor* StimulusActor, FAIStimulus StimulusData)
{
	
	// Add sensed characters to the target list
	ACharacterBase* StimulusTarget = Cast<ACharacterBase>(StimulusActor);
	if (!IsValid(StimulusTarget))
		return;
	
	if (StimulusData.WasSuccessfullySensed())
	{
		if (!_ValidTargets.Contains(StimulusTarget))
		{
			_ValidTargets.Add(StimulusTarget);
			UE_LOG(LogTemp, Display, TEXT("%s is now tracking %s for a total of %d targets"),
				*GetName(), *StimulusTarget->GetName(), _ValidTargets.Num());
		}
	}
	else
	{
		if (_ValidTargets.Contains(StimulusTarget))
		{
			_ValidTargets.Remove(StimulusTarget);
			UE_LOG(LogTemp, Display, TEXT("%s is no longer tracking %s for a total of %d targets"),
				*GetName(), *StimulusTarget->GetName(), _ValidTargets.Num());
		}
	}
	
}