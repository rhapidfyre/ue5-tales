// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Characters/Components/TalesAbilityComponent.h"

#include "Characters/CharacterBase.h"
#include "DataAssets/CharacterDefaults.h"
#include "Gamemode/AdventureMode/TalesPlayerStateBase.h"
#include "Gas/AttributeSets/CoreStatsAttributes.h"
#include "Gas/AttributeSets/DamageAttributes.h"
#include "Gas/AttributeSets/EffectAttributes.h"

UTalesAbilityComponent::UTalesAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FGameplayTag UTalesAbilityComponent::GetCharacterRace() const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))	{ return CharacterBase->GetCharacterRace(); }
	return TAG_Character_Race_Human.GetTag();
}

FGameplayTag UTalesAbilityComponent::GetCharacterClass() const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))	{ return CharacterBase->GetCharacterClass(); }
	return TAG_Character_Class_Warrior.GetTag();
}

/**
 * Recalculates all of the core stats, damage resists, and damage bonuses
 * given stats, race and class. Does not factor for equipment or effects,
 * which are handled at the time damage is taken or dealt, or equipment is
 * changed.
 */
void UTalesAbilityComponent::PerformTotalRecalculation()
{
	RecalculateCoreStats();
	RecalculateDamageResists();
	RecalculateDamageBonuses();
}

/**
 * Calculates what the characters core stats values should be, accounting for
 * base stat values, racial stats and class stats. This must be preformed, at a
 * minimum, when the character is restored/loaded.
 */
void UTalesAbilityComponent::RecalculateCoreStats()
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	
	const ATalesPlayerStateBase* PlayerStateBase = Cast<ATalesPlayerStateBase>(CharacterBase->GetPlayerState());
	if (!IsValid(PlayerStateBase)) { return; }
	
	const FGameplayTag ClassTag = CharacterBase->GetCharacterClass();
	const UCharacterClassData* ClassData = Cast<UCharacterClassData>(PlayerStateBase->GetClassDataAsset(ClassTag));
	if (!IsValid(ClassData)) { return; }
	
	const FGameplayTag RaceTag = CharacterBase->GetCharacterRace();
	const UCharacterRaceData* RaceData = Cast<UCharacterRaceData>(PlayerStateBase->GetRaceDataAsset(RaceTag));
	if (!IsValid(RaceData)) { return; }

	// Calculate as we go, so we only set it a single time and notify delegates once
	float NewStrength   = 100.f, NewDexterity = 100.f, NewFortitude = 100.f;
	float NewAstuteness = 100.f, NewIntellect = 100.f, NewCharisma  = 100.f;
	
	UCoreStatsAttributes* CoreStats		= CharacterBase->AttributeCoreStatsSet;
	//UDamageAttributes* DamageStats		= CharacterBase->AttributeDamageSet;
	//UVitalityAttributes* VitalityStats	= CharacterBase->AttributeVitalitySet;
	
	for (const TPair<FGameplayAttribute,int>& dataModifier : ClassData->CoreStatsModifiers)
	{
		if (dataModifier.Key == CoreStats->GetStrengthAttribute())	{NewStrength   += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetDexterityAttribute())	{NewDexterity  += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetFortitudeAttribute())	{NewFortitude  += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetAstutenessAttribute()){NewAstuteness += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetIntellectAttribute())	{NewIntellect  += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetCharismaAttribute())	{NewCharisma   += dataModifier.Value;}
	}
	
	for (const TPair<FGameplayAttribute,int>& dataModifier : RaceData->CoreStatsModifiers)
	{
		if (dataModifier.Key == CoreStats->GetStrengthAttribute())	{NewStrength   += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetDexterityAttribute())	{NewDexterity  += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetFortitudeAttribute())	{NewFortitude  += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetAstutenessAttribute()){NewAstuteness += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetIntellectAttribute())	{NewIntellect  += dataModifier.Value;}
		if (dataModifier.Key == CoreStats->GetCharismaAttribute())	{NewCharisma   += dataModifier.Value;}
	}

	// Update the new values
	CoreStats->SetStrength(NewStrength);
	CoreStats->SetDexterity(NewDexterity);
	CoreStats->SetFortitude(NewFortitude);
	CoreStats->SetAstuteness(NewAstuteness);
	CoreStats->SetIntellect(NewIntellect);
	CoreStats->SetCharisma(NewCharisma);
	
}

/**
 * Calculates what the characters base damage resist should be, accounting for
 * core stats, racial resist and class resist. This does NOT calculate resists
 * granted for effects or equipment, which are calculated at the time damage is
 * taken.
 */
void UTalesAbilityComponent::RecalculateDamageResists()
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	
	const ATalesPlayerStateBase* PlayerStateBase = Cast<ATalesPlayerStateBase>(CharacterBase->GetPlayerState());
	if (!IsValid(PlayerStateBase)) { return; }
	
	const FGameplayTag ClassTag = CharacterBase->GetCharacterClass();
	const UCharacterClassData* ClassData = Cast<UCharacterClassData>(PlayerStateBase->GetClassDataAsset(ClassTag));
	if (!IsValid(ClassData)) { return; }
	
	const FGameplayTag RaceTag = CharacterBase->GetCharacterRace();
	const UCharacterRaceData* RaceData = Cast<UCharacterRaceData>(PlayerStateBase->GetRaceDataAsset(RaceTag));
	if (!IsValid(RaceData)) { return; }
	
	const UCoreStatsAttributes* CoreStats	= CharacterBase->AttributeCoreStatsSet;
	UDamageAttributes* DamageStats			= CharacterBase->AttributeDamageSet;
	//UVitalityAttributes* VitalityStats	= CharacterBase->AttributeVitalitySet;

	// Fortitude must be calculated or this won't work
	const float TotalFortitude = CoreStats->GetFortitude();
	float NewBaseResistance = 0.f;	float NewResistBlunt  = 0.f;	float NewResistSlash  = 0.f;
	float NewResistPierce = 0.f;	float NewResistFire   = 0.f;	float NewResistFrost  = 0.f;
	float NewResistAcid   = 0.f;	float NewResistShock  = 0.f;	float NewResistRadio  = 0.f;
	float NewResistSonic  = 0.f;	float NewResistHoly   = 0.f;	float NewResistDark   = 0.f;
	
	for (const TPair<FGameplayAttribute,int>& dataModifier : ClassData->DamageResistModifiers)
	{
		if (dataModifier.Key == DamageStats->GetBluntResistanceAttribute())	{NewResistBlunt  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSlashResistanceAttribute())	{NewResistSlash  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetPierceResistanceAttribute()){NewResistPierce += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFireResistanceAttribute())	{NewResistFire   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFrostResistanceAttribute())	{NewResistFrost  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetAcidResistanceAttribute())	{NewResistAcid   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetShockResistanceAttribute()) {NewResistShock  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetRadioResistanceAttribute()) {NewResistRadio  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSonicResistanceAttribute()) {NewResistSonic  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetHolyResistanceAttribute()) 	{NewResistHoly   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetDarkResistanceAttribute()) 	{NewResistDark   += dataModifier.Value;}
	}
	
	for (const TPair<FGameplayAttribute,int>& dataModifier : RaceData->DamageResistModifiers)
	{
		if (dataModifier.Key == DamageStats->GetBluntResistanceAttribute())	{NewResistBlunt  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSlashResistanceAttribute())	{NewResistSlash  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetPierceResistanceAttribute()){NewResistPierce += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFireResistanceAttribute())	{NewResistFire   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFrostResistanceAttribute())	{NewResistFrost  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetAcidResistanceAttribute())	{NewResistAcid   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetShockResistanceAttribute()) {NewResistShock  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetRadioResistanceAttribute()) {NewResistRadio  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSonicResistanceAttribute()) {NewResistSonic  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetHolyResistanceAttribute()) 	{NewResistHoly   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetDarkResistanceAttribute()) 	{NewResistDark   += dataModifier.Value;}
	}

	const int fMultiplier = TotalFortitude >= 0 ? 1 : -1;
	
	// For every 10 fortitude, resists will change by 5
	const float ModifiedFortitude = FMath::Floor(TotalFortitude / BonusMultiple) * fMultiplier;
	NewBaseResistance += (BonusModifier * ModifiedFortitude);
	
	DamageStats->SetBluntResistance(NewBaseResistance	+ NewResistBlunt);
	DamageStats->SetSlashResistance(NewBaseResistance	+ NewResistSlash);
	DamageStats->SetPierceResistance(NewBaseResistance	+ NewResistPierce);
	
	DamageStats->SetFireResistance(NewBaseResistance		+ NewResistFire);
	DamageStats->SetFrostResistance(NewBaseResistance	+ NewResistFrost);
	DamageStats->SetAcidResistance(NewBaseResistance		+ NewResistAcid);
	DamageStats->SetShockResistance(NewBaseResistance	+ NewResistShock);
	DamageStats->SetRadioResistance(NewBaseResistance	+ NewResistRadio);
	DamageStats->SetSonicResistance(NewBaseResistance	+ NewResistSonic);
	DamageStats->SetHolyResistance(NewBaseResistance		+ NewResistHoly);
	DamageStats->SetDarkResistance(NewBaseResistance		+ NewResistDark);
	
}

/**
 * Calculates what the characters base damage bonus should be, accounting for
 * core stats, racial bonuses and class bonuses. This does NOT calculate bonuses
 * granted for effects or equipment, which are calculated at the time damage is
 * handled.
 */
void UTalesAbilityComponent::RecalculateDamageBonuses()
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (!IsValid(CharacterBase)) { return; }
	
	const ATalesPlayerStateBase* PlayerStateBase = Cast<ATalesPlayerStateBase>(CharacterBase->GetPlayerState());
	if (!IsValid(PlayerStateBase)) { return; }
	
	const FGameplayTag ClassTag = CharacterBase->GetCharacterClass();
	const UCharacterClassData* ClassData = Cast<UCharacterClassData>(PlayerStateBase->GetClassDataAsset(ClassTag));
	if (!IsValid(ClassData)) { return; }
	
	const FGameplayTag RaceTag = CharacterBase->GetCharacterRace();
	const UCharacterRaceData* RaceData = Cast<UCharacterRaceData>(PlayerStateBase->GetRaceDataAsset(RaceTag));
	if (!IsValid(RaceData)) { return; }
	
	const UCoreStatsAttributes* CoreStats	= CharacterBase->AttributeCoreStatsSet;
	UDamageAttributes* DamageStats			= CharacterBase->AttributeDamageSet;
	UEffectAttributes* EffectAttributes     = CharacterBase->AttributeEffectSet;

	// CoreStats must be calculated
	const float TotalStrength  = CoreStats->GetStrength();
	const float TotalIntellect = CoreStats->GetIntellect();
	float NewBaseBonus = 0.f;	float NewStrBonus	 = 0.f; float NewBonusBlunt  = 0.f;	float NewBonusSlash  = 0.f;
	float NewBonusPierce = 0.f;	float NewBonusFire   = 0.f;	float NewBonusFrost  = 0.f;
	float NewBonusAcid   = 0.f;	float NewBonusShock  = 0.f;	float NewBonusRadio  = 0.f;
	float NewBonusSonic  = 0.f;	float NewBonusHoly   = 0.f;	float NewBonusDark   = 0.f;
	
	for (const TPair<FGameplayAttribute,int>& dataModifier : ClassData->DamageBonusModifiers)
	{
		if (dataModifier.Key == DamageStats->GetBluntBonusAttribute())	{NewBonusBlunt  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSlashBonusAttribute())	{NewBonusSlash  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetPierceBonusAttribute()){NewBonusPierce += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFireBonusAttribute())	{NewBonusFire   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFrostBonusAttribute())	{NewBonusFrost  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetAcidBonusAttribute())	{NewBonusAcid   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetShockBonusAttribute()) {NewBonusShock  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetRadioBonusAttribute()) {NewBonusRadio  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSonicBonusAttribute()) {NewBonusSonic  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetHolyBonusAttribute()) 	{NewBonusHoly   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetDarkBonusAttribute()) 	{NewBonusDark   += dataModifier.Value;}
	}
	
	for (const TPair<FGameplayAttribute,int>& dataModifier : RaceData->DamageBonusModifiers)
	{
		if (dataModifier.Key == DamageStats->GetBluntBonusAttribute())	{NewBonusBlunt  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSlashBonusAttribute())	{NewBonusSlash  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetPierceBonusAttribute()){NewBonusPierce += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFireBonusAttribute())	{NewBonusFire   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetFrostBonusAttribute())	{NewBonusFrost  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetAcidBonusAttribute())	{NewBonusAcid   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetShockBonusAttribute()) {NewBonusShock  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetRadioBonusAttribute()) {NewBonusRadio  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetSonicBonusAttribute()) {NewBonusSonic  += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetHolyBonusAttribute()) 	{NewBonusHoly   += dataModifier.Value;}
		if (dataModifier.Key == DamageStats->GetDarkBonusAttribute()) 	{NewBonusDark   += dataModifier.Value;}
	}

	const int sMultiplier = TotalStrength	>= 0 ? 1 : -1;
	const int iMultiplier = TotalIntellect	>= 0 ? 1 : -1;
	
	// For every BonusMultiple strength, melee bonuses will increase by BonusModifier
	// For every BonusMultiple fortitude, non-melee bonuses will change by BonusModifier
	const float ModifiedStrength  = FMath::Floor(TotalStrength / BonusMultiple)  * sMultiplier;
	const float ModifiedIntellect = FMath::Floor(TotalIntellect / BonusMultiple) * iMultiplier;
	NewStrBonus  += (BonusModifier * ModifiedStrength);
	NewBaseBonus += (BonusModifier * ModifiedIntellect);
	
	DamageStats->SetBluntBonus(NewStrBonus	+ NewBonusBlunt);
	DamageStats->SetSlashBonus(NewStrBonus	+ NewBonusSlash);
	DamageStats->SetPierceBonus(NewStrBonus	+ NewBonusPierce);
	
	DamageStats->SetFireBonus(NewBaseBonus	+ NewBonusFire);
	DamageStats->SetFrostBonus(NewBaseBonus	+ NewBonusFrost);
	DamageStats->SetAcidBonus(NewBaseBonus	+ NewBonusAcid);
	DamageStats->SetShockBonus(NewBaseBonus	+ NewBonusShock);
	DamageStats->SetRadioBonus(NewBaseBonus	+ NewBonusRadio);
	DamageStats->SetSonicBonus(NewBaseBonus	+ NewBonusSonic);
	DamageStats->SetHolyBonus(NewBaseBonus	+ NewBonusHoly);
	DamageStats->SetDarkBonus(NewBaseBonus	+ NewBonusDark);
}

