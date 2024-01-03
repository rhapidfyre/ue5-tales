// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Gas/AttributeSets/VitalityAttributes.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

UVitalityAttributes::UVitalityAttributes() :
	CurrentHealth(40.f), CurrentStamina(0.f), CurrentArmorClass(0.f),
	CurrentArmor(100.f), CurrentMagic(0.f), CurrentHunger(1000.f),
	CurrentHydration(1000.f), MaximumHealth(100.f), MaximumArmorClass(100.f),
	MaximumArmor(100.f), MaximumMagic(100.f), MaximumStamina(100.f),
	MaximumHunger(1000.f), MaximumHydration(1000.f),
	PassiveHealthRegen(1.f), PassiveMagicRegen(0.25), PassiveStaminaRegen(2.f),
	PassiveHungerDrain(0.016), PassiveHydroDrain(0.035), Ammunition(0.f)
{
	
}

TArray<FGameplayAttribute> UVitalityAttributes::GetAllVitalityAttributes() const
{
	return {
		GetCurrentHealthAttribute(),	GetMaximumHealthAttribute(),
		GetCurrentArmorAttribute(),		GetMaximumArmorAttribute(),
		GetCurrentStaminaAttribute(),	GetMaximumStaminaAttribute(),
		GetCurrentMagicAttribute(),		GetMaximumMagicAttribute(),
		GetCurrentHungerAttribute(),	GetMaximumHungerAttribute(),
		GetCurrentHydrationAttribute(),	GetMaximumHydrationAttribute()
	};
}

void UVitalityAttributes::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

void UVitalityAttributes::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

void UVitalityAttributes::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	float maxAttributeValue;
	if (GetCurrentHealthAttribute() == Attribute)
	{
		maxAttributeValue = GetMaximumHealth();
	}
	else if (GetCurrentArmorAttribute() == Attribute)
	{
		maxAttributeValue = GetMaximumArmor();
	}
	else if (GetCurrentArmorClassAttribute() == Attribute)
	{
		maxAttributeValue = GetMaximumArmorClass();
	}
	else if (GetCurrentHungerAttribute() == Attribute)
	{
		maxAttributeValue = GetMaximumHunger();
	}
	else if (GetCurrentHydrationAttribute() == Attribute)
	{
		maxAttributeValue = GetMaximumHydration();
	}
	else
	{
		return;
	}
	NewValue = FMath::Clamp(NewValue, 0.f, maxAttributeValue);
}

void UVitalityAttributes::OnRep_CurrentHealth(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetCurrentHealth();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Current Health Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
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

void UVitalityAttributes::OnRep_PassiveHealthRegen(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetPassiveHealthRegen();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Passive Health Regen Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, PassiveHealthRegen, OldData);
}

void UVitalityAttributes::OnRep_PassiveStaminaRegen(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetPassiveStaminaRegen();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Passive Stamina Regen Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, PassiveStaminaRegen, OldData);
}

void UVitalityAttributes::OnRep_PassiveMagicRegen(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetPassiveMagicRegen();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Passive Magic Regen Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, PassiveMagicRegen, OldData);
}

void UVitalityAttributes::OnRep_PassiveHungerDrain(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetPassiveHungerDrain();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Passive Hunger Drain Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, PassiveHungerDrain, OldData);
}

void UVitalityAttributes::OnRep_PassiveHydroDrain(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetPassiveHydroDrain();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Passive Hydration Drain Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVitalityAttributes, PassiveHydroDrain, OldData);
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


////////////////////////////////////////////////////////////////////////////////
//	REPLICATION
////////////////////////////////////////////////////////////////////////////////

void UVitalityAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Vitality Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, CurrentHealth,		COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, CurrentArmor,		COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, CurrentArmorClass,	COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, CurrentMagic,		COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, CurrentStamina,		COND_None, 		REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, CurrentHunger,		COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, CurrentHydration,	COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, PassiveHealthRegen,	COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, PassiveStaminaRegen,COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, PassiveMagicRegen,	COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, PassiveHungerDrain,	COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, PassiveHydroDrain,	COND_OwnerOnly, REPNOTIFY_Always);

	// Attribute Maximums
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, MaximumHealth,		COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, MaximumArmor,		COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, MaximumArmorClass,	COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, MaximumMagic,		COND_None, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, MaximumStamina,		COND_None, 		REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, MaximumHunger,		COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, MaximumHydration,	COND_OwnerOnly, REPNOTIFY_Always);

	// Ammo Count
	DOREPLIFETIME_CONDITION_NOTIFY(UVitalityAttributes, Ammunition, COND_None, REPNOTIFY_Always);
}