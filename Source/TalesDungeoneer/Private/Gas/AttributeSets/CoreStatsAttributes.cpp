// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Gas/AttributeSets/CoreStatsAttributes.h"

#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"


UE_DEFINE_GAMEPLAY_TAG(TAG_Stats,				"Stats")
UE_DEFINE_GAMEPLAY_TAG(TAG_Stats_Strength,		"Stats.Strength")
UE_DEFINE_GAMEPLAY_TAG(TAG_Stats_Dexterity,		"Stats.Dexterity")
UE_DEFINE_GAMEPLAY_TAG(TAG_Stats_Fortitude,		"Stats.Fortitude")
UE_DEFINE_GAMEPLAY_TAG(TAG_Stats_Astuteness,	"Stats.Astuteness")
UE_DEFINE_GAMEPLAY_TAG(TAG_Stats_Intellect,		"Stats.Intellect")
UE_DEFINE_GAMEPLAY_TAG(TAG_Stats_Charisma,		"Stats.Charisma")


UCoreStatsAttributes::UCoreStatsAttributes() :
	Strength(STAT_DEFAULT),  Dexterity(STAT_DEFAULT),
	Fortitude(STAT_DEFAULT), Astuteness(STAT_DEFAULT),
	Intellect(STAT_DEFAULT), Charisma(STAT_DEFAULT)
{
	
}

TArray<FGameplayAttribute> UCoreStatsAttributes::GetAllCoreStatAttributes() const
{
	return {
		GetStrengthAttribute(), GetDexterityAttribute(), GetFortitudeAttribute(),
		GetAstutenessAttribute(), GetIntellectAttribute(), GetCharismaAttribute()
	};
}

void UCoreStatsAttributes::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

void UCoreStatsAttributes::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttributeOnChange(Attribute, NewValue);
}

void UCoreStatsAttributes::ClampAttributeOnChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	// Allow some stats to go below zero
	float minAttributeValue = 0.f;
	if (GetDexterityAttribute() == Attribute)
		{ minAttributeValue = STAT_MIN; }
	else if (GetCharismaAttribute() == Attribute)
		{ minAttributeValue = STAT_MIN; }
	
	// Cap all core stats at 1,000
	NewValue = FMath::Clamp(NewValue, minAttributeValue, STAT_MAX);
}

void UCoreStatsAttributes::OnRep_Strength(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetStrength();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Strength Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreStatsAttributes, Strength, OldData);
}

void UCoreStatsAttributes::OnRep_Dexterity(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetDexterity();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Dexterity Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreStatsAttributes, Dexterity, OldData);
}

void UCoreStatsAttributes::OnRep_Fortitude(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetFortitude();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Fortitude Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreStatsAttributes, Fortitude, OldData);
}

void UCoreStatsAttributes::OnRep_Astuteness(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetAstuteness();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Astuteness Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreStatsAttributes, Astuteness, OldData);
}

void UCoreStatsAttributes::OnRep_Intellect(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetIntellect();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Intellect Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreStatsAttributes, Intellect, OldData);
}

void UCoreStatsAttributes::OnRep_Charisma(const FGameplayAttributeData& OldData)
{
	float oldValue = OldData.GetCurrentValue();
	float newValue = GetCharisma();
	UE_LOGFMT(LogAbilitySystemComponent, VeryVerbose,  "{Name}({Authority}) REPNOTIFY: "
		"Charisma Updated. Was {OldValue}, Now {NewValue}", GetName(),
		GetOwningActor()->HasAuthority()?"SRV":"CLI", oldValue, newValue);
	GAMEPLAYATTRIBUTE_REPNOTIFY(UCoreStatsAttributes, Charisma, OldData);
}


////////////////////////////////////////////////////////////////////////////////
//	REPLICATION
////////////////////////////////////////////////////////////////////////////////

void UCoreStatsAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Vitality Attributes
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreStatsAttributes, Strength,		COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreStatsAttributes, Dexterity,		COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreStatsAttributes, Fortitude,		COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreStatsAttributes, Astuteness,	COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreStatsAttributes, Intellect,		COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UCoreStatsAttributes, Charisma,		COND_OwnerOnly, REPNOTIFY_Always);

}