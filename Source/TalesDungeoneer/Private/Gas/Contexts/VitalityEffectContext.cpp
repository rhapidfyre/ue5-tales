// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gas/Contexts/VitalityEffectContext.h"



UScriptStruct* FVitalityEffectContext::GetScriptStruct() const
{
	return FGameplayEffectContext::GetScriptStruct();
}

FVitalityEffectContext* FVitalityEffectContext::Duplicate() const
{
	FVitalityEffectContext* NewContext = new FVitalityEffectContext();
	*NewContext = *this;
	NewContext->AddActors(Actors);
	if (GetHitResult())
	{
		// Deep Copy
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

bool FVitalityEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	uint8 RepBits = 0;
	if (Ar.IsSaving())
	{
		if (bReplicateInstigator && Instigator.IsValid())
		{
			RepBits |= 1 << 0;
		}
		if (bReplicateEffectCauser && EffectCauser.IsValid() )
		{
			RepBits |= 1 << 1;
		}
		if (AbilityCDO.IsValid())
		{
			RepBits |= 1 << 2;
		}
		if (bReplicateSourceObject && SourceObject.IsValid())
		{
			RepBits |= 1 << 3;
		}
		if (Actors.Num() > 0)
		{
			RepBits |= 1 << 4;
		}
		if (HitResult.IsValid())
		{
			RepBits |= 1 << 5;
		}
		if (bHasWorldOrigin)
		{
			RepBits |= 1 << 6;
		}
		if (bIsCriticalHit)
		{
			RepBits |= 1 << 7;
		}
		if (bIsLuckyHit)
		{
			RepBits |= 1 << 8;
		}
	}

	Ar.SerializeBits(&RepBits, 9);

	if (RepBits & (1 << 0))
	{
		Ar << Instigator;
	}
	if (RepBits & (1 << 1))
	{
		Ar << EffectCauser;
	}
	if (RepBits & (1 << 2))
	{
		Ar << AbilityCDO;
	}
	if (RepBits & (1 << 3))
	{
		Ar << SourceObject;
	}
	if (RepBits & (1 << 4))
	{
		SafeNetSerializeTArray_Default<31>(Ar, Actors);
	}
	if (RepBits & (1 << 5))
	{
		if (Ar.IsLoading())
		{
			if (!HitResult.IsValid())
			{
				HitResult = TSharedPtr<FHitResult>(new FHitResult());
			}
		}
		HitResult->NetSerialize(Ar, Map, bOutSuccess);
	}
	if (RepBits & (1 << 6))
	{
		Ar << WorldOrigin;
		bHasWorldOrigin = true;
	}
	else
	{
		bHasWorldOrigin = false;
	}
	if (RepBits & (1 << 7))
	{
		Ar << bIsCriticalHit;
	}
	if (RepBits & (1 << 8))
	{
		Ar << bIsLuckyHit;
	}

	if (Ar.IsLoading())
	{
		// Just to initialize InstigatorAbilitySystemComponent
		AddInstigator(Instigator.Get(), EffectCauser.Get()); 
	}	
	
	bOutSuccess = true;
	return true;
}
