
#include "lib/datastructures/GlobalData.h"

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

FString UGlobalData::GetAppVersion(bool bShowBranch, bool bSafeString)
{
	return GetFullAppVersion().ToString(bShowBranch, bSafeString);
}

FTalesVersion UGlobalData::GetFullAppVersion()
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

int UGlobalData::GetGameMaxCharacterLevel()
{
	return 100;
}
