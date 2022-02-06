// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PluginDescriptor.h"
#include "PluginUtils.h"

class IPlugin;

/**
* Parameters for creating a new plugin.
*/
struct FMGCNewPluginParamsWithDescriptor
{
	/** The description of the plugin */
	FPluginDescriptor Descriptor;

	/** Path to plugin icon to copy in the plugin resources folder */
	FString PluginIconPath;

	/**
	* Folders containing template files to copy into the plugin folder (Required if Descriptor.Modules is not empty).
	* Occurrences of the string PLUGIN_NAME in the filename or file content will be replaced by the plugin name.
	*/
	TArray<FString> TemplateFolders;
};

class MODULARGASCOMPANIONEDITOR_API FMGCPluginUtils
{
public:

	/**
	 * 4.27 Specifics - This one differ from UPluginUtils in 4.27 by passing in CreationParams with a plugin descriptor
	 *
	 * Helper to create and mount a new plugin.
	 * @param PluginName Plugin name
	 * @param PluginLocation Directory that contains the plugin folder
	 * @param CreationParams Plugin creation parameters
	 * @param MountParams Plugin mounting parameters
	 * @param FailReason Reason the plugin creation/mount failed
	 * @return The newly created plugin. If something goes wrong during the creation process, the plugin folder gets deleted and null is returned.
	 * @note Will fail if the plugin already exists
	 */
	static TSharedPtr<IPlugin> CreateAndMountNewPlugin(const FString& PluginName, const FString& PluginLocation, const FMGCNewPluginParamsWithDescriptor& CreationParams, const FPluginUtils::FMountPluginParams& MountParams, FText& FailReason);
};
