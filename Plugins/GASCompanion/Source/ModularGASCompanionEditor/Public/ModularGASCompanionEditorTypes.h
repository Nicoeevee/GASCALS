// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleDescriptor.h"

struct FPluginDescriptor;
class IPlugin;

struct FMGCPluginEditingContext
{
	TSharedPtr<IPlugin> PluginBeingEdited;
};

enum class EMGCBuiltInAutoState : uint8
{
	Invalid,
	Installed,
	Registered,
	Loaded,
	Active
};

/**
 * Description of a plugin template
 */
struct FMGCPluginTemplateDescription
{
	/** Name of this template in the GUI */
	FText Name;

	/** Description of this template in the GUI */
	FText Description;

	/** Path to the directory containing template files */
	FString OnDiskPath;

	/** Brush resource for the image that is dynamically loaded */
	TSharedPtr<FSlateDynamicImageBrush> PluginIconDynamicImageBrush;

	/** Sorting priority (higer values go to the top of the list) */
	int32 SortPriority = 0;

	/** Can the plugin contain content? */
	bool bCanContainContent = false;

	/** Can the plugin be in the engine folder? */
	bool bCanBePlacedInEngine = true;

	/** What is the expected ModuleDescriptor type for this plugin? */
	EHostType::Type ModuleDescriptorType;

	/** What is the expected Loading Phase for this plugin? */
	ELoadingPhase::Type LoadingPhase;

	/** Called just before the plugin is created */
	virtual void CustomizeDescriptorBeforeCreation(FPluginDescriptor& Descriptor) {}

	/** Called after the plugin has been created */
	virtual void OnPluginCreated(TSharedPtr<IPlugin> NewPlugin) {}

	/** Called to perform *additional* path validation when the path is modified (the bCanBePlacedInEngine validation will have already occurred and passed by this point) */
	virtual bool ValidatePathForPlugin(const FString& ProposedAbsolutePluginPath, FText& OutErrorMessage)
	{
		OutErrorMessage = FText::GetEmpty();
		return true;
	}

	/** Called to enforce any restrictions this template has on paths when it is first selected (so it doesn't generate an error unnecessarily) */
	virtual void UpdatePathWhenTemplateSelected(FString& InOutPath) {}

	/** Called to change away from special folders if needed */
	virtual void UpdatePathWhenTemplateUnselected(FString& InOutPath) {}

	/** Constructor */
	FMGCPluginTemplateDescription(FText InName, FText InDescription, FString InOnDiskPath, bool InCanContainContent, EHostType::Type InModuleDescriptorType, ELoadingPhase::Type InLoadingPhase = ELoadingPhase::Default)
		: Name(InName)
		, Description(InDescription)
		, OnDiskPath(InOnDiskPath)
		, bCanContainContent(InCanContainContent)
		, ModuleDescriptorType(InModuleDescriptorType)
		, LoadingPhase(InLoadingPhase)
	{
	}

	virtual ~FMGCPluginTemplateDescription() {}
};
