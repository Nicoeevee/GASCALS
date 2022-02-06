// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGASCompanionTypes.h"
#include "Subsystems/EngineSubsystem.h"
#include "MGCGameFeatureStateChangeSubsystem.generated.h"

/**
 * The manager subsystem to keep track of game feature changes done in Game Feature DataAsset via the custom DetailsCustomization used in 4.27
 *
 * It is required to handle accurate tracking of state changes for Game Feature plugins.
 *
 * The reason it's needed is because we can't rely on GameFeatureStateMachine like it's done in ue5 customization,
 * (as State Machines are not exposed to external code / modules) and the customization itself might be re-instanciated when actions
 * or other props are changes, so keeping track of the State there is not possible.
 *
 * Exposed from runtime module because observers from project policy in 4.27 needs to update states as well.
 *
 * This subsystem provides a singleton way of keeping a Map of states per GameFeature PluginURL.
 */
UCLASS()
class MODULARGASCOMPANION_API UMGCGameFeatureStateChangeSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	static UMGCGameFeatureStateChangeSubsystem& Get() { return *GEngine->GetEngineSubsystem<UMGCGameFeatureStateChangeSubsystem>(); }

	/** Returns PluginState associated with this GameFeature */
	EMGCGameFeaturePluginState GetPluginState(const FString PluginURL);

	/** Updates PluginState for a give GameFeature Name */
	void SetPluginState(const FString PluginURL, EMGCGameFeaturePluginState PluginState);

private:
	TMap<FString, EMGCGameFeaturePluginState> GameFeatureMap;
};
