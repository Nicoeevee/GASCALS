// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureStateChangeObserver.h"
#include "MGCStateChangeObserver.generated.h"

/**
 * GameFeatureStateChangeObserver for game features plugin change state to update the internal state of editor customization for
 * game features Data Asset and handle state changes when done by other means than the customization itself
 * (switch state buttons in Data Asset)
 */
UCLASS()
class MODULARGASCOMPANION_API UMGCStateChangeObserver : public UGameFeatureStateChangeObserver
{
	GENERATED_BODY()

public:
	virtual void OnGameFeatureRegistering(const UGameFeatureData* GameFeatureData, const FString& PluginName) override;
	virtual void OnGameFeatureActivating(const UGameFeatureData* GameFeatureData) override;
	virtual void OnGameFeatureLoading(const UGameFeatureData* GameFeatureData) override;
	virtual void OnGameFeatureDeactivating(const UGameFeatureData* GameFeatureData, FGameFeatureDeactivatingContext& Context) override;
};
