// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "ModularGASCompanionEditorTypes.h"
#include "ModularGASCompanionTypes.h"
#include "Editor/MGCPluginMetadaObject.h"

struct FMGCPluginEditingContext;
class IDetailLayoutBuilder;
class IPlugin;
struct FPluginDescriptor;

//////////////////////////////////////////////////////////////////////////
// FGameFeaturePluginMetadataCustomization

class FMGCGameFeaturePluginMetadataCustomization : public FMGCPluginEditorExtension
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	void CustomizeDetailsForPlugin(FMGCPluginEditingContext& InPluginContext, IDetailLayoutBuilder& DetailBuilder);

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	virtual void CommitEdits(FPluginDescriptor& Descriptor) override;

protected:
	static EMGCBuiltInAutoState DetermineBuiltInInitialFeatureState(TSharedPtr<FJsonObject> Descriptor, const FString& ErrorContext);
	static EMGCGameFeaturePluginState ConvertInitialFeatureStateToTargetState(EMGCBuiltInAutoState InInitialState);

private:
	TSharedPtr<IPlugin> Plugin;
	EMGCGameFeaturePluginState InitialState = EMGCGameFeaturePluginState::UnknownStatus;

	EMGCGameFeaturePluginState GetDefaultState() const;
	void ChangeDefaultState(EMGCGameFeaturePluginState DesiredState);

	/** Updates the given json object with values in this descriptor */
	void UpdateJson(FJsonObject& JsonObject, FPluginDescriptor& Descriptor) const;

	bool GetPluginJson(TSharedPtr<FJsonObject>& OutJsonObject, FText& FailReason) const;

};
