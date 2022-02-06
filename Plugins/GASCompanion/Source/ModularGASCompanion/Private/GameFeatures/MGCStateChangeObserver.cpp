// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "GameFeatures/MGCStateChangeObserver.h"

#include "GameFeatureData.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeatures/Private/GameFeaturePluginStateMachine.h"
#include "Subsystems/MGCGameFeatureStateChangeSubsystem.h"
#include "ModularGASCompanionLog.h"

void UMGCStateChangeObserver::OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName)
{
	Super::OnGameFeatureRegistering(GameFeatureData, PluginName);
	FString PluginURL;
	if (!UGameFeaturesSubsystem::Get().GetPluginURLForBuiltInPluginByName(GameFeatureData->GetName(), PluginURL))
	{
		MGC_LOG(Error, TEXT("UMGCStateChangeObserver - Couldn't determine PluginURL from GameFeature name: %s"), *GameFeatureData->GetName())
		return;
	}

	UMGCGameFeatureStateChangeSubsystem::Get().SetPluginState(PluginURL, EMGCGameFeaturePluginState::Registered);
	MGC_LOG(Log, TEXT("UMGCStateChangeObserver - Game Feature registering %s, State set to Registered"), *GameFeatureData->GetName())
}

void UMGCStateChangeObserver::OnGameFeatureActivating(const UGameFeatureData* GameFeatureData)
{
	Super::OnGameFeatureActivating(GameFeatureData);
	FString PluginURL;
	if (!UGameFeaturesSubsystem::Get().GetPluginURLForBuiltInPluginByName(GameFeatureData->GetName(), PluginURL))
	{
		MGC_LOG(Error, TEXT("UMGCStateChangeObserver - Couldn't determine PluginURL from GameFeature name: %s"), *GameFeatureData->GetName())
		return;
	}

	UMGCGameFeatureStateChangeSubsystem::Get().SetPluginState(PluginURL, EMGCGameFeaturePluginState::Active);
	MGC_LOG(Log, TEXT("UMGCStateChangeObserver - Game Feature activating %s, State set to Active"), *GameFeatureData->GetName())
}

void UMGCStateChangeObserver::OnGameFeatureLoading(const UGameFeatureData* GameFeatureData)
{
	Super::OnGameFeatureLoading(GameFeatureData);
	FString PluginURL;
	if (!UGameFeaturesSubsystem::Get().GetPluginURLForBuiltInPluginByName(GameFeatureData->GetName(), PluginURL))
	{
		MGC_LOG(Error, TEXT("UMGCStateChangeObserver - Couldn't determine PluginURL from GameFeature name: %s"), *GameFeatureData->GetName())
		return;
	}

	UMGCGameFeatureStateChangeSubsystem::Get().SetPluginState(PluginURL, EMGCGameFeaturePluginState::Loaded);
	MGC_LOG(Log, TEXT("UMGCStateChangeObserver - Game Feature loading %s, State set to Loaded"), *GameFeatureData->GetName())
}

void UMGCStateChangeObserver::OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(GameFeatureData, Context);
	// TODO: This is a real issue, as this event is called regardless of final state and can be either loaded, registered or uninstalled
	// TODO: Document it, for now default it to custom OutOfSync state

	FString PluginURL;
	if (!UGameFeaturesSubsystem::Get().GetPluginURLForBuiltInPluginByName(GameFeatureData->GetName(), PluginURL))
	{
		MGC_LOG(Error, TEXT("UMGCStateChangeObserver - Couldn't determine PluginURL from GameFeature name: %s"), *GameFeatureData->GetName())
		return;
	}

	UMGCGameFeatureStateChangeSubsystem::Get().SetPluginState(PluginURL, EMGCGameFeaturePluginState::OutOfSync);
	MGC_LOG(Log, TEXT("UMGCStateChangeObserver - Game Feature deactivating %s, State set to OutOfSync"), *GameFeatureData->GetName())
}
