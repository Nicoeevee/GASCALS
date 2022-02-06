// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "ModularGASCompanionEditor.h"

#include "GameFeatureData.h"
#include "GameFeaturesSubsystemSettings.h"
#include "ISettingsEditorModule.h"
#include "Editor/MGCGameFeaturePluginMetadataCustomization.h"
#include "Slate/MGCEditorStyle.h"
#include "Editor/MGCGameFeatureDataDetailsCustomization.h"
#include "Editor/MGCPluginMetadaObject.h"
#include "ModularGASCompanionLog.h"
#include "SSettingsEditorCheckoutNotice.h"
#include "ToolMenus.h"
#include "Logging/MessageLog.h"
#include "Engine/AssetManager.h"
#include "Engine/AssetManagerSettings.h"
#include "Features/EditorFeatures.h"
#include "Features/IPluginsEditorFeature.h"
#include "Framework/Notifications/NotificationManager.h"
#include "GameFeatures/MGCGameFeaturesProjectPolicies.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Slate/SMGCNewPluginWizard.h"


#define LOCTEXT_NAMESPACE "ModularGASCompanionEditor"

const FName FModularGASCompanionEditorModule::PluginCreatorTabName(TEXT("MGCPluginCreator"));

void FModularGASCompanionEditorModule::StartupModule()
{
#if ENGINE_MAJOR_VERSION < 5
	SetupGameFeatureEditorCustomization();
#endif
}

void FModularGASCompanionEditorModule::ShutdownModule()
{
#if ENGINE_MAJOR_VERSION < 5
	ShutdownGameFeatureEditorCustomization();
#endif
}

void FModularGASCompanionEditorModule::InitializeStyles()
{
	FMGCEditorStyle::Initialize();
	FMGCEditorStyle::ReloadTextures();
}

void FModularGASCompanionEditorModule::ShutdownStyle()
{
	FMGCEditorStyle::Shutdown();
}

void FModularGASCompanionEditorModule::SetupGameFeatureEditorCustomization()
{
	// Register the details customizations
	InitializeStyles();

	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(UGameFeatureData::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FMGCGameFeatureDataDetailsCustomization::MakeInstance));
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	// Register to get a warning on startup if settings aren't configured correctly
	UAssetManager::CallOrRegister_OnAssetManagerCreated(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FModularGASCompanionEditorModule::OnAssetManagerCreated));
	HandleGameFeatureSubsystemSettings();

	{
		IModularFeatures& ModularFeatures = IModularFeatures::Get();
		ModularFeatures.OnModularFeatureRegistered().AddRaw(this, &FModularGASCompanionEditorModule::OnModularFeatureRegistered);
		ModularFeatures.OnModularFeatureUnregistered().AddRaw(this, &FModularGASCompanionEditorModule::OnModularFeatureUnregistered);

		if (ModularFeatures.IsModularFeatureAvailable(EditorFeatures::PluginsEditor))
		{
			OnModularFeatureRegistered(EditorFeatures::PluginsEditor, &ModularFeatures.GetModularFeature<IPluginsEditorFeature>(EditorFeatures::PluginsEditor));
		}

		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.RegisterCustomClassLayout(UMGCPluginMetadaObject::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FMGCGameFeaturePluginMetadataCustomization::MakeInstance));
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
			PluginCreatorTabName,
			FOnSpawnTab::CreateRaw(this, &FModularGASCompanionEditorModule::HandleSpawnPluginCreatorTab)
		)
		.SetDisplayName(LOCTEXT("NewPluginTabHeader", "New Plugin"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

		// Register a default size for this tab
		const FVector2D DefaultSize(1000.0f, 750.0f);
		FTabManager::RegisterDefaultTabWindowSize(PluginCreatorTabName, DefaultSize);
	}

	// Menu extender
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateStatic(&FModularGASCompanionEditorModule::RegisterMenus));
}

void FModularGASCompanionEditorModule::ShutdownGameFeatureEditorCustomization()
{
	ShutdownStyle();

	// Remove the customization
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomClassLayout(UGameFeatureData::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UMGCPluginMetadaObject::StaticClass()->GetFName());
		PropertyModule.NotifyCustomizationModuleChanged();
	}

	// Remove the plugin wizard override
	{
		IModularFeatures& ModularFeatures = IModularFeatures::Get();
		ModularFeatures.OnModularFeatureRegistered().RemoveAll(this);
		ModularFeatures.OnModularFeatureUnregistered().RemoveAll(this);

		if (ModularFeatures.IsModularFeatureAvailable(EditorFeatures::PluginsEditor))
		{
			OnModularFeatureUnregistered(EditorFeatures::PluginsEditor, &ModularFeatures.GetModularFeature<IPluginsEditorFeature>(EditorFeatures::PluginsEditor));
		}
		// ContentOnlyTemplate.Reset();
	}

	// Unregister the tab spawner
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PluginCreatorTabName);
	}
}

void FModularGASCompanionEditorModule::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.GASCompanionEditor.ComboMenu");
	FToolMenuSection& Section = Menu->FindOrAddSection("ProjectSection");
	Section.AddMenuEntry(
		"ModularGASCompanionNewGameFeaturePlugin",
		LOCTEXT("ModularGASCompanionNewGameFeaturePlugin", "New Game Feature Plugin..."),
		LOCTEXT("ModularGASCompanionNewGameFeaturePluginTooltip", "Adds Content-only Game Feature Plugin to the project."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "MainFrame.AddCodeToProject"),
		FUIAction(FExecuteAction::CreateStatic(&FModularGASCompanionEditorModule::OpenGameFeaturePluginCreation))
	);
}

void FModularGASCompanionEditorModule::OpenGameFeaturePluginCreation()
{
	MGC_LOG(Log, TEXT("FModularGASCompanionEditorModule::OpenGameFeaturePluginCreation"))
	FGlobalTabmanager::Get()->TryInvokeTab(PluginCreatorTabName);
}

void FModularGASCompanionEditorModule::OnAssetManagerCreated()
{
	// Make sure the game has the appropriate asset manager configuration or we won't be able to load game feature data assets
	const FPrimaryAssetId DummyGameFeatureDataAssetId(UGameFeatureData::StaticClass()->GetFName(), NAME_None);
	const FPrimaryAssetRules GameDataRules = UAssetManager::Get().GetPrimaryAssetRules(DummyGameFeatureDataAssetId);
	if (GameDataRules.IsDefault())
	{
		FMessageLog("LoadErrors").Error()
			->AddToken(
				FTextToken::Create(FText::Format(
					NSLOCTEXT(
						"ModularGASCompanionEditor",
						"MissingRuleForGameFeatureData",
						"GAS Companion: Asset Manager settings do not include an entry for assets of type {0}, which is required for game feature plugins to function."
					),
					FText::FromName(UGameFeatureData::StaticClass()->GetFName())
				))
			)
			->AddToken(FActionToken::Create(
				NSLOCTEXT("ModularGASCompanionEditor", "AddRuleForGameFeatureData", "Add entry to PrimaryAssetTypesToScan?"),
				FText(),
				FOnActionTokenExecuted::CreateRaw(this, &FModularGASCompanionEditorModule::AddDefaultGameDataRule), true)
			);
	}
}

void FModularGASCompanionEditorModule::HandleGameFeatureSubsystemSettings()
{
	if (!GConfig)
	{
		return;
	}

	const UAssetManagerSettings* Settings = GetDefault<UAssetManagerSettings>();
	const FString& ConfigFileName = Settings->GetDefaultConfigFilename();

	// Make sure the game has the appropriate game feature configuration to handle the InitialState of GameFeatures similar to how it's done in ue5, handled by UMGCGameFeaturesProjectPolicies in 4.27
	bool bShouldProceed = false;

	//Retrieve Default Game Type
	FString ManagerClassName;
	GConfig->GetString(
		TEXT("/Script/GameFeatures.GameFeaturesSubsystemSettings"),
		TEXT("GameFeaturesManagerClassName"),
		ManagerClassName,
		ConfigFileName
	);

	bShouldProceed = ManagerClassName.IsEmpty();
	if (bShouldProceed)
	{
		const FText DefaultManagerClassName = FText::FromName(UMGCGameFeaturesProjectPolicies::StaticClass()->GetFName());
		FMessageLog("LoadErrors").Error()
			->AddToken(
				FTextToken::Create(FText::Format(
					NSLOCTEXT(
						"ModularGASCompanionEditor",
						"MissingRuleForGameFeaturesManagerClass",
						"GAS Companion: Game Features settings do not include an entry for Game Feature Manager Class of type {0},\nwhich is required for game feature plugins to function in 4.27 similar to ue5 (InitialState won't work as expected)."
					),
					DefaultManagerClassName
				))
			)
			->AddToken(FActionToken::Create(
				FText::Format(NSLOCTEXT("ModularGASCompanionEditor", "AddRuleForGameFeatureData", "Setup ManagerClass to {0}?"), DefaultManagerClassName),
				FText(),
				FOnActionTokenExecuted::CreateStatic(&FModularGASCompanionEditorModule::AddDefaultGameFeaturesManagerClass), true)
			);
	}
}


void FModularGASCompanionEditorModule::AddDefaultGameDataRule()
{
	// Check out the ini or make it writable
	UAssetManagerSettings* Settings = GetMutableDefault<UAssetManagerSettings>();
	const FString& ConfigFileName = Settings->GetDefaultConfigFilename();

	bool bSuccess = false;

	FText NotificationOpText;
	if (!SettingsHelpers::IsCheckedOut(ConfigFileName, true))
	{
		FText ErrorMessage;
		bSuccess = SettingsHelpers::CheckOutOrAddFile(ConfigFileName, true, !IsRunningCommandlet(), &ErrorMessage);
		if (bSuccess)
		{
			NotificationOpText = LOCTEXT("CheckedOutAssetManagerIni", "Checked out {0}");
		}
		else
		{
			MGC_LOG(Error, TEXT("%s"), *ErrorMessage.ToString());
			bSuccess = SettingsHelpers::MakeWritable(ConfigFileName);

			if (bSuccess)
			{
				NotificationOpText = LOCTEXT("MadeWritableAssetManagerIni", "Made {0} writable (you may need to manually add to source control)");
			}
			else
			{
				NotificationOpText = LOCTEXT("FailedToTouchAssetManagerIni", "Failed to check out {0} or make it writable, so no rule was added");
			}
		}
	}
	else
	{
		NotificationOpText = LOCTEXT("UpdatedAssetManagerIni", "Updated {0}");
		bSuccess = true;
	}

	// Add the rule to project settings
	if (bSuccess)
	{
		FDirectoryPath DummyPath;
		DummyPath.Path = TEXT("/Game/Unused");

		// TODO: Directories / SpecificAssets not setup (private) and no default constructor for them in 4.27
		FPrimaryAssetTypeInfo NewTypeInfo(
			UGameFeatureData::StaticClass()->GetFName(),
			UGameFeatureData::StaticClass(),
			false,
			false
		);

		NewTypeInfo.Rules.CookRule = EPrimaryAssetCookRule::AlwaysCook;

		Settings->Modify(true);

		Settings->PrimaryAssetTypesToScan.Add(NewTypeInfo);

		Settings->PostEditChange();
		Settings->UpdateDefaultConfigFile();

		FText FailReason;
		if (!UpdateGameFeatureAssetTypeDirectories(FailReason))
		{
			MGC_LOG(Error, TEXT("4.27 - Failed to update ini file to add \"/Game/Unused\" directory to GameFeatureData AssetType. This is needed for game features to work.\nReason: %s"), *FailReason.ToString())
		}

		UAssetManager::Get().ReinitializeFromConfig();

		if (GConfig)
		{
			GConfig->Flush(true, ConfigFileName);
		}
	}

	// Show a message that the file was checked out/updated and must be submitted
	FNotificationInfo Info(FText::Format(NotificationOpText, FText::FromString(FPaths::GetCleanFilename(ConfigFileName))));
	Info.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	// Show a message that the file was checked out/updated and must be submitted
	const FText RestartText = LOCTEXT("RestartNeeded", "You need to restart the editor for this change to take effect.");
	FNotificationInfo RestartInfo(FText::Format(RestartText, FText::FromString(FPaths::GetCleanFilename(ConfigFileName))));
	RestartInfo.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(RestartInfo);

	ISettingsEditorModule* SettingsEditorModule = FModuleManager::GetModulePtr<ISettingsEditorModule>("SettingsEditor");
	if (SettingsEditorModule)
	{
		SettingsEditorModule->OnApplicationRestartRequired();
	}
}

void FModularGASCompanionEditorModule::AddDefaultGameFeaturesManagerClass()
{
	MGC_LOG(Log, TEXT("FModularGASCompanionEditorModule::AddDefaultGameFeaturesManagerClass"))
	if (!GConfig)
	{
		return;
	}

	const UAssetManagerSettings* Settings = GetDefault<UAssetManagerSettings>();
	const FString& ConfigFileName = Settings->GetDefaultConfigFilename();

	GConfig->SetString(
		TEXT("/Script/GameFeatures.GameFeaturesSubsystemSettings"),
		TEXT("GameFeaturesManagerClassName"),
		TEXT("/Script/ModularGASCompanion.MGCGameFeaturesProjectPolicies"),
		ConfigFileName
	);

	GConfig->Flush(true, ConfigFileName);

	// Show a message that the file was modified and editor needs a restart
	const FText NotificationOpText = LOCTEXT("UpdatedGameFeaturesIni", "Updated {0}");
	FNotificationInfo Info(FText::Format(NotificationOpText, FText::FromString(FPaths::GetCleanFilename(ConfigFileName))));
	Info.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	// Show a message that the file was checked out/updated and must be submitted
	const FText RestartText = LOCTEXT("RestartNeeded", "You need to restart the editor for this change to take effect.");
	FNotificationInfo RestartInfo(FText::Format(RestartText, FText::FromString(FPaths::GetCleanFilename(ConfigFileName))));
	RestartInfo.ExpireDuration = 5.0f;
	FSlateNotificationManager::Get().AddNotification(RestartInfo);

	ISettingsEditorModule* SettingsEditorModule = FModuleManager::GetModulePtr<ISettingsEditorModule>("SettingsEditor");
	if (SettingsEditorModule)
	{
		SettingsEditorModule->OnApplicationRestartRequired();
	}
}

bool FModularGASCompanionEditorModule::UpdateGameFeatureAssetTypeDirectories(FText& FailReason)
{
	const UAssetManagerSettings* Settings = GetDefault<UAssetManagerSettings>();
	const FString& ConfigFileName = Settings->GetDefaultConfigFilename();

	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *ConfigFileName))
	{
		FailReason = FText::Format(LOCTEXT("FailedToReadPluginTemplateFile", "Failed to read ini file\n{0}"), FText::FromString(FPaths::ConvertRelativePathToFull(ConfigFileName)));
		return false;
	}

	TArray<FString> Lines;
	FileContents.ParseIntoArray(Lines, LINE_TERMINATOR, false);

	TArray<FString> OutLines;
	for (FString Line : Lines)
	{
		if (Line.StartsWith("+PrimaryAssetTypesToScan=(PrimaryAssetType=\"GameFeatureData\""))
		{
			Line = Line.Replace(TEXT("Directories=,"), TEXT("Directories=((Path=\"/Game/Unused\")),"), ESearchCase::CaseSensitive);
		}

		OutLines.Add(Line);
	}

	FString OutFileContents = FString::Join(OutLines, LINE_TERMINATOR);
	if (!FFileHelper::SaveStringToFile(OutFileContents, *ConfigFileName))
	{
		FailReason = FText::Format(LOCTEXT("FailedToWritePluginFile", "Failed to write ini file\n{0}"), FText::FromString(FPaths::ConvertRelativePathToFull(ConfigFileName)));
		return false;
	}

	return true;
}

void FModularGASCompanionEditorModule::OnModularFeatureRegistered(const FName& Type, class IModularFeature* ModularFeature)
{
	MGC_LOG(Log, TEXT("FModularGASCompanionEditorModule::OnModularFeatureRegistered %s"), *Type.ToString())
}

void FModularGASCompanionEditorModule::OnModularFeatureUnregistered(const FName& Type, class IModularFeature* ModularFeature)
{
	MGC_LOG(Log, TEXT("FModularGASCompanionEditorModule::OnModularFeatureUnregistered %s"), *Type.ToString())
}

FPluginEditorExtensionHandle FModularGASCompanionEditorModule::RegisterPluginEditorExtension(FMGCOnPluginBeingEdited Extension)
{
	++EditorExtensionCounter;
	FPluginEditorExtensionHandle Result = EditorExtensionCounter;
	CustomizePluginEditingDelegates.Add(MakeTuple(Extension, Result));
	return Result;
}

TSharedRef<SDockTab> FModularGASCompanionEditorModule::HandleSpawnPluginCreatorTab(const FSpawnTabArgs& SpawnTabArgs) const
{
	TSharedRef<SDockTab> ResultTab = SNew(SDockTab).TabRole(ETabRole::NomadTab);
	const TSharedRef<SWidget> TabContentWidget = SNew(SMGCNewPluginWizard, ResultTab, nullptr);
	ResultTab->SetContent(TabContentWidget);
	return ResultTab;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModularGASCompanionEditorModule, ModularGASCompanionEditor)
