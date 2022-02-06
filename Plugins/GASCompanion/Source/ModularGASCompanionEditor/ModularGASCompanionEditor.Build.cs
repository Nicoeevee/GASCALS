// Copyright Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class ModularGASCompanionEditor : ModuleRules
	{
        public ModularGASCompanionEditor(ReadOnlyTargetRules Target) : base(Target)
		{
			PublicDependencyModuleNames.AddRange(
				new string[]
                {
                    "Core",
                    "CoreUObject",
					"ModularGameplay",
					"Engine",
                    "InputCore",
					"DeveloperSettings",
					"PluginBrowser",
					"GameFeatures",
					"ModularGASCompanion"
                }
			);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"EditorSubsystem",
					"UnrealEd",
					"Projects",
					"ToolMenus",
					"Slate",
					"SlateCore",
					"EditorStyle",
					"PropertyEditor",
					"SharedSettingsWidgets",
					"Json",
					"GameProjectGeneration",
					"MainFrame",
					"PluginUtils",
					"SettingsEditor"
				}
			);

			PrivateIncludePathModuleNames.AddRange(
				new string[] {
					"DesktopPlatform",
					"GameProjectGeneration",
				}
			);
		}
	}
}
