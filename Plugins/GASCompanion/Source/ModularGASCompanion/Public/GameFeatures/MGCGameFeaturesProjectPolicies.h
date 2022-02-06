// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeaturesProjectPolicies.h"
#include "GameFeaturesSubsystem.h"
#include "MGCGameFeaturesProjectPolicies.generated.h"

class FJsonObject;
class UMGCStateChangeObserver;
class UMGCGameFeatureStateChangeObserver;

/**
 * This class implements project-specific rules for game feature plugins, and is only needed for 4.27.
 *
 * The default implementation UDefaultGameFeaturesProjectPolicies is used if no project-specific policy
 * is set in Project Settings .. Game Features
 *
 * In the context of GAS Companion and 4.27, this class is needed mostly to handle the InitialState of
 * GameFeatures similar to how it's done in ue5, eg. considering BuiltInInitialFeatureState value in .uplugin
 * files to setup AutoStateOverrides to ensure initial state of Game Feature matches BuiltInInitialFeatureState
 * values on game / editor startup.
 *
 * Also registers observers for game features plugin change state to update the internal state of editor customization for
 * game features Data Asset and handle state changes when done by other means than the customization itself
 * (switch state buttons in Data Asset)
 *
 * This also mimics default implementation that immediately processes all game feature plugins based on
 * their BuiltInAutoRegister, BuiltInAutoLoad, and BuiltInAutoActivate settings.
 *
 * It will be used if no project-specific policy is set in Project Settings .. Game Features
 */
UCLASS()
class MODULARGASCOMPANION_API UMGCGameFeaturesProjectPolicies : public UGameFeaturesProjectPolicies
{
	GENERATED_BODY()

public:
	//~UGameFeaturesProjectPolicies interface
	virtual void InitGameFeatureManager() override;
	virtual void ShutdownGameFeatureManager() override;
	virtual void GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const override;
	//~End of UGameFeaturesProjectPolicies interface

	/** Determine the initial feature state for a built-in plugin */
	static EBuiltInAutoState DetermineBuiltInInitialFeatureState(TSharedPtr<FJsonObject> JsonObject, const FString& ErrorContext);

	/** Reads JSON from passed in Plugin filename and returns according JSONObject */
	static bool GetPluginJsonObject(FString PluginFilename, TSharedPtr<FJsonObject>& JsonObject, FString& FailReason);
private:

	UPROPERTY(Transient)
	UMGCStateChangeObserver* StateChangeObserver;
};
