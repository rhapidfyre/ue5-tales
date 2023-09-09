// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "CombatAiControllerBase.h"

float ACombatAiControllerBase::AddHateTowardsTarget(ACharacterBase* HateTarget, float HatePoints)
{
	if (!FMath::IsNearlyZero(HatePoints, 0.001f))
	{
		const float* OldValue = _HateList.Find(HateTarget);
		if (OldValue == nullptr)
			_HateList.Add(HateTarget, HatePoints);
		else
			_HateList.Add(HateTarget, *OldValue + HatePoints);
		const float* NewValue = _HateList.Find(HateTarget);
		return NewValue != nullptr ? *NewValue : 0.f;
	}
	return 0.f;
}

float ACombatAiControllerBase::RemoveHateFromTarget(ACharacterBase* HateTarget, float HatePoints)
{
	if (!FMath::IsNearlyZero(HatePoints, 0.001f))
	{
		const float newValue = _HateList.Add(HateTarget, _HateList[HateTarget] - abs(HatePoints));
		if (FMath::IsNearlyZero(newValue, 0.001f))
			ClearTargetFromHateList(HateTarget);
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

void ACombatAiControllerBase::CheckCombatState(
	ECombatState OldCombatState, ECombatState NewCombatState)
{
	const bool isHateListEmpty = _HateList.IsEmpty();

	// Special behaviors for when the state makes specific changes
	switch (OldCombatState)
	{
		
		// NPC was engaged in combat
		case ECombatState::ENGAGED:
			break;
		
		// NPC was aware of a threat, or ending an engagement
		case ECombatState::ALERT:
			break;

		// NPC was incapacitated
		case ECombatState::INJURED:
			break;
		
		// NPC was recovering from combat or alertness
		case ECombatState::RECOVERY:
			break;

		// NPC was relaxed
		case ECombatState::RELAXED:
			break;
		
	default:
		break;
	}

	// If the NPC has restarted combat, keep the damage memory and hate list
	// If the NPC has returned to a recovery state, wipe memory lists
	switch (NewCombatState)
	{
	case ECombatState::ALERT:
		__fallthrough;
	case ECombatState::ENGAGED:
		return; // do nothing

	default:
		break; // wipe memory
	}
	
	WipeHateListMemory();
	_DamageList.Empty();
	
}

void ACombatAiControllerBase::BeginPlay()
{
	Super::BeginPlay();
	CharacterReference = Cast<ANpcCharacterBase>(GetPawn());
	if (IsValid(CharacterReference))
	{
		UVitalityWelfareComponent* VitalityWelfare = CharacterReference->VitalityWelfare;
		if (IsValid(VitalityWelfare))
		{
			if (!VitalityWelfare->OnDamageTaken.IsAlreadyBound(this, &ACombatAiControllerBase::RememberDamage))
				VitalityWelfare->OnDamageTaken.AddDynamic(this, &ACombatAiControllerBase::RememberDamage);
	
			if (!VitalityWelfare->OnCombatStateChanged.IsAlreadyBound(this, &ACombatAiControllerBase::CheckCombatState))
				VitalityWelfare->OnCombatStateChanged.AddDynamic(this, &ACombatAiControllerBase::CheckCombatState);
		}
	}
}

void ACombatAiControllerBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
