// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TalesDungeoneer : ModuleRules
{
	public TalesDungeoneer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GameplayTags",
			"UMG",
			"Slate",
			"SlateCore",
			"AIModule",
			"GameplayAbilities",
			"Niagara",
			"EnhancedInput",
			"GameplayTasks",
			"T5GInventorySystem",
			"RealmsForge",
			"RoleStats"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
