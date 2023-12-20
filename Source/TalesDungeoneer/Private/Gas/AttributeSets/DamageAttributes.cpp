// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "Gas/AttributeSets/DamageAttributes.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"


UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_SetByCaller,			"Damage.SetByCaller")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_World,				"Damage.World")

UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Generic,		"Damage.Physical.Generic")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Blunt, 		"Damage.Physical.Blunt")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Slash, 		"Damage.Physical.Slash")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Pierce,		"Damage.Physical.Pierce")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Bite, 		"Damage.Physical.Bite")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Kick, 		"Damage.Physical.Kick")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Claw, 		"Damage.Physical.Claw")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Physical_Sting, 		"Damage.Physical.Sting")

UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Elemental_Generic,		"Damage.Elemental.Generic")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Elemental_Fire,			"Damage.Elemental.Fire")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Elemental_Frost,			"Damage.Elemental.Frost")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Elemental_Acid,			"Damage.Elemental.Acid")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Elemental_Shock,			"Damage.Elemental.Shock")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Elemental_Radioactive,	"Damage.Elemental.Radioactive")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Elemental_Sonic,			"Damage.Elemental.Sonic")

UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Magic_Generic,		"Damage.Magic.Generic")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Magic_Holy,			"Damage.Magic.Holy")
UE_DEFINE_GAMEPLAY_TAG(TAG_Damage_Magic_Dark,			"Damage.Magic.Dark")


UDamageAttributes::UDamageAttributes() :
	IncomingDamage(0.f), CriticalChance(0.f), CriticalMultiplier(1.f),
	LuckyChance(0.f), DamageModifier(0.f), DamageMultiplier(1.f),

	BluntResistance(0.f), SlashResistance(0.f), PierceResistance(0.f),
	BiteResistance(0.f), KickResistance(0.f), ClawResistance(0.f),
	StingResistance(0.f),FireResistance(0.f), FrostResistance(0.f),
	AcidResistance(0.f), ShockResistance(0.f), RadioResistance(0.f),
	SonicResistance(0.f), HolyResistance(0.f), DarkResistance(0.f),

	BluntBonus(0.f), SlashBonus(0.f), PierceBonus(0.f),
	BiteBonus(0.f), KickBonus(0.f), ClawBonus(0.f),
	StingBonus(0.f),FireBonus(0.f), FrostBonus(0.f),
	AcidBonus(0.f), ShockBonus(0.f), RadioBonus(0.f),
	SonicBonus(0.f), HolyBonus(0.f), DarkBonus(0.f)
{

}


void UDamageAttributes::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

void UDamageAttributes::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

void UDamageAttributes::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// All resistances and bonuses are capped at -1,000 to 1,000
	//  where negative is a vulnerability/handicap and positive is resistance/bonus
	NewValue = FMath::Clamp(NewValue, -1000.f, 1000.f);
}


void UDamageAttributes::OnRep_IncomingDamage(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetIncomingDamage();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Incoming Damage Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, IncomingDamage, OldData);
}

void UDamageAttributes::OnRep_CriticalChance(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCriticalChance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Critical Chance Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, CriticalChance, OldData);
}

void UDamageAttributes::OnRep_CriticalMultiplier(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetCriticalMultiplier();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Critical Multiplier Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, CriticalMultiplier, OldData);
}

void UDamageAttributes::OnRep_LuckyChance(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetLuckyChance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Lucky Chance Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, LuckyChance, OldData);
}

void UDamageAttributes::OnRep_DamageModifier(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetDamageModifier();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Damage Modifier Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, DamageModifier, OldData);
}

void UDamageAttributes::OnRep_DamageMultiplier(const FGameplayAttributeData& OldData)
{
	const float oldValue = OldData.GetCurrentValue();
	const float newValue = GetDamageMultiplier();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Damage Multiplier Updated. Was {OldValue}, Now {NewValue}",
		GetName(), GetOwningActor()->HasAuthority()?"SRV":"CLI",
		oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, DamageMultiplier, OldData);
}


void UDamageAttributes::OnRep_BluntResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetBluntResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Blunt Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, BluntResistance, OldData);
}

void UDamageAttributes::OnRep_SlashResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetSlashResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Slash Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, SlashResistance, OldData);
}

void UDamageAttributes::OnRep_PierceResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetPierceResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Piercing Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, PierceResistance, OldData);
}

void UDamageAttributes::OnRep_FireResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetFireResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Fire Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, FireResistance, OldData);
}

void UDamageAttributes::OnRep_FrostResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetFrostResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Frost Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, FrostResistance, OldData);
}

void UDamageAttributes::OnRep_AcidResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetAcidResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Acid Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, AcidResistance, OldData);
}

void UDamageAttributes::OnRep_ShockResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetShockResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Shock Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, ShockResistance, OldData);
}

void UDamageAttributes::OnRep_RadioResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetRadioResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Radio Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, RadioResistance, OldData);
}

void UDamageAttributes::OnRep_SonicResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetSonicResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Sonic Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, SonicResistance, OldData);
}

void UDamageAttributes::OnRep_HolyResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetHolyResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Holy Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, HolyResistance, OldData);
}

void UDamageAttributes::OnRep_DarkResistance(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetDarkResistance();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Dark Resistance Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, DarkResistance, OldData);
}


void UDamageAttributes::OnRep_BluntBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetBluntBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Blunt Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, BluntBonus, OldData);
}

void UDamageAttributes::OnRep_SlashBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetSlashBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Slashing Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, SlashBonus, OldData);
}

void UDamageAttributes::OnRep_PierceBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetPierceBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Piercing Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, PierceBonus, OldData);
}

void UDamageAttributes::OnRep_FireBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetFireBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Fire Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, FireBonus, OldData);
}

void UDamageAttributes::OnRep_FrostBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetFrostBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Frost Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, FrostBonus, OldData);
}

void UDamageAttributes::OnRep_AcidBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetAcidBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Acid Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, AcidBonus, OldData);
}

void UDamageAttributes::OnRep_ShockBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetShockBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Shock Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, ShockBonus, OldData);
}

void UDamageAttributes::OnRep_RadioBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetRadioBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Radioactive Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, RadioBonus, OldData);
}

void UDamageAttributes::OnRep_SonicBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetSonicBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Sonic Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, SonicBonus, OldData);
}

void UDamageAttributes::OnRep_HolyBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetHolyBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Holy Magic Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, HolyBonus, OldData);
}

void UDamageAttributes::OnRep_DarkBonus(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetDarkBonus();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Dark Magic Bonus Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UDamageAttributes, DarkBonus, OldData);
}


////////////////////////////////////////////////////////////////////////////////
//	REPLICATION
////////////////////////////////////////////////////////////////////////////////

void UDamageAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Damage Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, CriticalChance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, CriticalMultiplier,	COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, LuckyChance,			COND_OwnerOnly, 	REPNOTIFY_Always);
	
	// Resistance Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, BluntResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, SlashResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, PierceResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, FireResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, FrostResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, AcidResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, ShockResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, RadioResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, SonicResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, HolyResistance,		COND_OwnerOnly, 	REPNOTIFY_Always);

	// Damage Bonus Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, BluntBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, SlashBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, PierceBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, FireBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, FrostBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, AcidBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, ShockBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, RadioBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, SonicBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, HolyBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UDamageAttributes, DarkBonus,		COND_OwnerOnly, 		REPNOTIFY_Always);
}
