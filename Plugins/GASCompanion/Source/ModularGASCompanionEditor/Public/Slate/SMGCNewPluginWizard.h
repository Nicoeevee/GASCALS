// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SCompoundWidget.h"
#include "IPluginWizardDefinition.h"
#include "Editor/MGCNewPluginDescriptorData.h"

struct FMGCGameFeaturePluginTemplateDescription;
class SMGCFilePathBlock;
class FMGCPluginWizardDefinition;
struct FMGCPluginTemplateDescription;
class ITableRow;
class STableViewBase;
enum class ECheckBoxState : uint8;

template <typename ItemType>
class SListView;

/**
 * A wizard to create a new plugin
 */
class SMGCNewPluginWizard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMGCNewPluginWizard){}
	SLATE_END_ARGS()

	/** Constructor */
	SMGCNewPluginWizard();

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab, TSharedPtr<FMGCPluginWizardDefinition> InPluginWizardDefinition = nullptr);

private:
	/**
	 * Called when Folder Path textbox changes value
	 * @param InText The new Plugin Folder Path text
	 */
	void OnFolderPathTextChanged(const FText& InText);

	/**
	 * Called to generate a widget for the specified list item
	 * @param Item The template information for this row
	 * @param OwnerTable The table that owns these rows
	 * @return The widget for this template
	 */
	TSharedRef<ITableRow> OnGenerateTemplateRow(TSharedRef<FMGCPluginTemplateDescription> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * Called to generate a widget for the specified tile item
	 * @param Item The template information for this row
	 * @param OwnerTable The table that owns these rows
	 * @return The widget for this template
	 */
	TSharedRef<ITableRow> OnGenerateTemplateTile(TSharedRef<FMGCPluginTemplateDescription> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * Called when the template selection changes
	 */
	void OnTemplateSelectionChanged(TSharedPtr<FMGCPluginTemplateDescription> InItem, ESelectInfo::Type SelectInfo);

	/**
	 * Called when Plugin Name textbox changes value
	 * @param InText The new Plugin name text
	 */
	void OnPluginNameTextChanged(const FText& InText);

	/** Handler for when the Browse button is clicked */
	FReply OnBrowseButtonClicked();

	/**
	 * Validates both the current path and plugin name as the final step in name
	 * validation requires a valid path.
	 */
	void ValidateFullPluginPath();

	/**
	 * Whether we are currently able to create a plugin
	 */
	bool CanCreatePlugin() const;

	/**
	 * Get the path where we will create a plugin
	 */
	FText GetPluginDestinationPath() const;

	/** Get the current name of the plugin */
	FText GetCurrentPluginName() const;

	/**
	 * Whether we will create a plugin in the engine directory
	 */
	ECheckBoxState IsEnginePlugin() const;

	/**
	 * Called when state of Engine plugin checkbox changes
	 * @param NewCheckedState New state of the checkbox
	 */
	void OnEnginePluginCheckboxChanged(ECheckBoxState NewCheckedState);

	/**
	 * This is where all the magic happens.
	 * Create actual plugin using parameters collected from other widgets
	 */
	FReply OnCreatePluginClicked();

	/**
	 * Generates the expected list view for the plugin wizard, based on the wizard's definition
	 */
	void GenerateListViewWidget();

	/**
	 * Generates the dynamic brush resource for a plugin template definition if it has not yet been created
	 */
	void GeneratePluginTemplateDynamicBrush(TSharedRef<FMGCPluginTemplateDescription> InItem);

private:
	/** Additional user-defined descriptor data */
	TWeakObjectPtr<UMGCNewPluginDescriptorData> DescriptorData;

	/** The current plugin wizard definition */
	TSharedPtr<FMGCPluginWizardDefinition> PluginWizardDefinition;

	/** The list view for template selection */
	TSharedPtr<SListView<TSharedRef<FMGCPluginTemplateDescription>>> ListView;

	/** Absolute path to game plugins directory so we don't have to convert it repeatedly */
	FString AbsoluteGamePluginPath;

	/** Absolute path to engine plugins directory so we don't have to convert it repeatedly */
	FString AbsoluteEnginePluginPath;

	/** Last Path used to browse, so that we know it will open dialog */
	FString LastBrowsePath;

	/** Path where you want to create the plugin*/
	FString PluginFolderPath;

	/** Name of the plugin you want to create*/
	FText PluginNameText;

	/** File Path widget that user will choose plugin location and name with */
	TSharedPtr<SMGCFilePathBlock> FilePathBlock;

	/** Check box to show a plugin's content directory once a plugin has been successfully created */
	TSharedPtr<class SCheckBox> ShowPluginContentDirectoryCheckBox;

	/** Whether the path of the plugin entered is currently valid */
	bool bIsPluginPathValid;

	/** Whether the name of the plugin entered is currently valid */
	bool bIsPluginNameValid;

	/** Whether we want to create a plugin in the engine folder */
	bool bIsSelectedPathInEngine;

	/** Tab that owns this wizard so that we can ask to close after completion */
	TWeakPtr<SDockTab> OwnerTab;
};
