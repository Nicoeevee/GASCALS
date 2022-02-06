// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class SGSCLaunchPadDockTab;
/**
 * LaunchPAD UI Extender. Registers, unregisters and spawn nomad tab to display Launch Pad.
 */
class GSCLaunchPad
{
public:

	// static FName GSCLaunchPadWindowID = FName(TEXT("GSCLaunchPad"));
	static FName GSCLaunchPadWindowID;

	static void Register();
	static void Unregister();

	static void Open();
	static void Close();

	static bool CanExecuteAction();

	static TSharedRef<SDockTab> SpawnLaunchPad(const FSpawnTabArgs& Args);
};
