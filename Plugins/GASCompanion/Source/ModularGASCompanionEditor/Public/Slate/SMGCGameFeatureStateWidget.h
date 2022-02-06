// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModularGASCompanionTypes.h"
#include "Widgets/SCompoundWidget.h"

// enum class EMGCGameFeaturePluginState : uint8;

/**
 * A delegate that is invoked when widgets want to notify a user that they have been clicked.
 * Intended for use by buttons and other button-like widgets.
 */
DECLARE_DELEGATE_OneParam(FMGCOnWidgetChangesGameFeatureState, EMGCGameFeaturePluginState)

class MODULARGASCOMPANIONEDITOR_API SMGCGameFeatureStateWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMGCGameFeatureStateWidget) {}
		SLATE_ATTRIBUTE(EMGCGameFeaturePluginState, CurrentState)
		SLATE_ARGUMENT(FString, PluginURL)
		SLATE_EVENT(FMGCOnWidgetChangesGameFeatureState, OnStateChanged)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	EVisibility GetNormalVisibility() const;
	EVisibility GetOutOfSyncVisibility() const;
	static FText GetDisplayNameOfState(EMGCGameFeaturePluginState State);

private:
	TAttribute<EMGCGameFeaturePluginState> CurrentState;
	FString PluginURL;

	FText GetStateStatusDisplay() const;
};
