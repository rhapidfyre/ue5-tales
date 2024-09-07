// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "Characters/Components/TalesAbilityComponent.h"

#include "Characters/CharacterBase.h"
#include "DataAssets/AbilityDataAsset.h"
#include "DataAssets/CharacterDefaults.h"
#include "Engine/AssetManager.h"
#include "Gamemode/AdventureMode/TalesPlayerStateBase.h"
#include "Gas/AttributeSets/CoreStatsAttributes.h"
#include "Gas/AttributeSets/DamageAttributes.h"
#include "Gas/AttributeSets/VitalityAttributes.h"

#include "lib/Damage/TalesDamageTypes.h"


UTalesAbilityComponent::UTalesAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

FString UTalesAbilityComponent::ConvertGameplayTagToString(const FGameplayTag& GameplayTag)
{
	FString TagString = GameplayTag.ToString();
	TArray<FString> ParsedTags;
	TagString.ParseIntoArray(ParsedTags, TEXT("."), true);
	return ParsedTags.Num() > 0 ? ParsedTags.Last() : "UNKNOWN";
}

/**
 * Re-enables the ability system to perform calculations.
 */
void UTalesAbilityComponent::EnableStatCalculation(bool bCalculateNow)
{
	bAllowCalculation = true;
	if (bCalculateNow)
		{ PerformTotalRecalculation(); }
}

void UTalesAbilityComponent::DisableStatCalculation()
{
	bAllowCalculation = false;
}

FGameplayTag UTalesAbilityComponent::GetCharacterRace() const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))	{ return CharacterBase->GetCharacterRace(); }
	return TAG_Character_Race_Human.GetTag();
}

FString UTalesAbilityComponent::GetCharacterRaceAsString() const
{
	FGameplayTag CharacterRaceTag = GetCharacterRace();
	if (CharacterRaceTag.MatchesTag(TAG_Character_Race.GetTag()))
	{
		FString TagString = CharacterRaceTag.ToString();
		TArray<FString> ParsedTags;
		TagString.ParseIntoArray(ParsedTags, TEXT("."), true);
		return ParsedTags.Num() > 0 ? ParsedTags.Last() : "UNKNOWN";
	}
	return "UNKNOWN";
}

FGameplayTag UTalesAbilityComponent::GetCharacterClass() const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase))	{ return CharacterBase->GetCharacterClass(); }
	return TAG_Character_Class_Warrior.GetTag();
}

FString UTalesAbilityComponent::GetCharacterClassAsString() const
{
	FGameplayTag CharacterClassTag = GetCharacterClass();
	if (CharacterClassTag.MatchesTag(TAG_Character_Class.GetTag()))
	{
		FString TagString = CharacterClassTag.ToString();
		TArray<FString> ParsedTags;
		TagString.ParseIntoArray(ParsedTags, TEXT("."), true);
		return ParsedTags.Num() > 0 ? ParsedTags.Last() : "UNKNOWN";
	}
	return "UNKNOWN";
}

/**
 * \brief Returns an array that contains all eligible abilities for this character's race and class combination
 * \return Array of all abilities eligible for this class
 */
TArray<UPrimaryAbilityDataAsset*> UTalesAbilityComponent::GetAllClassAbilities() const
{
	const FGameplayTag CharacterRace = GetCharacterRace();
	const FGameplayTag CharacterClass = GetCharacterClass();
	TArray<UPrimaryAbilityDataAsset*> Abilities;
	UAssetManager& AssetManager = UAssetManager::Get();

	FPrimaryAssetType AssetType = UPrimaryAbilityDataAsset::StaticClass()->GetFName();
	TArray<FPrimaryAssetId> AssetIdsList;
	AssetManager.GetPrimaryAssetIdList(AssetType, AssetIdsList);

	for (const FPrimaryAssetId& AssetId : AssetIdsList)
	{
		FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
		UPrimaryAbilityDataAsset* LoadedAbility = Streamable.LoadSynchronous
			<UPrimaryAbilityDataAsset>(AssetManager.GetPrimaryAssetPath(AssetId));

		if (IsValid(LoadedAbility))
		{
			UTalesGameplayAbility* AbilityData = LoadedAbility->GetAbilityReference();
			if (AbilityData->AbilityRaces.HasTagExact(CharacterRace))
			{
				bool bIsAllClasses = AbilityData->AbilityClasses.Contains(TAG_Character_Class.GetTag());
				if (bIsAllClasses || AbilityData->AbilityClasses.Contains(CharacterClass))
				{
					Abilities.Add(LoadedAbility);
				}
			}
		}
	}
	return Abilities;
}

float UTalesAbilityComponent::GetCoreStatByTag(const FGameplayTag& StatTag) const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase)) { return 0.f; }

	const UCoreStatsAttributes* CoreStats = CharacterBase->AttributeCoreStatsSet;

	// Get the initial value of the Core Stat
	float statValue = 0.f;
	if (StatTag == TAG_Stats_Strength)        { statValue = CoreStats->GetStrength();   }
	else if (StatTag == TAG_Stats_Dexterity)  { statValue = CoreStats->GetDexterity();  }
	else if (StatTag == TAG_Stats_Fortitude)  { statValue = CoreStats->GetFortitude();  }
	else if (StatTag == TAG_Stats_Astuteness) { statValue = CoreStats->GetAstuteness(); }
	else if (StatTag == TAG_Stats_Intellect)  { statValue = CoreStats->GetIntellect();  }
	else if (StatTag == TAG_Stats_Charisma)   { statValue = CoreStats->GetCharisma();   }

	// Return the total value of the stat
	return statValue;
}


/**
 * Gets the character's total damage resistance from all factors,
 * in real time, as it is when the method is called.
 * \param DamageTag The TAG_Damage to look for
 * \return The total value of the base damage, effects damage, and equipment damage value
 */
float UTalesAbilityComponent::GetDamageResistanceByTag(const FGameplayTag& DamageTag) const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase)) { return 0.f; }

	const UDamageAttributes* DamageSet = CharacterBase->AttributeDamageSet;
	float damageResistance = 0.f;

	     if (DamageTag == TAG_Damage_Physical)				{ damageResistance = DamageSet->GetNaturalResistance();}
	else if (DamageTag == TAG_Damage_Physical_Blunt) 		{ damageResistance = DamageSet->GetBluntResistance();}
	else if (DamageTag == TAG_Damage_Physical_Slash) 		{ damageResistance = DamageSet->GetSlashResistance();}
	else if (DamageTag == TAG_Damage_Physical_Pierce)		{ damageResistance = DamageSet->GetPierceResistance();}
	else if (DamageTag == TAG_Damage_Physical_Bite) 		{ damageResistance = DamageSet->GetBiteResistance();}
	else if (DamageTag == TAG_Damage_Physical_Kick) 		{ damageResistance = DamageSet->GetKickResistance();}
	else if (DamageTag == TAG_Damage_Physical_Claw) 		{ damageResistance = DamageSet->GetClawResistance();}
	else if (DamageTag == TAG_Damage_Physical_Sting)		{ damageResistance = DamageSet->GetStingResistance();}
	else if (DamageTag == TAG_Damage_Elemental) 			{ damageResistance = DamageSet->GetNaturalResistance();}
	else if (DamageTag == TAG_Damage_Elemental_Fire)		{ damageResistance = DamageSet->GetFireResistance();}
	else if (DamageTag == TAG_Damage_Elemental_Frost)		{ damageResistance = DamageSet->GetFrostResistance();}
	else if (DamageTag == TAG_Damage_Elemental_Acid)		{ damageResistance = DamageSet->GetAcidResistance();}
	else if (DamageTag == TAG_Damage_Elemental_Shock)		{ damageResistance = DamageSet->GetShockResistance();}
	else if (DamageTag == TAG_Damage_Elemental_Radioactive) { damageResistance = DamageSet->GetRadioResistance();}
	else if (DamageTag == TAG_Damage_Elemental_Sonic)		{ damageResistance = DamageSet->GetSonicResistance();}
	else if (DamageTag == TAG_Damage_Magic)					{ damageResistance = DamageSet->GetNaturalResistance();}
	else if (DamageTag == TAG_Damage_Magic_Holy) 			{ damageResistance = DamageSet->GetHolyResistance();}
	else if (DamageTag == TAG_Damage_Magic_DarkMagic) 		{ damageResistance = DamageSet->GetDarkResistance();}

	return damageResistance;
}

/**
 * Gets the character's total damage bonus from all factors,
 * in real time, as it is when the method is called.
 * \param DamageTag The TAG_Damage to look for
 * \return The total value of the base damage, effects damage, and equipment damage value
 */
float UTalesAbilityComponent::GetDamageBonusByTag(const FGameplayTag& DamageTag) const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
	if (IsValid(CharacterBase)) { return 0.f; }

	const UDamageAttributes* DamageSet = CharacterBase->AttributeDamageSet;
	float damageBonus = 0.f;

	     if (DamageTag == TAG_Damage_Physical)				{ damageBonus = DamageSet->GetNaturalBonus();}
	else if (DamageTag == TAG_Damage_Physical_Blunt) 		{ damageBonus = DamageSet->GetBluntBonus();}
	else if (DamageTag == TAG_Damage_Physical_Slash) 		{ damageBonus = DamageSet->GetSlashBonus();}
	else if (DamageTag == TAG_Damage_Physical_Pierce)		{ damageBonus = DamageSet->GetPierceBonus();}
	else if (DamageTag == TAG_Damage_Physical_Bite) 		{ damageBonus = DamageSet->GetBiteBonus();}
	else if (DamageTag == TAG_Damage_Physical_Kick) 		{ damageBonus = DamageSet->GetKickBonus();}
	else if (DamageTag == TAG_Damage_Physical_Claw) 		{ damageBonus = DamageSet->GetClawBonus();}
	else if (DamageTag == TAG_Damage_Physical_Sting)		{ damageBonus = DamageSet->GetStingBonus();}
	else if (DamageTag == TAG_Damage_Elemental) 			{ damageBonus = DamageSet->GetNaturalBonus();}
	else if (DamageTag == TAG_Damage_Elemental_Fire)		{ damageBonus = DamageSet->GetFireBonus();}
	else if (DamageTag == TAG_Damage_Elemental_Frost)		{ damageBonus = DamageSet->GetFrostBonus();}
	else if (DamageTag == TAG_Damage_Elemental_Acid)		{ damageBonus = DamageSet->GetAcidBonus();}
	else if (DamageTag == TAG_Damage_Elemental_Shock)		{ damageBonus = DamageSet->GetShockBonus();}
	else if (DamageTag == TAG_Damage_Elemental_Radioactive) { damageBonus = DamageSet->GetRadioBonus();}
	else if (DamageTag == TAG_Damage_Elemental_Sonic)		{ damageBonus = DamageSet->GetSonicBonus();}
	else if (DamageTag == TAG_Damage_Magic)					{ damageBonus = DamageSet->GetNaturalBonus();}
	else if (DamageTag == TAG_Damage_Magic_Holy) 			{ damageBonus = DamageSet->GetHolyBonus();}
	else if (DamageTag == TAG_Damage_Magic_DarkMagic) 		{ damageBonus = DamageSet->GetDarkBonus();}

	return damageBonus;
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
	float NewStrength   = STAT_DEFAULT, NewDexterity = STAT_DEFAULT, NewFortitude = STAT_DEFAULT;
	float NewAstuteness = STAT_DEFAULT, NewIntellect = STAT_DEFAULT, NewCharisma  = STAT_DEFAULT;

	UCoreStatsAttributes* CoreStats		= CharacterBase->AttributeCoreStatsSet;

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

FGameplayTag UTalesAbilityComponent::GetGameplayTagFromGameplayAttribute(
	const FGameplayAttribute& GameplayAttribute) const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetOwner());
	if (!IsValid(CharacterBase)) { return {}; }

	const UCoreStatsAttributes* CoreStats = CharacterBase->AttributeCoreStatsSet;
	if (GameplayAttribute == CoreStats->GetStrengthAttribute())		{ return TAG_Stats_Strength; }
	if (GameplayAttribute == CoreStats->GetFortitudeAttribute())    { return TAG_Stats_Fortitude; }
	if (GameplayAttribute == CoreStats->GetDexterityAttribute())    { return TAG_Stats_Dexterity; }
	if (GameplayAttribute == CoreStats->GetIntellectAttribute())    { return TAG_Stats_Intellect; }
	if (GameplayAttribute == CoreStats->GetAstutenessAttribute())	{ return TAG_Stats_Astuteness; }
	if (GameplayAttribute == CoreStats->GetCharismaAttribute())		{ return TAG_Stats_Charisma; }

	const UDamageAttributes* DamageAttributes = CharacterBase->AttributeDamageSet;
	if (GameplayAttribute == DamageAttributes->GetNaturalBonusAttribute())		{ return TAG_Damage_World; }
	if (GameplayAttribute == DamageAttributes->GetNaturalResistanceAttribute()) { return TAG_Damage_World; }
	if (GameplayAttribute == DamageAttributes->GetBluntBonusAttribute())		{ return TAG_Damage_Physical_Blunt; }
	if (GameplayAttribute == DamageAttributes->GetBluntResistanceAttribute())	{ return TAG_Damage_Physical_Blunt; }
	if (GameplayAttribute == DamageAttributes->GetSlashBonusAttribute())		{ return TAG_Damage_Physical_Slash; }
	if (GameplayAttribute == DamageAttributes->GetSlashResistanceAttribute())	{ return TAG_Damage_Physical_Slash; }
	if (GameplayAttribute == DamageAttributes->GetPierceBonusAttribute())		{ return TAG_Damage_Physical_Pierce; }
	if (GameplayAttribute == DamageAttributes->GetPierceResistanceAttribute())	{ return TAG_Damage_Physical_Pierce; }
	if (GameplayAttribute == DamageAttributes->GetBiteBonusAttribute())			{ return TAG_Damage_Physical_Bite; }
	if (GameplayAttribute == DamageAttributes->GetBiteResistanceAttribute())	{ return TAG_Damage_Physical_Bite; }
	if (GameplayAttribute == DamageAttributes->GetKickBonusAttribute())			{ return TAG_Damage_Physical_Kick; }
	if (GameplayAttribute == DamageAttributes->GetKickResistanceAttribute())	{ return TAG_Damage_Physical_Kick; }
	if (GameplayAttribute == DamageAttributes->GetClawBonusAttribute())			{ return TAG_Damage_Physical_Claw; }
	if (GameplayAttribute == DamageAttributes->GetClawResistanceAttribute())	{ return TAG_Damage_Physical_Claw; }
	if (GameplayAttribute == DamageAttributes->GetStingBonusAttribute())		{ return TAG_Damage_Physical_Sting; }
	if (GameplayAttribute == DamageAttributes->GetStingResistanceAttribute())	{ return TAG_Damage_Physical_Sting; }
	if (GameplayAttribute == DamageAttributes->GetFireBonusAttribute())			{ return TAG_Damage_Elemental_Fire; }
	if (GameplayAttribute == DamageAttributes->GetFireResistanceAttribute())	{ return TAG_Damage_Elemental_Fire; }
	if (GameplayAttribute == DamageAttributes->GetFrostBonusAttribute())		{ return TAG_Damage_Elemental_Frost; }
	if (GameplayAttribute == DamageAttributes->GetFrostResistanceAttribute())	{ return TAG_Damage_Elemental_Frost; }
	if (GameplayAttribute == DamageAttributes->GetAcidBonusAttribute())			{ return TAG_Damage_Elemental_Acid; }
	if (GameplayAttribute == DamageAttributes->GetAcidResistanceAttribute())	{ return TAG_Damage_Elemental_Acid; }
	if (GameplayAttribute == DamageAttributes->GetShockBonusAttribute())		{ return TAG_Damage_Elemental_Shock; }
	if (GameplayAttribute == DamageAttributes->GetShockResistanceAttribute())	{ return TAG_Damage_Elemental_Shock; }
	if (GameplayAttribute == DamageAttributes->GetRadioBonusAttribute())		{ return TAG_Damage_Elemental_Radioactive; }
	if (GameplayAttribute == DamageAttributes->GetRadioResistanceAttribute())	{ return TAG_Damage_Elemental_Radioactive; }
	if (GameplayAttribute == DamageAttributes->GetSonicBonusAttribute())		{ return TAG_Damage_Elemental_Sonic; }
	if (GameplayAttribute == DamageAttributes->GetSonicResistanceAttribute())	{ return TAG_Damage_Elemental_Sonic; }
	if (GameplayAttribute == DamageAttributes->GetHolyBonusAttribute())			{ return TAG_Damage_Magic_Holy; }
	if (GameplayAttribute == DamageAttributes->GetHolyResistanceAttribute())	{ return TAG_Damage_Magic_Holy; }
	if (GameplayAttribute == DamageAttributes->GetDarkBonusAttribute())			{ return TAG_Damage_Magic_DarkMagic; }
	if (GameplayAttribute == DamageAttributes->GetDarkResistanceAttribute())	{ return TAG_Damage_Magic_DarkMagic; }

	//const UVitalityAttributes* VitalityStats = CharacterBase->AttributeVitalitySet;
	//if (GameplayAttribute == VitalityStats->GetAmmunitionAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetCurrentArmorAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetCurrentHealthAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetCurrentHungerAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetCurrentHydrationAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetCurrentMagicAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetCurrentStaminaAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetMaximumArmorAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetMaximumHealthAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetMaximumHungerAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetMaximumHydrationAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetMaximumMagicAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetMaximumStaminaAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetPassiveHealthRegenAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetPassiveHungerDrainAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetPassiveHydroDrainAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetPassiveMagicRegenAttribute()) { return TAG_Damage; }
	//if (GameplayAttribute == VitalityStats->GetPassiveStaminaRegenAttribute()) { return TAG_Damage; }

	//const UEffectAttributes* EffectStats = CharacterBase->AttributeEffectSet;

	return {};
}

FGameplayAttribute UTalesAbilityComponent::GetGameplayAttributeFromGameplayTag(
	const FGameplayTag& GameplayTag, bool bIsDamageResist) const
{
	const ACharacterBase* CharacterBase = Cast<ACharacterBase>(GetOwner());
	if (!IsValid(CharacterBase)) { return {}; }

	const UCoreStatsAttributes* CoreStats = CharacterBase->AttributeCoreStatsSet;
	if (GameplayTag == TAG_Stats_Strength)	{ return CoreStats->GetStrengthAttribute(); }
	if (GameplayTag == TAG_Stats_Fortitude)	{ return CoreStats->GetFortitudeAttribute(); }
	if (GameplayTag == TAG_Stats_Dexterity)	{ return CoreStats->GetDexterityAttribute(); }
	if (GameplayTag == TAG_Stats_Intellect)	{ return CoreStats->GetIntellectAttribute(); }
	if (GameplayTag == TAG_Stats_Astuteness)	{ return CoreStats->GetAstutenessAttribute(); }
	if (GameplayTag == TAG_Stats_Charisma)	{ return CoreStats->GetCharismaAttribute(); }

	const UDamageAttributes* DamageAttributes = CharacterBase->AttributeDamageSet;
	if (bIsDamageResist)
	{
		if (GameplayTag == TAG_Damage_World) { return DamageAttributes->GetNaturalResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Blunt) { return DamageAttributes->GetBluntResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Slash) { return DamageAttributes->GetSlashResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Pierce) { return DamageAttributes->GetPierceResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Claw) { return DamageAttributes->GetClawResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Sting) { return DamageAttributes->GetStingResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Fire) { return DamageAttributes->GetFireResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Frost) { return DamageAttributes->GetFrostResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Acid) { return DamageAttributes->GetAcidResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Shock) { return DamageAttributes->GetShockResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Radioactive) { return DamageAttributes->GetRadioResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Sonic) { return DamageAttributes->GetSonicResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Magic_Holy) { return DamageAttributes->GetHolyResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Magic_DarkMagic) { return DamageAttributes->GetDarkResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Kick) { return DamageAttributes->GetKickResistanceAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Bite) { return DamageAttributes->GetBiteResistanceAttribute(); }
	}
	else
	{
		if (GameplayTag == TAG_Damage_World) { return DamageAttributes->GetNaturalBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Blunt) { return DamageAttributes->GetBluntBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Slash) { return DamageAttributes->GetSlashBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Pierce) { return DamageAttributes->GetPierceBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Bite) { return DamageAttributes->GetBiteBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Kick) { return DamageAttributes->GetKickBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Claw) { return DamageAttributes->GetClawBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Physical_Sting) { return DamageAttributes->GetStingBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Fire) { return DamageAttributes->GetFireBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Frost) { return DamageAttributes->GetFrostBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Acid) { return DamageAttributes->GetAcidBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Shock) { return DamageAttributes->GetShockBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Radioactive) { return DamageAttributes->GetRadioBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Elemental_Sonic) { return DamageAttributes->GetSonicBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Magic_Holy) { return DamageAttributes->GetHolyBonusAttribute(); }
		if (GameplayTag == TAG_Damage_Magic_DarkMagic) { return DamageAttributes->GetDarkBonusAttribute(); }
	}
	return {};
}
