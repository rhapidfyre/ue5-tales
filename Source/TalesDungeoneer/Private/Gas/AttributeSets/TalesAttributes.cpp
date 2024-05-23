// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gas/AttributeSets/TalesAttributes.h"
#include "Gas/Contexts/VitalityEffectContext.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

#include "GameplayEffectExtension.h"	// For:		const FGameplayEffectModCallbackData& Data


UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Flag_IgnoreArmor, "Damage.Flag.IgnoreArmor")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Flag_IgnoreHealth, "Damage.Flag.IgnoreHealth")


UTalesAttributes::UTalesAttributes()
{
	VitalityAttributes  = CreateDefaultSubobject<UVitalityAttributes>("VitalityAttributes");
	CoreStatsAttributes = CreateDefaultSubobject<UCoreStatsAttributes>("CoreStatsAttributes");
	EffectAttributes    = CreateDefaultSubobject<UEffectAttributes>("EffectAttributes");
}

/**
 *  Called just before a GameplayEffect is executed to modify the base value
 *  of an attribute. No more changes can be made.
 * \param Data Contains the data from the effect post execution
 */
void UTalesAttributes::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == DamageAttributes->GetIncomingDamageAttribute())
	{
		// Saves the damage to local variable, then clears the class member
		float inDamage = DamageAttributes->GetIncomingDamage();
		DamageAttributes->SetIncomingDamage(0.f);

		// Do nothing if the damage is extremely minuscule
		if (FMath::IsNearlyZero(inDamage, 0.001))
		{
			return;
		}

		// If the character is dead, we're done
		if (GetIsDead())
		{
			return;
		}

		const FGameplayEffectSpec EffectSpec = Data.EffectSpec;

		// Hit armor first
		const FGameplayTag tagIgnoreArmor(TAG_Damage_Flag_IgnoreArmor.GetTag());
		const bool ignoreArmor = EffectSpec.CapturedSourceTags.GetSpecTags().HasTagExact(tagIgnoreArmor);
		if (VitalityAttributes->GetCurrentArmor() > 0.f && !ignoreArmor)
		{
			float inDamageToArmor = inDamage;
			float newArmor = VitalityAttributes->GetCurrentArmor();
			
			// Attempts to reduce the armor to zero and not negative
			const float armorDiff = FMath::Min(newArmor, inDamageToArmor);
			inDamage -= armorDiff; // Reduce the incoming damage
			newArmor -= armorDiff;
			VitalityAttributes->SetCurrentArmor(
				FMath::Clamp( newArmor, 0.f, VitalityAttributes->GetMaximumArmor() ));
			
			// This hit just depleted the armor
			if (VitalityAttributes->GetCurrentArmor() <= 0.f && !bArmorDepleted)
			{
				const FGameplayEffectContextHandle EffectContext = EffectSpec.GetEffectContext();
				AActor* EffectInstigator = EffectContext.GetOriginalInstigator();
				AActor* EffectCauser     = EffectContext.GetEffectCauser();

				UE_LOGFMT(LogAbilitySystemComponent, Display,
					"{Name}({Authority}): Armor Depleted by Hit from {Instigator} for {Damage}",
					GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
					EffectInstigator->GetName(), armorDiff);
				
				if (OnHealthDepleted.IsBound())
				{
				
					OnArmorDepleted.Broadcast(EffectInstigator, EffectCauser,
						EffectSpec, Data.EvaluatedData.Magnitude);
				}
			}
			bArmorDepleted = (VitalityAttributes->GetCurrentArmor() <= 0.f);
		}

		// Repeat process but for health
		const FGameplayTag tagIgnoreHealth(TAG_Damage_Flag_IgnoreHealth.GetTag());
		const bool ignoreHealth = EffectSpec.CapturedSourceTags.GetSpecTags().HasTagExact(tagIgnoreHealth);
		if (inDamage > 0.f && !ignoreHealth)
		{
			float inDamageToHealth = inDamage;
			const float newHealth = VitalityAttributes->GetCurrentHealth() - inDamageToHealth;

			// Allow health to go up to 15% below zero.
			// Where zero is unconscious and -15% is death.
			VitalityAttributes->SetCurrentHealth(FMath::Clamp(newHealth,
				VitalityAttributes->GetMaximumHealth()*(-0.15),
				VitalityAttributes->GetMaximumHealth()));

			const float finalHealth = VitalityAttributes->GetCurrentHealth();

			// Health is now below or at zero
			if (finalHealth <= 0.f && !bHealthDepleted && !bDead)
			{
				const FGameplayEffectContextHandle EffectContext = EffectSpec.GetEffectContext();
				AActor* EffectInstigator = EffectContext.GetOriginalInstigator();
				AActor* EffectCauser     = EffectContext.GetEffectCauser();

				UE_LOGFMT(LogAbilitySystemComponent, Display,
					"{Name}({Authority}): Health Depleted by Hit from {Instigator} for {Damage}",
					GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
					EffectInstigator->GetName(), inDamageToHealth);
				
				if (OnHealthDepleted.IsBound())
				{
					OnHealthDepleted.Broadcast(EffectInstigator, EffectCauser,
						Data.EffectSpec, Data.EvaluatedData.Magnitude);
				}
			}
			bHealthDepleted = (finalHealth <= 0.f);
			bDead = (finalHealth <= VitalityAttributes->GetDeathHealthValue());
		}

		if (OnDamageTaken.IsBound())
		{
			const FGameplayEffectContextHandle& ContextHandle = Data.EffectSpec.GetEffectContext();
			AActor* InstigatingActor = ContextHandle.GetOriginalInstigator();
			AActor* CausingActor     = ContextHandle.GetEffectCauser();

			bool isCrit	 = false;
			bool isLucky = false;
			const FVitalityEffectContext* EffectContext =
				static_cast<FVitalityEffectContext*>(Data.EffectSpec.GetContext().Get());
				
			if (EffectContext != nullptr)
			{
				isCrit = EffectContext->IsCriticalHit();
				isLucky = EffectContext->IsLuckyHit();
			}
			OnDamageTaken.Broadcast(InstigatingActor, CausingActor,
				Data.EffectSpec.CapturedSourceTags.GetSpecTags(),
				Data.EvaluatedData.Magnitude, isCrit, isLucky);
		}
	}
}

