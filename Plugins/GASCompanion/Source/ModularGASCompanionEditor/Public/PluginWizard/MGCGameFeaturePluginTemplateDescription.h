// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "GameFeatureData.h"
#include "GameFeaturesSubsystemSettings.h"
#include "ModularGASCompanionEditorTypes.h"
#include "ModularGASCompanionLog.h"
#include "PluginDescriptor.h"
#include "Editor/MGCGameFeatureDataDetailsCustomization.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FGameFeaturePluginTemplateDescription"

struct FMGCGameFeaturePluginTemplateDescription : public FMGCPluginTemplateDescription
{
	FMGCGameFeaturePluginTemplateDescription(FText InName, FText InDescription, FString InOnDiskPath)
		: FMGCPluginTemplateDescription(InName, InDescription, InOnDiskPath, /*bCanContainContent=*/ true, EHostType::Runtime)
	{
		SortPriority = 10;
		bCanBePlacedInEngine = false;
	}

	virtual bool ValidatePathForPlugin(const FString& ProposedAbsolutePluginPath, FText& OutErrorMessage) override
	{
		if (!IsRootedInGameFeaturesRoot(ProposedAbsolutePluginPath))
		{
			OutErrorMessage = LOCTEXT("InvalidPathForGameFeaturePlugin", "Game features must be inside the Plugins/GameFeatures folder");
			return false;
		}

		OutErrorMessage = FText::GetEmpty();
		return true;
	}

	virtual void UpdatePathWhenTemplateSelected(FString& InOutPath) override
	{
		if (!IsRootedInGameFeaturesRoot(InOutPath))
		{
			InOutPath = GetGameFeatureRoot();
		}
	}

	virtual void UpdatePathWhenTemplateUnselected(FString& InOutPath) override
	{
		InOutPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*FPaths::ProjectPluginsDir());
		FPaths::MakePlatformFilename(InOutPath);
	}

	virtual void CustomizeDescriptorBeforeCreation(FPluginDescriptor& Descriptor) override
	{
		Descriptor.bExplicitlyLoaded = true;
		// Todo: 4.27 Handle writing of BuiltInInitialFeatureState
		// Descriptor.AdditionalFieldsToWrite.FindOrAdd(TEXT("BuiltInInitialFeatureState")) = MakeShared<FJsonValueString>(TEXT("Active"));
		Descriptor.Category = TEXT("Game Features");
	}

	virtual void OnPluginCreated(TSharedPtr<IPlugin> NewPlugin) override
	{
		MGC_LOG(Log, TEXT("OnPluginCreated %s"), *NewPlugin->GetName())

		// TODO: 4.27 - Expose DefaultGameFeatureDataClass to DeveloperSettings
		// TSubclassOf<UGameFeatureData> DefaultGameFeatureDataClass = GetDefault<UGameFeaturesSubsystemSettings>()->DefaultGameFeatureDataClass;
		TSubclassOf<UGameFeatureData> DefaultGameFeatureDataClass = UGameFeatureData::StaticClass();
		if (DefaultGameFeatureDataClass == nullptr)
		{
			DefaultGameFeatureDataClass = UGameFeatureData::StaticClass();
		}

		// Create the game feature data asset
		FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

		UObject* NewAsset = AssetToolsModule.Get().CreateAsset(NewPlugin->GetName(), NewPlugin->GetMountedAssetPath(), DefaultGameFeatureDataClass, /*Factory=*/ nullptr);

		// Activate the new game feature plugin
		auto AdditionalFilter = [&](const FString& PluginFilename, const FGameFeaturePluginDetails& PluginDetails, FBuiltInGameFeaturePluginBehaviorOptions& OutOptions) -> bool
		{
			const bool bShouldProcess = PluginFilename == NewPlugin->GetDescriptorFileName();
			MGC_LOG(Log, TEXT("OnPluginCreated: filter for load game feature %s, should process: %s"), *NewPlugin->GetName(), bShouldProcess ? TEXT("true") : TEXT("false"))
			return bShouldProcess;
		};

		// TODO: 4.27 - LoadBuiltInGameFeaturePlugin per plugin not exposed in 4.27
		UGameFeaturesSubsystem::Get().LoadBuiltInGameFeaturePlugins(AdditionalFilter);

		// Edit the new game feature data
		if (NewAsset != nullptr)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(NewAsset);
		}
	}

	FString GetGameFeatureRoot() const
	{
		FString Result = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*(FPaths::ProjectPluginsDir() / TEXT("GameFeatures/")));
		FPaths::MakePlatformFilename(Result);
		return Result;
	}

	bool IsRootedInGameFeaturesRoot(const FString& InStr)
	{
		const FString DesiredRoot = GetGameFeatureRoot();

		FString TestStr = InStr / TEXT("");
		FPaths::MakePlatformFilename(TestStr);

		return TestStr.StartsWith(DesiredRoot);
	}
};

#undef LOCTEXT_NAMESPACE
