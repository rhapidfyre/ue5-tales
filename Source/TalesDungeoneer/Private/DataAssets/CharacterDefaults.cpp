// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "DataAssets/CharacterDefaults.h"
#include "Gas/Abilities/TalesGameplayAbility.h"
#include "GameplayAbilities/Public/GameplayEffect.h"
#include "Logging/StructuredLog.h"


UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Sex_Female,	"Character.Sex.Female");
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Sex_Male,		"Character.Sex.Male");
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Sex_Nonbinary, "Character.Sex.Nonbinary");


/**
 * Gets a random eye color if argument is < 0
 * \param EyeColorIndex Optional index of the requested eye color
 * \return The FLinearColor for the eye material
 */
FLinearColor UCharacterRaceData::GetEyeColor(int EyeColorIndex) const
{
	if (EyeColorOptions.IsValidIndex(EyeColorIndex))
	{
		return EyeColorOptions[EyeColorIndex];
	}
	return EyeColorOptions[ FMath::RandRange(0, EyeColorOptions.Num()-1) ];
}
/**
 * Gets a random skin color if argument is < 0
 * \param SkinColorIndex Optional index of the requested skin color
 * \return The FLinearColor for the skin material
 */
FLinearColor UCharacterRaceData::GetSkinColor(int SkinColorIndex) const
{
	if (SkinColorOptions.IsValidIndex(SkinColorIndex))
	{
		return SkinColorOptions[SkinColorIndex];
	}
	return SkinColorOptions[ FMath::RandRange(0, SkinColorOptions.Num()-1) ];
}
/**
 * Gets a random hair color if argument is < 0
 * \param HairColorIndex Optional index of the requested hair color
 * \return The FLinearColor for the hair material
 */
FLinearColor UCharacterRaceData::GetHairColor(int HairColorIndex) const
{
	if (HairColorOptions.IsValidIndex(HairColorIndex))
	{
		return HairColorOptions[HairColorIndex];
	}
	return HairColorOptions[ FMath::RandRange(0, HairColorOptions.Num()-1) ];
}
/**
 * Gets a random beard color if argument is < 0
 * \param BeardColorIndex Optional index of the requested beard color
 * \return The FLinearColor for the beard material
 */
FLinearColor UCharacterRaceData::GetBeardColor(int BeardColorIndex) const
{
	if (BeardColorOptions.IsValidIndex(BeardColorIndex))
	{
		return BeardColorOptions[BeardColorIndex];
	}
	return BeardColorOptions[ FMath::RandRange(0, BeardColorOptions.Num()-1) ];
}

UPrimaryCharacterData::UPrimaryCharacterData()
	: CharacterDataAsset(nullptr), CharacterRaceData(nullptr), CharacterClassData(nullptr) 
{
	
}

/**
 * Returns a random skin color from the racial skin color options
 * \return The chosen skin color
 */
FLinearColor UPrimaryCharacterData::GetCharacterSkinColor() const
{
	TArray<FLinearColor> SkinColors = {};
	if (IsValid(CharacterRaceData))
	{
		if (!CharacterRaceData->SkinColorOptions.IsEmpty())
		{
			for (const FLinearColor& SkinColor : CharacterRaceData->SkinColorOptions)
			{
				SkinColors.Add(SkinColor);
			}
		}
	}
	if (SkinColors.Num() > 0)
	{
		const int randomIndex = FMath::RandRange(0, SkinColors.Num()-1);
		const FLinearColor ChosenColor = SkinColors[randomIndex];
		UE_LOGFMT(LogAssetData, Display, "GetCharacterSkinColor(): Out of {NumOptions} - Chose: '{ChosenOption}'",
			SkinColors.Num(), ChosenColor.ToString());
		return ChosenColor;
	}
	UE_LOGFMT(LogAssetData, Warning, "GetCharacterSkinColor(): No options available. Chosing default.");
	return FLinearColor::White;
}

/**
 * Returns a random skeleton & animation instance from the skeleton options.
 * Will return a feminine or masculine skeleton if one boolean is true.
 * Will disregard sex is both booleans are true or both false.
 * \param bForceMasculine If TRUE, only masculine options will be considered
 * \param bForceFeminine If TRUE, only feminine options will be considered
 * \return The mesh and anim instance being used by this character
 */
FSkeletonOptionsData UPrimaryCharacterData::GetCharacterSkeleton(
		const bool bForceMasculine, const bool bForceFeminine) const
{
	TSet<FSkeletonOptionsData> SkeletonOptions = {};
	if (IsValid(CharacterRaceData))
	{
		const bool bIsAgender =
			   ( bForceFeminine &&  bForceMasculine)
			|| (!bForceFeminine && !bForceMasculine);
		
		for (const FSkeletonOptionsData& SkeletonOption : CharacterRaceData->SkeletonOptions)
		{
			if (bIsAgender) {SkeletonOptions.Add(SkeletonOption);}
			else
			{
				if (SkeletonOption.OptionTags.HasTag(TAG_Character_Sex_Female))
				{
					if (bForceFeminine) { SkeletonOptions.Add(SkeletonOption); }
				}
				else
				{
					if (!bForceFeminine) { SkeletonOptions.Add(SkeletonOption); }
				}
			}
		}
		
	}
	
	TArray<FSkeletonOptionsData> SkeletonArray = SkeletonOptions.Array();
	if (SkeletonArray.Num() > 0)
	{
		const int randomIndex = FMath::RandRange(0, SkeletonArray.Num()-1);
		FSkeletonOptionsData ChosenOption = SkeletonArray[randomIndex];
	
		UE_LOGFMT(LogAssetData, Display, "GetCharacterSkeleton(): Out of {NumOptions} - Chose: '{cSkeleton}, {cAnim}'",
			SkeletonArray.Num(), ChosenOption.Skeleton->GetName(), ChosenOption.AnimInstance->GetName());
	
		return ChosenOption;
	}
	UE_LOGFMT(LogAssetData, Warning, "GetCharacterSkinColor(): No options available. Chosing default.");
	return FSkeletonOptionsData();
}

/**
 * Returns one of the EligibleClass tags in CharacterClassData
 * If the array is empty, it will return Warrior as the default.
 * \return One of the eligible class tags, or Character.Class.Warrior
 */
FGameplayTag UPrimaryCharacterData::GetCharacterClass() const
{
	if (IsValid(CharacterRaceData))
	{
		if (!CharacterRaceData->EligibleClasses.IsEmpty())
		{
			const FGameplayTagContainer tags = CharacterRaceData->EligibleClasses;
			const int randomIndex = FMath::RandRange(0, tags.Num()-1);

			const FGameplayTag ChosenTag = tags.IsValidIndex(randomIndex)
				 ? tags.GetByIndex(randomIndex) : tags.First();
			
			UE_LOGFMT(LogAssetData, Display,
				"GetCharacterClass(): Class Chosen = '{ChosenOption}'", ChosenTag.ToString());
			return ChosenTag;
		}
	}
	const FGameplayTag DefaultTag = TAG_Character_Class_Warrior;
	UE_LOGFMT(LogAssetData, Display,
		"GetCharacterClass(): No class options found. Defaulting to {DefaultTag}", DefaultTag.ToString());
	return DefaultTag;
}


/**
 * Returns one of the EligibleRace tags in CharacterRaceData
 * If the array is empty, it will return Warrior as the default.
 * \return One of the eligible race tags, or Character.Race.Human
 */
FGameplayTag UPrimaryCharacterData::GetCharacterRace() const
{
	if (IsValid(CharacterDataAsset))
	{
		if (!CharacterDataAsset->EligibleRaces.IsEmpty())
		{
			const FGameplayTagContainer tags = CharacterDataAsset->EligibleRaces;
			const int randomIndex = FMath::RandRange(0, tags.Num()-1);
			
			const FGameplayTag ChosenTag = tags.IsValidIndex(randomIndex)
				 ? tags.GetByIndex(randomIndex) : tags.First();
			UE_LOGFMT(LogAssetData, Display,
				"GetCharacterRace(): Race Chosen = '{ChosenOption}'", ChosenTag.ToString());
			return ChosenTag;
		}
	}
	const FGameplayTag DefaultTag = TAG_Character_Race_Human;
	UE_LOGFMT(LogAssetData, Display,
		"GetCharacterRace(): No race options found. Defaulting to {DefaultTag}", DefaultTag.ToString());
	return DefaultTag;
}

/**
 * Returns a TMap of all core stats and their values.
 * \return The TMap, either empty or populated with valid core stats.
 */
TMap<FGameplayAttribute, float> UPrimaryCharacterData::GetDefaultCoreStats() const
{
	TMap<FGameplayAttribute, float> DamageBonusMappings = {};
	// Loops through CharacterData, Race & Class assets for all damage resistances
	TArray<UCharacterCommonData*> LoopAssets = {CharacterDataAsset, CharacterRaceData, CharacterClassData};
	for (const UCharacterCommonData* DataAsset : LoopAssets)
	{
		if (IsValid(DataAsset))
		{
			for (auto const& [CoreStatKey, CoreStatValue] : DataAsset->CoreStatsModifiers)
			{
				UE_LOGFMT(LogAssetData, Display,
					"Found Default '{BaseAbility} = {BaseValue}' in Data Asset '{DataAsset}'",
						CoreStatKey.GetName(), CoreStatValue, DataAsset->GetName());
				if (!DamageBonusMappings.Contains(CoreStatKey))
				{
					DamageBonusMappings.Add(CoreStatKey, CoreStatValue);
				}
				DamageBonusMappings[CoreStatKey] += CoreStatValue;
			}
		}
	}
	return DamageBonusMappings;
}

/**
 * Returns a TMap of all damage bonuses across class, race and common data.
 * \return The TMap, either empty or populated with valid damage bonuses.
 */
TMap<FGameplayAttribute, float> UPrimaryCharacterData::GetDefaultDamageBonus() const
{
	TMap<FGameplayAttribute, float> DamageBonusMappings = {};
	// Loops through CharacterData, Race & Class assets for all damage resistances
	TArray<UCharacterCommonData*> LoopAssets = {CharacterDataAsset, CharacterRaceData, CharacterClassData};
	for (const UCharacterCommonData* DataAsset : LoopAssets)
	{
		if (IsValid(DataAsset))
		{
			for (auto const& [DamageKey, DamageValue] : DataAsset->DamageBonusModifiers)
			{
				UE_LOGFMT(LogAssetData, Display,
					"Found Default Damage Bonus '{BaseAbility} = {BaseValue}' in Data Asset '{DataAsset}'",
						DamageKey.GetName(), DamageValue, DataAsset->GetName());
				if (!DamageBonusMappings.Contains(DamageKey))
				{
					DamageBonusMappings.Add(DamageKey, DamageValue);
				}
				DamageBonusMappings[DamageKey] += DamageValue;
			}
		}
	}
	return DamageBonusMappings;
}

/**
 * Returns a TMap of all damage resistances across class, race and common data.
 * \return The TMap, either empty or populated with valid damage resistances.
 */
TMap<FGameplayAttribute, float> UPrimaryCharacterData::GetDefaultDamageResist() const
{
	TMap<FGameplayAttribute, float> DamageResistMappings = {};
	// Loops through CharacterData, Race & Class assets for all damage resistances
	TArray<UCharacterCommonData*> LoopAssets = {CharacterDataAsset, CharacterRaceData, CharacterClassData};
	for (const UCharacterCommonData* DataAsset : LoopAssets)
	{
		if (IsValid(DataAsset))
		{
			for (const TPair<FGameplayAttribute, float> DamagePair : DataAsset->DamageResistModifiers)
			{
				for (auto const& [DamageKey, DamageValue] : DataAsset->DamageBonusModifiers)
			
				{
					UE_LOGFMT(LogAssetData, Display,
						"Found Default Damage Resist '{BaseAbility} = {BaseValue}' in Data Asset '{DataAsset}'",
							DamageKey.GetName(), DamageValue, DataAsset->GetName());
					if (!DamageResistMappings.Contains(DamageKey))
					{
						DamageResistMappings.Add(DamageKey, DamageValue);
					}
					DamageResistMappings[DamageKey] += DamageValue;
				}
			}
		}
	}
	return DamageResistMappings;
}

/**
 * Returns an array of all the ability classes that this character should start with.
 * \return The array of abilities
 */
TArray<TSubclassOf<UTalesGameplayAbility>> UPrimaryCharacterData::GetDefaultAbilities() const
{
	TSet< TSubclassOf<UTalesGameplayAbility> > ReturnArray = {};

	// Loops through CharacterData, Race & Class assets for all starting abilities
	TArray<UCharacterCommonData*> LoopAssets = {CharacterDataAsset, CharacterRaceData, CharacterClassData};
	for (const UCharacterCommonData* DataAsset : LoopAssets)
	{
		if (IsValid(DataAsset))
		{
			for (const TSubclassOf<UTalesGameplayAbility> BaseAbility : DataAsset->Abilities)
			{
				UE_LOGFMT(LogAssetData, Display,
					"Found Default Abilityy {BaseAbility} in Data Asset '{DataAsset}'",
						BaseAbility->GetName(), DataAsset->DisplayName);
				ReturnArray.Add(BaseAbility);
			}
		}
	}
	
	UE_LOGFMT(LogAssetData, Display,
		"GetDefaultAbilities(): Returning an array with {NumOptions} options.", ReturnArray.Num());
	return ReturnArray.Array();
}


/**
 * Returns an array of all the effect classes that this character should start with.
 * \return The array of effects
 */
TArray<TSubclassOf<UGameplayEffect>> UPrimaryCharacterData::GetDefaultEffects() const
{
	TSet< TSubclassOf<UGameplayEffect> > ReturnArray = {};

	// Loops through CharacterData, Race & Class assets for all starting abilities
	TArray<UCharacterCommonData*> LoopAssets = {CharacterDataAsset, CharacterRaceData, CharacterClassData};
	for (const UCharacterCommonData* DataAsset : LoopAssets)
	{
		if (IsValid(DataAsset))
		{
			for (const TSubclassOf<UGameplayEffect> BaseEffect : DataAsset->Effects)
			{
				UE_LOGFMT(LogAssetData, Display,
					"Found Default Effect {BaseEffect} in Data Asset '{DataAsset}'",
						BaseEffect->GetName(), DataAsset->DisplayName);
				ReturnArray.Add(BaseEffect);
			}
		}
	}
	
	UE_LOGFMT(LogAssetData, Display,
		"GetDefaultAbilities(): Returning an array with {NumOptions} options.", ReturnArray.Num());
	return ReturnArray.Array();
}
