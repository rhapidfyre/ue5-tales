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
			"UMG", "Slate", "SlateCore", "AIModule",
			"T5GInventorySystem", "GameplayAbilities", "GameplayTags",
			"Niagara", "EnhancedInput", "GameplayTasks", "RealmsForge"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
}
}
