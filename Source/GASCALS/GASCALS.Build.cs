// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GASCALS : ModuleRules
{
	public GASCALS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"ALSV4_CPP",
			"GASCompanion",
			"ModularGASCompanion",
			"GameplayAbilities", 
			"GameplayTasks", 
			"GameplayTags"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
