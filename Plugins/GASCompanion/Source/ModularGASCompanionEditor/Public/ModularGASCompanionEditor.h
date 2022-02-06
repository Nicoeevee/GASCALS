// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IPlugin;
class IDetailLayoutBuilder;
struct FMGCPluginEditingContext;
class FMGCPluginEditorExtension;

using FPluginEditorExtensionHandle = int32;


/** Notification when editing a plugin to allow further customization */
DECLARE_DELEGATE_RetVal_TwoParams(TSharedPtr<FMGCPluginEditorExtension>, FMGCOnPluginBeingEdited, FMGCPluginEditingContext& /*InPluginContext*/, IDetailLayoutBuilder& /*DetailBuilder*/);

class FModularGASCompanionEditorModule : public IModuleInterface
{
public:
	/** ID name for the plugin creator tab */
	static const FName PluginCreatorTabName;

	/** Accessor for the module interface */
	static FModularGASCompanionEditorModule& Get()
	{
		return FModuleManager::Get().GetModuleChecked<FModularGASCompanionEditorModule>(TEXT("ModularGASCompanionEditor"));
	}

	// TSharedRef<IDetailCustomization> PluginMetadataDetailCustomization;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void InitializeStyles();
	static void ShutdownStyle();

	const TArray<TPair<FMGCOnPluginBeingEdited, FPluginEditorExtensionHandle>>& GetCustomizePluginEditingDelegates() const { return CustomizePluginEditingDelegates; }

private:
	FPluginEditorExtensionHandle PluginEditorExtensionDelegate = 0;

	/** Additional customizers of plugin editing */
	TArray<TPair<FMGCOnPluginBeingEdited, FPluginEditorExtensionHandle>> CustomizePluginEditingDelegates;
	int32 EditorExtensionCounter = 0;

	/** Only called for 4.27 to setup editor extensions and customization */
	void SetupGameFeatureEditorCustomization();
	void ShutdownGameFeatureEditorCustomization();

	static void RegisterMenus();
	static void OpenGameFeaturePluginCreation();

	/** Make sure the game has the appropriate asset manager configuration or we won't be able to load game feature data assets */
	void OnAssetManagerCreated();

	/** Make sure the game has the appropriate game feature configuration to handle the InitialState of GameFeatures similar to how it's done in ue5 */
	static void HandleGameFeatureSubsystemSettings();

	/** Handles writing of AssetManager settings in .ini file */
	void AddDefaultGameDataRule();
	static void AddDefaultGameFeaturesManagerClass();

	/** Handles writing of AssetManager settings in .ini file manually via GConfig (needed for 4.27 to setup directory for /Game/Unused */
	static bool UpdateGameFeatureAssetTypeDirectories(FText& FailReason);

	void OnModularFeatureRegistered(const FName& Type, IModularFeature* ModularFeature);
	void OnModularFeatureUnregistered(const FName& Type, IModularFeature* ModularFeature);

	/* Part of IPluginBrowser in ue5, fallback implementation for 4.27 */
	virtual FPluginEditorExtensionHandle RegisterPluginEditorExtension(FMGCOnPluginBeingEdited Extension);

	/** Called to spawn the plugin creator tab */
	TSharedRef<SDockTab> HandleSpawnPluginCreatorTab(const FSpawnTabArgs& SpawnTabArgs) const;

};
