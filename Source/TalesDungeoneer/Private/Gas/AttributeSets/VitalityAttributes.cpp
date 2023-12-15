// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Gas/AttributeSets/VitalityAttributes.h"
#include "GameplayEffectExtension.h"	// For:		const FGameplayEffectModCallbackData& Data
#include "Gas/Contexts/VitalityEffectContext.h"

#include "Logging/StructuredLog.h"


UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Flag_IgnoreArmor, "Damage.Flag.IgnoreArmor")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Flag_IgnoreHealth, "Damage.Flag.IgnoreHealth")


UVitalityAttributes::UVitalityAttributes()
{
	
}

/**
 *	This is called just before any modification happens to an attribute's base value when an attribute aggregator exists.
 *	This function should enforce clamping (presuming you wish to clamp the base value along with the final value in PreAttributeChange)
 *	This function should NOT invoke gameplay related events or callbacks. Do those in PreAttributeChange() which will be called prior to the
 *	final value of the attribute actually changing.
 * @param Attribute the attribute about to be modified
 * @param NewValue the new value that is about to be modified
 */
void UVitalityAttributes::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

/**
 *  Called just before any modification happens to an attribute. This is lower level than PreAttributeModify/PostAttribute modify.
 *	There is no additional context provided here since anything can trigger this. Executed effects, duration based effects, effects being removed, immunity being applied, stacking rules changing, etc.
 *	This function is meant to enforce things like "Health = Clamp(Health, 0, MaxHealth)" and NOT things like "trigger this extra thing if damage is applied, etc".
 * @param Attribute the attribute about to be modified
 * @param NewValue the new value that is about to be modified, passed by reference
 */
void UVitalityAttributes::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

/**
 *  Performs clamping for specific attributes to be within a certain range.
 * @param Attribute The attribute that is being checked
 * @param NewValue The value being proposed, by reference (to be modified)
 */
void UVitalityAttributes::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	float maxAttributeValue;
	switch(Attribute)
	{
	case GetCurrentArmorClassAttribute():
		maxAttributeValue = GetMaximumArmorClass();
		break;
	case GetCurrentArmorAttribute():
		maxAttributeValue = GetMaximumArmor();
		break;
	case GetCurrentHungerAttribute():
		maxAttributeValue = GetMaximumHunger();
		break;
	case GetCurrentHydrationAttribute():
		maxAttributeValue = GetMaximumHydration();
		break;
	default:
		return; // Do not perform clamping
	}
	NewValue = FMath::Clamp(NewValue, 0.f, maxAttributeValue);
}

/**
 *  Called just before a GameplayEffect is executed to modify the base value
 *  of an attribute. No more changes can be made.
 * @param Data Contains the data from the effect post execution
 */
void UVitalityAttributes::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		// Saves the damage to local variable, then clears the class member
		float inDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);

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
		if (GetCurrentArmor() > 0.f && !ignoreArmor)
		{
			float inDamageToArmor = inDamage;
			float newArmor = GetCurrentArmor();
			
			// Attempts to reduce the armor to zero and not negative
			const float armorDiff = FMath::Min(newArmor, inDamageToArmor);
			inDamage -= armorDiff; // Reduce the incoming damage
			newArmor -= armorDiff;
			SetCurrentArmor(FMath::Clamp(newArmor, 0.f, GetMaximumArmor()));

			// This hit just depleted the armor
			if (GetCurrentArmor() <= 0.f && !bArmorDepleted)
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
			bArmorDepleted = (GetCurrentArmor() <= 0.f);
		}

		// Repeat process but for health
		const FGameplayTag tagIgnoreHealth(TAG_Damage_Flag_IgnoreHealth.GetTag());
		const bool ignoreHealth = EffectSpec.CapturedSourceTags.GetSpecTags().HasTagExact(tagIgnoreHealth);
		if (inDamage > 0.f && !ignoreHealth)
		{
			float inDamageToHealth = inDamage;
			const float newHealth = GetCurrentHealth() - inDamageToHealth;

			// Allow health to go up to 15% below zero.
			// Where zero is unconscious and -15% is death.
			SetCurrentHealth(FMath::Clamp(newHealth,
				GetMaximumHealth()*(-0.15), GetMaximumHealth()));

			const float finalHealth = GetCurrentHealth();

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
			bDead = (finalHealth <= GetDeathHealthValue());
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


////////////////////////////////////////////////////////////////////////////////
//	REPLICATION
////////////////////////////////////////////////////////////////////////////////


void UVitalityAttributes::OnRep_CurrentHealth(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetCurrentHealth();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Health Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CurrentHealth, OldData);
}

void UVitalityAttributes::OnRep_CurrentArmorClass(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetCurrentArmorClass();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Armor Class Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CurrentArmorClass, OldData);
}

void UVitalityAttributes::OnRep_CurrentArmor(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCurrentArmor();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Armor Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CurrentArmor, OldData);
}

void UVitalityAttributes::OnRep_CurrentMagic(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCurrentMagic();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Magic Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CurrentMagic, OldData);
}

void UVitalityAttributes::OnRep_CurrentStamina(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCurrentStamina();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Stamina Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CurrentStamina, OldData);
}

void UVitalityAttributes::OnRep_CurrentHunger(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCurrentHunger();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Hunger Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CurrentHunger, OldData);
}

void UVitalityAttributes::OnRep_CurrentHydration(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCurrentHydration();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Hydration Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CurrentHydration, OldData);
}

void UVitalityAttributes::OnRep_MaximumHealth(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetMaximumHealth();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Maximum Health Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, MaximumHealth, OldData);
}

void UVitalityAttributes::OnRep_MaximumArmorClass(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetMaximumArmorClass();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Maximum Armor Class Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, MaximumArmorClass, OldData);
}

void UVitalityAttributes::OnRep_MaximumArmor(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetMaximumArmor();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Maximum Armor Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, MaximumArmor, OldData);
}

void UVitalityAttributes::OnRep_MaximumMagic(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetMaximumMagic();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Maximum Magic Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, MaximumMagic, OldData);
}

void UVitalityAttributes::OnRep_MaximumStamina(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetMaximumStamina();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Maximum Stamina Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, MaximumStamina, OldData);
}

void UVitalityAttributes::OnRep_MaximumHunger(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetMaximumHunger();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Maximum Hunger Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, MaximumHunger, OldData);
}

void UVitalityAttributes::OnRep_MaximumHydration(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetMaximumHydration();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Maximum Hydration Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, MaximumHydration, OldData);
}

void UVitalityAttributes::OnRep_IncomingDamage(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetIncomingDamage();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Incoming Damage Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, IncomingDamage, OldData);
}

void UVitalityAttributes::OnRep_CriticalChance(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCriticalChance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Critical Chance Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CriticalChance, OldData);
}

void UVitalityAttributes::OnRep_CriticalMultiplier(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCriticalMultiplier();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Critical Multiplier Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, CriticalMultiplier, OldData);
}

void UVitalityAttributes::OnRep_LuckyChance(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetLuckyChance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Lucky Chance Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, LuckyChance, OldData);
}

void UVitalityAttributes::OnRep_DamageModifier(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetDamageModifier();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Damage Modifier Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, DamageModifier, OldData);
}

void UVitalityAttributes::OnRep_DamageMultiplier(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetDamageMultiplier();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Damage Multiplier Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, DamageMultiplier, OldData);
}

void UVitalityAttributes::OnRep_Ammunition(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetAmmunition();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Ammunition Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, Ammunition, OldData);
}
