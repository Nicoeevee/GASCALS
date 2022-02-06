// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "GameFeatures/MGCGameFeaturesProjectPolicies.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatures/MGCStateChangeObserver.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "ModularGASCompanionLog.h"

void UMGCGameFeaturesProjectPolicies::InitGameFeatureManager()
{
	Super::InitGameFeatureManager();

	MGC_LOG(Log, TEXT("UMGCGameFeaturesProjectPolicies::InitGameFeatureManager() Scanning for built-in game feature plugins"));

	// Handle loading of built-in game feature plugins, ensuring to override AutoState and setup InitialState based on BuiltInInitialFeatureState value in .uplugins (similar to ue5)
	auto AdditionalFilter = [&](const FString& PluginFilename, const FGameFeaturePluginDetails& PluginDetails, FBuiltInGameFeaturePluginBehaviorOptions& OutOptions) -> bool
	{
		MGC_LOG(Log, TEXT("UMGCGameFeaturesProjectPolicies: Building AdditionalFilter for %s"), *PluginFilename)

		FString FailReason;
		TSharedPtr<FJsonObject> JsonObject;
		if (!GetPluginJsonObject(PluginFilename, JsonObject, FailReason))
		{
			MGC_LOG(Error, TEXT("%s"), *FailReason)
			return false;
		}

		OutOptions.AutoStateOverride = DetermineBuiltInInitialFeatureState(JsonObject, PluginFilename);
		return true;
	};

	UGameFeaturesSubsystem::Get().LoadBuiltInGameFeaturePlugins(AdditionalFilter);

	// setup observers
	StateChangeObserver = NewObject<UMGCStateChangeObserver>();
	UGameFeaturesSubsystem::Get().AddObserver(StateChangeObserver);
}

void UMGCGameFeaturesProjectPolicies::ShutdownGameFeatureManager()
{
	MGC_LOG(Log, TEXT("UMGCGameFeaturesProjectPolicies::ShutdownGameFeatureManager() Scanning for built-in game feature plugins"));

	UGameFeaturesSubsystem::Get().RemoveObserver(StateChangeObserver);

	Super::ShutdownGameFeatureManager();
}

void UMGCGameFeaturesProjectPolicies::GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const
{
	// By default, load both unless we are a dedicated server or client only cooked build
	bLoadClientData = !IsRunningDedicatedServer();
	bLoadServerData = !IsRunningClientOnly();
}

EBuiltInAutoState UMGCGameFeaturesProjectPolicies::DetermineBuiltInInitialFeatureState(TSharedPtr<FJsonObject> JsonObject, const FString& ErrorContext)
{
	EBuiltInAutoState InitialState = EBuiltInAutoState::Invalid;
	FString InitialFeatureStateStr;
	if (JsonObject->TryGetStringField(TEXT("BuiltInInitialFeatureState"), InitialFeatureStateStr))
	{
		if (InitialFeatureStateStr == TEXT("Installed"))
		{
			InitialState = EBuiltInAutoState::Installed;
		}
		else if (InitialFeatureStateStr == TEXT("Registered"))
		{
			InitialState = EBuiltInAutoState::Registered;
		}
		else if (InitialFeatureStateStr == TEXT("Loaded"))
		{
			InitialState = EBuiltInAutoState::Loaded;
		}
		else if (InitialFeatureStateStr == TEXT("Active"))
		{
			InitialState = EBuiltInAutoState::Active;
		}
		else
		{
			if (!ErrorContext.IsEmpty())
			{
				MGC_LOG(Error, TEXT("Game feature '%s' has an unknown value '%s' for BuiltInInitialFeatureState (expected Installed, Registered, Loaded, or Active); defaulting to Active."), *ErrorContext, *InitialFeatureStateStr);
			}
			InitialState = EBuiltInAutoState::Active;
		}
	}
	else
	{
		// BuiltInAutoRegister. Default to true. If this is a built in plugin, should it be registered automatically (set to false if you intent to load late with LoadAndActivateGameFeaturePlugin)
		bool bBuiltInAutoRegister = true;
		JsonObject->TryGetBoolField(TEXT("BuiltInAutoRegister"), bBuiltInAutoRegister);

		// BuiltInAutoLoad. Default to true. If this is a built in plugin, should it be loaded automatically (set to false if you intent to load late with LoadAndActivateGameFeaturePlugin)
		bool bBuiltInAutoLoad = true;
		JsonObject->TryGetBoolField(TEXT("BuiltInAutoLoad"), bBuiltInAutoLoad);

		// The cooker will need to activate the plugin so that assets can be scanned properly
		bool bBuiltInAutoActivate = true;
		JsonObject->TryGetBoolField(TEXT("BuiltInAutoActivate"), bBuiltInAutoActivate);

		InitialState = EBuiltInAutoState::Installed;
		if (bBuiltInAutoRegister)
		{
			InitialState = EBuiltInAutoState::Registered;
			if (bBuiltInAutoLoad)
			{
				InitialState = EBuiltInAutoState::Loaded;
				if (bBuiltInAutoActivate)
				{
					InitialState = EBuiltInAutoState::Active;
				}
			}
		}

		if (!ErrorContext.IsEmpty())
		{
			//@TODO: Increase severity to a warning after changing existing features
			MGC_LOG(Log, TEXT("Game feature '%s' has no BuiltInInitialFeatureState key, using legacy BuiltInAutoRegister(%d)/BuiltInAutoLoad(%d)/BuiltInAutoActivate(%d) values to arrive at initial state."),
				*ErrorContext,
				bBuiltInAutoRegister ? 1 : 0,
				bBuiltInAutoLoad ? 1 : 0,
				bBuiltInAutoActivate ? 1 : 0);
		}
	}

	return InitialState;
}

bool UMGCGameFeaturesProjectPolicies::GetPluginJsonObject(FString PluginFilename, TSharedPtr<FJsonObject>& JsonObject, FString& FailReason)
{
	// Read the file to a string
	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *PluginFilename))
	{
		FailReason = FString::Printf(TEXT("UMGCGameFeaturesProjectPolicies could not determine if feature was hotfixable. Failed to read file. File:%s Error:%d"), *PluginFilename, FPlatformMisc::GetLastError());
		return false;
	}

	// Deserialize a JSON object from the string
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		FailReason = FString::Printf(TEXT("UMGCGameFeaturesProjectPolicies could not determine if feature was hotfixable. Json invalid. File:%s. Error:%s"), *PluginFilename, *Reader->GetErrorMessage());
		return false;
	}

	return true;
}

