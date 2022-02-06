// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Subsystems/MGCGameFeatureStateChangeSubsystem.h"

#include "ModularGASCompanionLog.h"

EMGCGameFeaturePluginState UMGCGameFeatureStateChangeSubsystem::GetPluginState(const FString PluginURL)
{
	EMGCGameFeaturePluginState* PluginStatePtr = GameFeatureMap.Find(PluginURL);
	if (!PluginStatePtr)
	{
		return EMGCGameFeaturePluginState::UnknownStatus;
	}

	return *PluginStatePtr;
}

void UMGCGameFeatureStateChangeSubsystem::SetPluginState(const FString PluginURL, EMGCGameFeaturePluginState PluginState)
{
	MGC_LOG(Verbose, TEXT("UMGCGameFeatureStateChangeSubsystem::SetPluginState for %s"), *PluginURL)
	GameFeatureMap.Add(PluginURL, PluginState);
}
