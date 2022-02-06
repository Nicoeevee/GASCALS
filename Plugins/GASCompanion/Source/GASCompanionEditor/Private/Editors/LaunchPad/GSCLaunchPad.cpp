// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Editors/LaunchPad/GSCLaunchPad.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Core/Logging/GASCompanionEditorLog.h"
#include "UI/Styling/GSCEditorStyle.h"
#include "UI/Widgets/LaunchPad/SGSCLaunchPadDockTab.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "GASCompanionEditorLaunchPad"

FName GSCLaunchPad::GSCLaunchPadWindowID = FName(TEXT("GSCLaunchPad"));

void GSCLaunchPad::Register()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(GSCLaunchPadWindowID, FOnSpawnTab::CreateStatic(&GSCLaunchPad::SpawnLaunchPad))
		.SetDisplayName(LOCTEXT("TabTitle", "GAS Companion - Launch Pad"))
		.SetTooltipText(LOCTEXT("TooltipText", "Browse GAS Companion Examples"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetLevelEditorCategory())
		.SetIcon(FSlateIcon(FGSCEditorStyle::GetStyleSetName(),"GASCompanionEditor.ToolbarItem.IconLaunchPad"))
		.SetAutoGenerateMenuEntry(false);
}

void GSCLaunchPad::Unregister()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GSCLaunchPadWindowID);
}

void GSCLaunchPad::Open()
{
	TSharedPtr<SDockTab> LaunchPadDockTab = FGlobalTabmanager::Get()->TryInvokeTab(GSCLaunchPadWindowID);
}

void GSCLaunchPad::Close()
{
	const TSharedPtr<SDockTab> DockTab = FGlobalTabmanager::Get()->FindExistingLiveTab(GSCLaunchPadWindowID);
	if (DockTab.IsValid())
	{
		DockTab->RequestCloseTab();
	}
}

bool GSCLaunchPad::CanExecuteAction()
{
	// Launch pad tab is disabled for now in UE5 (can't package examples plugin to include pre-built binaries for 5.0, requires GAS Companion,
	// which is a dependency for examples plugin, to be installed in engine and currently we can't do that)
	return FEngineVersion::Current().GetMajor() == 5 ? false : true;
}

TSharedRef<SDockTab> GSCLaunchPad::SpawnLaunchPad(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
	.TabRole(NomadTab)
	[
		SNew(SGSCLaunchPadDockTab)
	];
}

#undef LOCTEXT_NAMESPACE
