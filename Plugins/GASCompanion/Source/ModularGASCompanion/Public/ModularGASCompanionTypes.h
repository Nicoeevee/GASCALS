// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EMGCGameFeaturePluginState : uint8
{

	OutOfSync,					// Custom one. Special state used to indicate customization may be out of sync and dictate visibility of switchers

	Uninitialized,				// Unset. Not yet been set up.
	UnknownStatus,				// Initialized, but the only thing known is the URL to query status.
	CheckingStatus,				// Transition state UnknownStatus -> StatusKnown. The status is in the process of being queried.
	StatusKnown,				// The plugin's information is known, but no action has taken place yet.
	Uninstalling,				// Transition state Installed -> StatusKnown. In the process of removing from local storage.
	Downloading,				// Transition state StatusKnown -> Installed. In the process of adding to local storage.
	Installed,					// The plugin is in local storage (i.e. it is on the hard drive)
	WaitingForDependencies,		// Transition state Installed -> Registered. In the process of loading code/content for all dependencies into memory.
	Unmounting,					// Transition state Registered -> Installed. The content file(s) (i.e. pak file) for the plugin is unmounting.
	Mounting,					// Transition state Installed -> Registered. The content files(s) (i.e. pak file) for the plugin is getting mounted.
	Unregistering,				// Transition state Registered -> Installed. Cleaning up data gathered in Registering.
	Registering,				// Transition state Installed -> Registered. Discovering assets in the plugin, but not loading them, except a few for discovery reasons.
	Registered,					// The assets in the plugin are known, but have not yet been loaded, except a few for discovery reasons.
	Unloading,					// Transition state Loaded -> Registered. In the process of removing code/contnet from memory.
	Loading,					// Transition state Registered -> Loaded. In the process of loading code/content into memory.
	Loaded,						// The plugin is loaded into memory, but not registered with game systems and active.
	Deactivating,				// Transition state Active -> Loaded. Currently unregistering with game systems.
	Activating,					// Transition state Loaded -> Active. Currently registering plugin code/content with game systems.
	Active,						// Plugin is fully loaded and active. It is affecting the game.

	MAX
};

/** Enum listing all possible ability activation input trigger event. */
UENUM(BlueprintType)
enum class EMGCAbilityTriggerEvent : uint8
{
	/** The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button. */
	Started UMETA(DisplayName="Activate on Action Started (recommended)"),

	/**
	 * Triggered for continuous actions that happen every frame while holding an input
	 *
	 * Warning: This value should only be used for Input Actions that you know only trigger once. If your action
	 * triggered event happens on every tick, this might lead to issues with ability activation (since you'll be
	 * trying to activate abilities every frame). When in doubt, use the default Started value.
	 */
	Triggered UMETA(DisplayName="Activate on Action Triggered (use with caution)"),
};
