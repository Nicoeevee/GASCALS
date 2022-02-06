// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Slate/SMGCGameFeatureStateWidget.h"
#include "Slate/SMGCSegmentedControl.h"
#include "SlateOptMacros.h"
#include "Subsystems/MGCGameFeatureStateChangeSubsystem.h"

#define LOCTEXT_NAMESPACE "MGCGameFeatures"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMGCGameFeatureStateWidget::Construct(const FArguments& InArgs)
{
	CurrentState = InArgs._CurrentState;
	PluginURL = InArgs._PluginURL;

	ChildSlot
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SMGCSegmentedControl<EMGCGameFeaturePluginState>)
			.Value(CurrentState)
			.OnValueChanged(InArgs._OnStateChanged)
			.Visibility(this, &SMGCGameFeatureStateWidget::GetNormalVisibility)
			// .ToolTipText(LOCTEXT("StateSwitcherTooltip", "Attempt to change the current state of this game feature"))
			+SMGCSegmentedControl<EMGCGameFeaturePluginState>::Slot(EMGCGameFeaturePluginState::Installed)
				.Text(GetDisplayNameOfState(EMGCGameFeaturePluginState::Installed))
			+SMGCSegmentedControl<EMGCGameFeaturePluginState>::Slot(EMGCGameFeaturePluginState::Registered)
				.Text(GetDisplayNameOfState(EMGCGameFeaturePluginState::Registered))
			+SMGCSegmentedControl<EMGCGameFeaturePluginState>::Slot(EMGCGameFeaturePluginState::Loaded)
				.Text(GetDisplayNameOfState(EMGCGameFeaturePluginState::Loaded))
			+SMGCSegmentedControl<EMGCGameFeaturePluginState>::Slot(EMGCGameFeaturePluginState::Active)
				.Text(GetDisplayNameOfState(EMGCGameFeaturePluginState::Active))
		]

		+SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SMGCSegmentedControl<EMGCGameFeaturePluginState>)
			.Value(CurrentState)
			.OnValueChanged(InArgs._OnStateChanged)
			.Visibility(this, &SMGCGameFeatureStateWidget::GetOutOfSyncVisibility)

			.ToolTipText(LOCTEXT("StateSwitcherTooltip", "Game feature state got ouf ot sync - Set it to Active again to bring back Installed / Registered / Loaded buttons"))
			+SMGCSegmentedControl<EMGCGameFeaturePluginState>::Slot(EMGCGameFeaturePluginState::OutOfSync)
				.Text(GetDisplayNameOfState(EMGCGameFeaturePluginState::OutOfSync))
			+SMGCSegmentedControl<EMGCGameFeaturePluginState>::Slot(EMGCGameFeaturePluginState::Active)
				.Text(GetDisplayNameOfState(EMGCGameFeaturePluginState::Active))
		]

		+SHorizontalBox::Slot()
		.Padding(8.0f, 0.0f, 0.0f, 0.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(this, &SMGCGameFeatureStateWidget::GetStateStatusDisplay)
			.TextStyle(&FMGCEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("ButtonText"))
			.ColorAndOpacity(FLinearColor::FromSRGBColor(FColor::FromHex("#FFDC1AFF")))
		]
	];
}

EVisibility SMGCGameFeatureStateWidget::GetNormalVisibility() const
{
	const EMGCGameFeaturePluginState PluginState = UMGCGameFeatureStateChangeSubsystem::Get().GetPluginState(PluginURL);
	return PluginState == EMGCGameFeaturePluginState::OutOfSync ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility SMGCGameFeatureStateWidget::GetOutOfSyncVisibility() const
{
	const EMGCGameFeaturePluginState PluginState = UMGCGameFeatureStateChangeSubsystem::Get().GetPluginState(PluginURL);
	return PluginState == EMGCGameFeaturePluginState::OutOfSync ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SMGCGameFeatureStateWidget::GetDisplayNameOfState(EMGCGameFeaturePluginState State)
{
	switch (State)
	{
	case EMGCGameFeaturePluginState::OutOfSync: return LOCTEXT("OutOfSyncStateDisplayName", "Unknown Status");
	case EMGCGameFeaturePluginState::Uninitialized: return LOCTEXT("UninitializedStateDisplayName", "Uninitialized");
	case EMGCGameFeaturePluginState::UnknownStatus: return LOCTEXT("UnknownStatusStateDisplayName", "UnknownStatus");
	case EMGCGameFeaturePluginState::CheckingStatus: return LOCTEXT("CheckingStatusStateDisplayName", "CheckingStatus");
	case EMGCGameFeaturePluginState::StatusKnown: return LOCTEXT("StatusKnownStateDisplayName", "StatusKnown");
	case EMGCGameFeaturePluginState::Uninstalling: return LOCTEXT("UninstallingStateDisplayName", "Uninstalling");
	case EMGCGameFeaturePluginState::Downloading: return LOCTEXT("DownloadingStateDisplayName", "Downloading");
	case EMGCGameFeaturePluginState::Installed: return LOCTEXT("InstalledStateDisplayName", "Installed");
	case EMGCGameFeaturePluginState::Unmounting: return LOCTEXT("UnmountingStateDisplayName", "Unmounting");
	case EMGCGameFeaturePluginState::Mounting: return LOCTEXT("MountingStateDisplayName", "Mounting");
	case EMGCGameFeaturePluginState::WaitingForDependencies: return LOCTEXT("WaitingForDependenciesStateDisplayName", "WaitingForDependencies");
	case EMGCGameFeaturePluginState::Unregistering: return LOCTEXT("UnregisteringStateDisplayName", "Unregistering");
	case EMGCGameFeaturePluginState::Registering: return LOCTEXT("RegisteringStateDisplayName", "Registering");
	case EMGCGameFeaturePluginState::Registered: return LOCTEXT("RegisteredStateDisplayName", "Registered");
	case EMGCGameFeaturePluginState::Unloading: return LOCTEXT("UnloadingStateDisplayName", "Unloading");
	case EMGCGameFeaturePluginState::Loading: return LOCTEXT("LoadingStateDisplayName", "Loading");
	case EMGCGameFeaturePluginState::Loaded: return LOCTEXT("LoadedStateDisplayName", "Loaded");
	case EMGCGameFeaturePluginState::Deactivating: return LOCTEXT("DeactivatingStateDisplayName", "Deactivating");
	case EMGCGameFeaturePluginState::Activating: return LOCTEXT("ActivatingStateDisplayName", "Activating");
	case EMGCGameFeaturePluginState::Active: return LOCTEXT("ActiveStateDisplayName", "Active");
	default:
		check(0);
		return FText::GetEmpty();
	}
}

FText SMGCGameFeatureStateWidget::GetStateStatusDisplay() const
{
	// Display the current state/transition for anything but the four acceptable destination states (which are already covered by the switcher)
	const EMGCGameFeaturePluginState State = CurrentState.Get();
	switch (State)
	{
		case EMGCGameFeaturePluginState::OutOfSync:
		case EMGCGameFeaturePluginState::Active:
		case EMGCGameFeaturePluginState::Installed:
		case EMGCGameFeaturePluginState::Loaded:
		case EMGCGameFeaturePluginState::Registered:
			return FText::GetEmpty();
		default:
			return GetDisplayNameOfState(State);
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
