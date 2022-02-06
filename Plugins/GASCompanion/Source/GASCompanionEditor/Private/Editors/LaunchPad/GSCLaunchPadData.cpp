// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Editors/LaunchPad/GSCLaunchPadData.h"
#include "Core/Logging/GASCompanionEditorLog.h"
#include "Interfaces/IPluginManager.h"
#include "Core/Editor/GSCEditorTypes.h"
#include "GSC.h"

TArray<FGSCLaunchPadItemInfo> GSCLaunchPadData::RegisteredItems;

bool GSCLaunchPadData::IsExamplesPluginEnabled()
{
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("GASCompanionExamples")))
	{
		return Plugin->IsEnabled();
	}
	return false;
}

bool GSCLaunchPadData::IsExamplesPluginInstalled()
{
	static const FString PluginExamplesInstallLocation = FPaths::ProjectPluginsDir() / "GASCompanionExamples";
	return FPaths::DirectoryExists(PluginExamplesInstallLocation);
}

TArray<FGSCLaunchPadItemInfo> GSCLaunchPadData::CreateItems()
{
	TArray<FGSCLaunchPadItemInfo> Items;

	if (!IsExamplesPluginInstalled() || !IsExamplesPluginEnabled())
	{
		const FString PluginURL = GetExamplesPluginDownloadURL();

		EDITOR_LOG(Display, TEXT("GSCLaunchPadData::CreateItems Register install plugin with URL: %s"), *PluginURL)
		Items.Add(CreateItem(
			"Install GAS Companion Examples",
			"",
			"LaunchPad examples are managed via a standalone plugin you can enable / disable freely. Once installed, this page will be updated with all the examples maps included in the plugin.\n\nClick the \"Install Plugin\" button below to initiate the plugin installation. It will be downloaded from GitHub repository and installed as a Project Plugin, you'll then have to restart the editor to update this window.",
			"LaunchPad.Card.GASCompanionExamples",
			{
				FGSCLaunchPadItemAction(
					IsExamplesPluginInstalled() ? EGSCLaunchPadActionType::EnableExamplesPlugin : EGSCLaunchPadActionType::InstallExamplesPlugin,
					PluginURL,
					"",
					{},
					nullptr
				),
				FGSCLaunchPadItemAction(
					EGSCLaunchPadActionType::Documentation,
					FGSCModule::GetDocumentationURL("/launch-pad"),
					"",
					{},
					nullptr
				)
			}
		));

		return Items;
	}

	EDITOR_LOG(Verbose, TEXT("GSCLaunchPadData::CreateItems RegisteredItems %d"), RegisteredItems.Num())

	Items.Append(RegisteredItems);
	return Items;
}

FGSCLaunchPadItemInfo GSCLaunchPadData::CreateItem(const FString Title, const FString Subtitle, const FString Description, const FString Image, const TArray<FGSCLaunchPadItemAction> Actions)
{
	return FGSCLaunchPadItemInfo(
		Title,
		Subtitle,
		Description,
		Image,
		Actions
	);
}

void GSCLaunchPadData::RegisterItem(const FString Title, const FString Subtitle, const FString Description, const FString Image, const TArray<FGSCLaunchPadItemAction> Actions)
{
	const FGSCLaunchPadItemInfo Item = CreateItem(Title, Subtitle, Description, Image, Actions);
	RegisteredItems.Add(Item);
}

FString GSCLaunchPadData::GetPluginVersion()
{
	IPluginManager& PluginManager = IPluginManager::Get();
	const TSharedPtr<IPlugin> Plugin = PluginManager.FindPlugin("GASCompanion");
	if (!Plugin.IsValid())
	{
		return "";
	}

	const FPluginDescriptor PluginDescriptor = Plugin->GetDescriptor();
	const FString VersionName = PluginDescriptor.VersionName;
	const FString EngineVersion = FEngineVersion::Current().ToString(EVersionComponent::Minor);

	FString Left;
	FString Right;
	if (!VersionName.Split(TEXT("+"), &Left, &Right, ESearchCase::CaseSensitive, ESearchDir::FromEnd))
	{
		return VersionName;
	}

	return FString::Printf(TEXT("%s+%s"), *Left, *EngineVersion);
}

FString GSCLaunchPadData::GetExamplesPluginDownloadURL()
{
	const FString PluginVersion = "3.0.2+4.27";

	const FString PluginURL = FString::Printf(
		TEXT("https://github.com/GASCompanion/GASCompanionExamples/releases/download/%s/GASCompanionExamples_%s.zip"),
		*PluginVersion,
		*PluginVersion
	);

	return PluginURL;
}
