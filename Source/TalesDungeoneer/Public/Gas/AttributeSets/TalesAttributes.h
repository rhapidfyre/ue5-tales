// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "EffectAttributes.h"
#include "Delegates/Delegate.h"
#include "../AttributeHelpers.h"
#include "NativeGameplayTags.h"

#include "VitalityAttributes.h"
#include "CoreStatsAttributes.h"
#include "DamageAttributes.h"

#include "TalesAttributes.generated.h"


UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Flag_IgnoreArmor)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Flag_IgnoreHealth)


DECLARE_MULTICAST_DELEGATE_FourParams(FOnAttributeEvent,
	AActor*,						// Effect Instigator (what called the effect)
	AActor*,						// Effect Causer (what dun it)
	const FGameplayEffectSpec&, 	// the effect spec
	float							// Effect magnitude
	);

DECLARE_MULTICAST_DELEGATE_SixParams(FOnAttributeDamageEvent,
	AActor*,						// Effect Instigator (what called the effect)
	AActor*,						// Effect Causer (what dun it)
	const FGameplayTagContainer&,	// Tag Container
	float,							// Effect magnitude
	bool,							// bIsCriticalHit
	bool							// bIsLuckyHit
	);

/**
 * This is the primary attribute set that is to be included
 * into the ability system component
 */
UCLASS(BlueprintType)
class TALESDUNGEONEER_API UTalesAttributes : public UAttributeSet
{
	GENERATED_BODY()
	
public:

	UTalesAttributes();

	mutable FOnAttributeEvent OnHealthDepleted;		// Called when health hits zero
	mutable FOnAttributeEvent OnArmorDepleted;  	// Called when armor hits zero
	mutable FOnAttributeDamageEvent OnDamageTaken;	// Called whenever damage is dealt

	
	UFUNCTION(BlueprintCallable)
	bool GetIsUnconscious() const { return bHealthDepleted && !bDead; }

	UFUNCTION(BlueprintCallable)
	bool GetIsDead() const { return bDead; }

	UPROPERTY() UVitalityAttributes* VitalityAttributes;
	UPROPERTY() UEffectAttributes* EffectAttributes;
	UPROPERTY() UCoreStatsAttributes* CoreStatsAttributes;
	UPROPERTY() UDamageAttributes* DamageAttributes;

protected:

	virtual void PostGameplayEffectExecute(
		const FGameplayEffectModCallbackData& Data) override;

private:

	bool bDead			 = false;
	bool bHealthDepleted = false;
	bool bArmorDepleted  = false;
	bool bStarving       = false;
	bool bDehydrated     = false;
	
};
