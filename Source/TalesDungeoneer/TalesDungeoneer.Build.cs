// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TalesDungeoneer : ModuleRules
{
	public TalesDungeoneer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore",
			"HeadMountedDisplay", "EnhancedInput", "Niagara",
			"UMG", "Slate", "SlateCore", "GameplayTags",
			"T5GInventorySystem", "AIModule"
		});
	}
}
