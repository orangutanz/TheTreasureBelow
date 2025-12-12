// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ApeDungeonGenerator : ModuleRules
{
	public ApeDungeonGenerator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });
    }
}
