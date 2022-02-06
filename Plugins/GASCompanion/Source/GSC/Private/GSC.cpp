// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "GSC.h"

#include "AbilitySystemGlobals.h"
#include "GSCAssetManager.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "GSCLog.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

#define LOCTEXT_NAMESPACE "FGSCModule"

void FGSCModule::StartupModule()
{
	GSC_LOG(Display, TEXT("Startup GASCompanionModule Module"));

#if WITH_EDITOR
	// Register custom project settings
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		const FText SettingsDescription = FText::FromString(TEXT(
			"General Settings for GAS Companion Plugin.\n"
			"\n"
			"Deprecation Warnings:\n"
			"---------------------\n"
			"Old AttributeSets settings that were used in 2.0 version of GAS Companion for 4.26 have been deprecated "
			"and moved to AdvancedDisplay (the little arrow at the bottom of Attribute Sets category)\n"
			"\n"
			"These should no longer be used in favor of Modular Actors which allow configuration of Attribute Sets per actor.\n"
			"\n"
			"Configuration for clamping of minimal values still has some use, but will be revamped in future versions."
		));

		const TSharedPtr<ISettingsSection> SettingsSection = SettingsModule->RegisterSettings(
			"Project",
			"Game",
			"GAS Companion",
			NSLOCTEXT("FGSCModule", "DeveloperSettingsName", "GAS Companion"),
			SettingsDescription,
			GetMutableDefault<UGSCDeveloperSettings>()
		);

		// Register the save handler to your settings, you might want to use it to
		// validate those or just act to settings changes.
		if (SettingsSection.IsValid())
		{
			SettingsSection->OnModified().BindRaw(this, &FGSCModule::HandleSettingsSaved);
		}

	}
#endif

	UGSCDeveloperSettings* Settings = GetMutableDefault<UGSCDeveloperSettings>();
	// TODO: Load Config even needed anymore ?
	Settings->LoadConfig();

	if (!Settings->bPreventGlobalDataInitialization)
	{
		GSC_LOG(Display, TEXT("GAS Companion StartupModule - Initialize Ability System Component (InitGlobalData)"));
		UAbilitySystemGlobals::Get().InitGlobalData();
	}
}

void FGSCModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	GSC_LOG(Display, TEXT("Shutdown GSC module"));

#if WITH_EDITOR
	// unregister settings
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Game", "GAS Companion");
	}
#endif
}

FString FGSCModule::GetDocumentationURL(FString InPath)
{
	static FString DocumentationURL = "https://gascompanion.github.io";
	// static FString DocumentationURL = "https://gascompanion-dev.netlify.app";
	if (InPath.IsEmpty())
	{
		return DocumentationURL;
	}

	if (!InPath.StartsWith("/"))
	{
		InPath = "/" + InPath;
	}

	return DocumentationURL + InPath;
}

bool FGSCModule::HandleSettingsSaved()
{
#if WITH_EDITOR
	const UGSCDeveloperSettings* Settings = GetDefault<UGSCDeveloperSettings>();

	// You can put any validation code in here and resave the settings in case an invalid value has been entered

	if (Settings->bPreventGlobalDataInitialization)
	{
		const UEngine* EngineSettings = GetDefault<UEngine>();
		GSC_LOG(Verbose, TEXT("FGSCModule::HandleSettingsSaved - Current AssetManagerClassName: %s"), *EngineSettings->AssetManagerClassName.ToString())
		if (EngineSettings->AssetManagerClassName.ToString() == "/Script/Engine.AssetManager")
		{
			FNotificationInfo Info(LOCTEXT(
				"SettingsHasPreventGlobalDataInitialization",
				"If you disable Ability System Globals initialization in GAS Companion module, it is recommended to use a\n"
				"game-specific AssetManager to handle it and the current Asset Manager is set to the Engine's default class.\n"
				"\n"
				"Would you like to update Asset Manager Class Name to use GSCAssetManager?\n"
				"\n"
				"(Clicking the link below will update your DefaultEngine.ini settings and may require an editor restart.)\n\n"
			));

			Info.FadeInDuration = 0.2f;
			Info.ExpireDuration = 15.0f;
			Info.FadeOutDuration = 1.0f;
			Info.bUseThrobber = false;
			Info.bUseLargeFont = false;

			Info.Hyperlink = FSimpleDelegate::CreateRaw(this, &FGSCModule::UpdateAssetManagerClass);
			Info.HyperlinkText = LOCTEXT("SettingsNotifLinkText", "Update Asset Manager Class to use GSCAssetManager ?");

			SettingsNotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
			if (SettingsNotificationItem.IsValid())
			{
				SettingsNotificationItem->SetCompletionState(SNotificationItem::CS_None);
			}
		}
	}
#endif

	return true;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void FGSCModule::UpdateAssetManagerClass()
{
#if WITH_EDITOR
	GSC_LOG(Warning, TEXT("Update asset manager class ..."))

	// Update engine settings for Asset Manager Class name
	UEngine* EngineSettings = GetMutableDefault<UEngine>();
	EngineSettings->AssetManagerClassName = UGSCAssetManager::StaticClass();

	const FString DefaultConfigFilename = EngineSettings->GetDefaultConfigFilename();
	GSC_LOG(Verbose, TEXT("Update asset manager class - Save settings to %s"), *DefaultConfigFilename)
	EngineSettings->UpdateSinglePropertyInConfigFile(EngineSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UEngine, AssetManagerClassName)), DefaultConfigFilename);

	// Notify user
	FNotificationInfo Info(LOCTEXT(
		"SettingsAssetManagerClassUpdated",
		"Asset Manager Class Name has been updated to use GSCAssetManager.\n\n"
		"You may need to restart the editor for it to be active."
	));

	Info.FadeInDuration = 0.2f;
	Info.ExpireDuration = 3.0f;
	Info.FadeOutDuration = 1.0f;
	Info.bUseThrobber = false;
	Info.bUseLargeFont = false;

	const TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
	if (Notification.IsValid())
	{
		Notification->SetCompletionState(SNotificationItem::CS_None);
	}

	// Fadeout previous notif
	if (SettingsNotificationItem.IsValid())
	{
		SettingsNotificationItem->Fadeout();
	}
#endif

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGSCModule, GASCompanion)
