// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Gas/AttributeSets/VitalityAttributes.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"

UVitalityAttributes::UVitalityAttributes()
{
	
}

void UVitalityAttributes::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
}

void UVitalityAttributes::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
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