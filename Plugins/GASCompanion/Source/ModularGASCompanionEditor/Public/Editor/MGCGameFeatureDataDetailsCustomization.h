// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "GameFeaturesSubsystem.h"
#include "ModularGASCompanionEditorTypes.h"
#include "ModularGASCompanionTypes.h"


class IDetailLayoutBuilder;
class SErrorText;
class IPlugin;

/** Provides fallback support for 4.27 customization for GameFeatureData */
class FMGCGameFeatureDataDetailsCustomization : public IDetailCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
	// End of IDetailCustomization interface

protected:
	void ChangeDesiredState(EMGCGameFeaturePluginState State);

	FText GetWarningText() const;
	EMGCGameFeaturePluginState GetCurrentState() const;

	/** Determine the initial feature state for a built-in plugin */
	static EMGCBuiltInAutoState DetermineBuiltInInitialFeatureState(TSharedPtr<FJsonObject> Descriptor, const FString& ErrorContext);
	static EMGCGameFeaturePluginState ConvertInitialFeatureStateToTargetState(EMGCBuiltInAutoState InInitialState);

	static void OnOperationCompletedOrFailed(const UE::GameFeatures::FResult& Result, const TWeakPtr<FMGCGameFeatureDataDetailsCustomization> WeakThisPtr, EMGCGameFeaturePluginState CompletedState);

	static void OnEditCommitted();

	static FReply OnEditPluginClicked();

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	FString PluginURL;
	TSharedPtr<IPlugin> PluginPtr;

	TSharedPtr<SErrorText> ErrorTextWidget;
};
