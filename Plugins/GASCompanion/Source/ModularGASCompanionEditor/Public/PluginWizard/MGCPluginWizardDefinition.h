// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "IPluginWizardDefinition.h"

struct FMGCPluginTemplateDescription;

class FMGCPluginWizardDefinition : public IPluginWizardDefinition
{
public:
	FMGCPluginWizardDefinition();

	virtual TSharedPtr<FMGCPluginTemplateDescription> GetSelectedTemplateCustom() const;
	virtual void OnTemplateSelectionChangedCustom(TSharedPtr<FMGCPluginTemplateDescription> InSelectedItem, ESelectInfo::Type SelectInfo);
	virtual bool GetTemplateIconPathCustom(TSharedRef<FMGCPluginTemplateDescription> InTemplate, FString& OutIconPath) const;

	virtual const TArray<TSharedRef<FMGCPluginTemplateDescription>>& GetTemplatesSourceCustom() const;

	// Begin IPluginWizardDefinition interface
	virtual const TArray<TSharedRef<FPluginTemplateDescription>>& GetTemplatesSource() const override;
	// virtual TSharedPtr<FPluginTemplateDescription> GetSelectedTemplate() const override;
	virtual void ClearTemplateSelection() override;
	virtual bool HasValidTemplateSelection() const override;

	virtual bool CanShowOnStartup() const override { return false; }
	virtual bool HasModules() const override;
	virtual bool IsMod() const override;
	virtual void OnShowOnStartupCheckboxChanged(ECheckBoxState CheckBoxState) override {}
	virtual ECheckBoxState GetShowOnStartupCheckBoxState() const override { return ECheckBoxState::Undetermined; }
	virtual TSharedPtr<class SWidget> GetCustomHeaderWidget() override { return nullptr; }
	virtual FText GetInstructions() const override;

	virtual bool GetPluginIconPath(FString& OutIconPath) const override;
	virtual EHostType::Type GetPluginModuleDescriptor() const override;
	virtual ELoadingPhase::Type GetPluginLoadingPhase() const override;
	virtual bool GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate, FString& OutIconPath) const override;
	virtual FString GetPluginFolderPath() const override;
	virtual TArray<FString> GetFoldersForSelection() const override;
	virtual void PluginCreated(const FString& PluginName, bool bWasSuccessful) const override;

#if ENGINE_MAJOR_VERSION < 5
	virtual void OnTemplateSelectionChanged(TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo) override;
	virtual ESelectionMode::Type GetSelectionMode() const override;
	virtual TArray<TSharedPtr<FPluginTemplateDescription>> GetSelectedTemplates() const override;
	virtual bool AllowsEnginePlugins() const override;
	virtual bool CanContainContent() const override;
#else
	virtual TSharedPtr<FPluginTemplateDescription> GetSelectedTemplate() const override;
	virtual void OnTemplateSelectionChanged(TSharedPtr<FPluginTemplateDescription> InSelectedItem, ESelectInfo::Type SelectInfo) override;
	virtual ESelectionMode::Type GetSelectionMode() const;
	virtual TArray<TSharedPtr<FPluginTemplateDescription>> GetSelectedTemplates() const;
	virtual bool AllowsEnginePlugins() const;
	virtual bool CanContainContent() const;
#endif
	// End IPluginWizardDefinition interface

private:

	/** The currently selected template definition */
	TSharedPtr<FMGCPluginTemplateDescription> CurrentTemplateDefinition;

	/** The templates available to this definition */
	TArray<TSharedRef<FMGCPluginTemplateDescription>> TemplateDefinitions;

	/** The templates available to this definition */
	TArray<TSharedRef<FPluginTemplateDescription>> PluginTemplateDefinitions;

	/** Base directory of the plugin templates */
	FString PluginBaseDir;

	FString GetFolderForTemplate(TSharedRef<FMGCPluginTemplateDescription> InTemplate) const;
	FString GetFolderForTemplate(TSharedRef<FPluginTemplateDescription> InTemplate) const;

	bool GetTemplateIconPath(TSharedRef<FMGCPluginTemplateDescription> InTemplate, FString& OutIconPath) const;
};
