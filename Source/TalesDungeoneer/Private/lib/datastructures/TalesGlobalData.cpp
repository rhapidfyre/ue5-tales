
#include "lib/datastructures/TalesGlobalData.h"

#include "Abilities/RsGameplayAbilityBase.h"
#include "Engine/AssetManager.h"
#include "Lib/AbilityData.h"

FStFactionData::FStFactionData()
{
}

FStFactionData::FStFactionData(const FStFactionData& InFactionData)
{
	if (this != &InFactionData)
	{
		FactionEnum = InFactionData.FactionEnum;
		FactionValue = InFactionData.FactionValue;
	}
}

FStFactionData::FStFactionData(const EFaction InFaction, const float InValue)
	: FactionEnum(InFaction)
{
	SetFactionValue(InValue);
}

FStFactionData::FStFactionData(const EFaction InFaction, const EFactionState InState)
	: FactionEnum(InFaction)
{
	SetFactionState(InState);
}

void FStFactionData::SetFactionValue(const float InValue)
{
	FactionValue = FMath::Clamp(InValue, MinFactionValue, MaxFactionValue);
}

void FStFactionData::SetFactionState(const EFactionState InState)
{
	if (InState == EFactionState::HATE) { FactionValue = -100.f; }
	if (InState == EFactionState::NONE) { FactionValue =    0.f; }
	if (InState == EFactionState::LIKE) { FactionValue =  100.f; }
	FactionValue = 500.f;
}

float FStFactionData::GetFactionValue() const
{
	return FactionValue;
}

EFactionState FStFactionData::GetFactionState() const
{
	if (FactionValue <= -100.f) return EFactionState::HATE;
	if (FactionValue < 100.f)  return EFactionState::NONE;
	if (FactionValue < 500.f) return EFactionState::LIKE;
	return EFactionState::ALLY;
}

FStFactionData& FStFactionData::operator=(const FStFactionData& InFactionData)
{
	if (this != &InFactionData)
	{
		FactionEnum = InFactionData.FactionEnum;
		FactionValue = InFactionData.FactionValue;
	}
	return *this;
}

FString FTalesVersion::ToString(const bool bShowBranch, const bool bSafeString) const
{
	TArray<FString> StringArray = {
		FString::FromInt(VersionMajor),
		FString::FromInt(VersionMinor),
		FString::FromInt(VersionPatch)
	};
	if (bShowBranch) { StringArray.Add(VersionBranch); }
	return bSafeString ?
			FString::Join(StringArray, TEXT("_"))
		:	FString::Join(StringArray, TEXT("."));
}

FString UTalesGlobalData::GetAppVersion(bool bShowBranch, bool bSafeString)
{
	return GetFullAppVersion().ToString(bShowBranch, bSafeString);
}

FTalesVersion UTalesGlobalData::GetFullAppVersion()
{
	FTalesVersion talesVersion = {};
	FString AppVersion;
	TArray<FString> SplitString;
	GConfig->GetString(
		TEXT("/Script/EngineSettings.GeneralProjectSettings"),
		TEXT("ProjectVersion"),
		AppVersion,
		GGameIni
	);
	AppVersion.ParseIntoArray(SplitString, TEXT("."));
	if (SplitString.IsValidIndex(2)) { talesVersion.VersionPatch = FCString::Atoi(*SplitString[2]); }
	if (SplitString.IsValidIndex(1)) { talesVersion.VersionMinor = FCString::Atoi(*SplitString[1]); }
	if (SplitString.IsValidIndex(0)) { talesVersion.VersionMajor = FCString::Atoi(*SplitString[0]); }
	talesVersion.VersionBranch = SplitString.IsValidIndex(3) ? SplitString[3] : "";
	return talesVersion;
}

int UTalesGlobalData::GetGameMaxCharacterLevel()
{
	return 100;
}

FString UTalesGlobalData::GetStringFromAttribute(const FGameplayAttribute& GameplayAttribute)
{
	if (GameplayAttribute.IsValid())
	{
		const FString AttributeName = GameplayAttribute.GetName();
		if (!AttributeName.IsEmpty())
		{
			int OutIndex = 0;
			AttributeName.FindLastChar('.', OutIndex);
			return AttributeName.RightChop(OutIndex);
		}
	}
	return "None";
}

/**
 * \brief
 * \param OptionalClass If valid, only returns abilities that are for this class
 * \param OptionalSchool If valid, only returns abilities that are in this ability school
 * \return An array of all abilities
 */
TArray<URsAbilityDataAsset*> UTalesGlobalData::GetAllGameplayAbilities(const FGameplayTag& OptionalClass, const FGameplayTag& OptionalSchool)
{
	TArray<URsAbilityDataAsset*> AbilityArray;
	UAssetManager& AssetManager = UAssetManager::Get();
	TArray<FPrimaryAssetId> AssetIds;

	// Get all asset IDs of the specified type
	const FName AssetTypeName = TEXT("RsAbilityDataAsset"); //URsAbilityDataAsset::StaticClass()->GetFName();
	const FPrimaryAssetType PrimaryAssetType(AssetTypeName);
	AssetManager.GetPrimaryAssetIdList(PrimaryAssetType, AssetIds);

	// Get all abilities that contain the required class
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		if (URsAbilityDataAsset* AbilityDataAsset = Cast<URsAbilityDataAsset>(AssetManager.GetPrimaryAssetObject(AssetId)))
		{
			const TSubclassOf<URsGameplayAbilityBase> AbilityClass = AbilityDataAsset->GetAbilityReference();
			if (OptionalClass.IsValid())
			{
				// If using a comparison tag and the ability requires a class tag, it must contain the comparison tag.
				const URsGameplayAbilityBase* AbilityObject = AbilityClass->GetDefaultObject<URsGameplayAbilityBase>();
				if (AbilityObject && AbilityObject->RequiredLevel.Num() > 0)
				{
					TArray<FGameplayTag> OutTags;
					AbilityObject->RequiredLevel.GetKeys(OutTags);
					if (OutTags.Contains(OptionalClass))
					{
						AbilityArray.Add(AbilityDataAsset);
					}
					continue;
				}
			}
			AbilityArray.Add(AbilityDataAsset);
		}
	}

	// Filter any abilities that don't match the optional school tag
	if (OptionalSchool.IsValid())
	{
		TArray<URsAbilityDataAsset*> FinalArray;
		for (auto* AbilityData : AbilityArray)
		{
			if (OptionalSchool.MatchesTag(AbilityData->GetAbilitySchool()))
			{
				FinalArray.Add(AbilityData);
			}
		}
		return FinalArray;
	}
	return AbilityArray;
}
