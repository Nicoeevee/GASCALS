// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Editor/MGCGameFeatureDataDetailsCustomization.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
// #include "SGameFeatureStateWidget.h"
#include "Widgets/Notifications/SErrorText.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"

#include "Interfaces/IPluginManager.h"
#include "Slate/SMGCGameFeatureStateWidget.h"

#include "GameFeatureData.h"
#include "Slate/MGCEditorStyle.h"
#include "Editor/MGCPluginDescriptorEditor.h"
#include "ModularGASCompanionLog.h"
#include "ModularGASCompanionTypes.h"
#include "Dom/JsonObject.h"
#include "Subsystems/MGCGameFeatureStateChangeSubsystem.h"

#define LOCTEXT_NAMESPACE "GameFeatures"

//////////////////////////////////////////////////////////////////////////
// FMGCGameFeatureDataDetailsCustomization

TSharedRef<IDetailCustomization> FMGCGameFeatureDataDetailsCustomization::MakeInstance()
{
	return MakeShareable(new FMGCGameFeatureDataDetailsCustomization);
}

void FMGCGameFeatureDataDetailsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	ErrorTextWidget = SNew(SErrorText)
		.ToolTipText(LOCTEXT("ErrorTooltip", "The error raised while attempting to change the state of this feature"));

	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& TopCategory = DetailBuilder.EditCategory("Feature State", FText::GetEmpty(), ECategoryPriority::Important);

	PluginURL.Reset();
	ObjectsBeingCustomized.Empty();
	DetailBuilder.GetObjectsBeingCustomized(/*out*/ ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() == 1)
	{
		const UGameFeatureData* GameFeature = CastChecked<const UGameFeatureData>(ObjectsBeingCustomized[0]);

		TArray<FString> PathParts;
		GameFeature->GetOutermost()->GetName().ParseIntoArray(PathParts, TEXT("/"));

		UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();
		Subsystem.GetPluginURLForBuiltInPluginByName(PathParts[0], /*out*/ PluginURL);
		PluginPtr = IPluginManager::Get().FindPlugin(PathParts[0]);

		const float Padding = 8.0f;

		if (PluginPtr.IsValid())
		{
			const FString ShortFilename = FPaths::GetCleanFilename(PluginPtr->GetDescriptorFileName());
			FDetailWidgetRow& EditPluginRow = TopCategory.AddCustomRow(LOCTEXT("InitialStateSearchText", "Initial State Edit Plugin"))
				.NameContent()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("InitialState", "Initial State"))
					.ToolTipText(LOCTEXT("InitialStateTooltip", "The initial or default state of this game feature (determines the state that it will be in at game/editor startup)"))
					.Font(DetailBuilder.GetDetailFont())
				]

				.ValueContent()
				[
					SNew(SHorizontalBox)

					// TODO: 4.27 - Disable display of initial state value for now (still available by clicking edit plugin)
					// +SHorizontalBox::Slot()
					// .AutoWidth()
					// .Padding(0.0f, 0.0f, Padding, 0.0f)
					// .VAlign(VAlign_Center)
					// [
					// 	SNew(STextBlock)
					// 	.Text(this, &FMGCGameFeatureDataDetailsCustomization::GetInitialStateText)
					// 	.Font(DetailBuilder.GetDetailFont())
					// ]

					+SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("EditPluginButton", "Edit Plugin"))
						.ButtonStyle(&FMGCEditorStyle::Get().GetWidgetStyle<FButtonStyle>("EditButtonStyle"))
						.TextStyle(&FMGCEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>("ButtonText"))
						// .ForegroundColor(FLinearColor::White)
						.ForegroundColor(FLinearColor::FromSRGBColor(FColor::FromHex("#C0C0C0FF")))
						.OnClicked_Lambda([this]()
							{
								FMGCPluginDescriptorEditor::OpenEditorWindow(PluginPtr.ToSharedRef(), nullptr, FSimpleDelegate());
								return FReply::Handled();
							})
					]

				];
		}

		FDetailWidgetRow& ControlRow = TopCategory.AddCustomRow(LOCTEXT("ControlSearchText", "Plugin State Control"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CurrentState", "Current State"))
				.ToolTipText(LOCTEXT("CurrentStateTooltip", "The current state of this game feature"))
				.Font(DetailBuilder.GetDetailFont())
			]

			.ValueContent()
			.MinDesiredWidth(920.f)
			[
				SNew(SVerticalBox)
				+SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SMGCGameFeatureStateWidget)
					.PluginURL(PluginURL)
					.ToolTipText(LOCTEXT("StateSwitcherTooltip", "Attempt to change the current state of this game feature"))
					.CurrentState(this, &FMGCGameFeatureDataDetailsCustomization::GetCurrentState)
					.OnStateChanged(this, &FMGCGameFeatureDataDetailsCustomization::ChangeDesiredState)
				]
				+SVerticalBox::Slot()
				.HAlign(HAlign_Left)
				.Padding(0.0f, 4.0f, 0.0f, 0.0f)
				[
					SNew(SHorizontalBox)
					.Visibility_Lambda([=]()
					{
						const EMGCGameFeaturePluginState State = GetCurrentState();
						return State == EMGCGameFeaturePluginState::Active || State == EMGCGameFeaturePluginState::OutOfSync ? EVisibility::Visible : EVisibility::Collapsed;
					})
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(Padding)
					.VAlign(VAlign_Center)
					[
						SNew(SImage)
						.Image(FMGCEditorStyle::Get().GetBrush("Icons.Lock"))
					]
					+SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(0.f, Padding, Padding, Padding))
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.WrapTextAt(300.0f)
						.Text(this, &FMGCGameFeatureDataDetailsCustomization::GetWarningText)
						// .Text(LOCTEXT("Active_PreventingEditing", "Deactivate the feature before editing the Game Feature Data"))
						.Font(DetailBuilder.GetDetailFont())
						.ColorAndOpacity(FLinearColor::FromSRGBColor(FColor::FromHex("#FFDC1AFF")))
					]
				]
				+SVerticalBox::Slot()
				.HAlign(HAlign_Center)
				[
					ErrorTextWidget.ToSharedRef()
				]
			];


//@TODO: This disables the mode switcher widget too (and it's a const cast hack...)
// 		if (IDetailsView* ConstHackDetailsView = const_cast<IDetailsView*>(DetailBuilder.GetDetailsView()))
// 		{
// 			ConstHackDetailsView->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateLambda([CapturedThis = this] { return CapturedThis->GetCurrentState() != EGameFeaturePluginState::Active; }));
// 		}
	}
}

FText FMGCGameFeatureDataDetailsCustomization::GetWarningText() const
{
	const EMGCGameFeaturePluginState PluginState = UMGCGameFeatureStateChangeSubsystem::Get().GetPluginState(PluginURL);
	return PluginState == EMGCGameFeaturePluginState::OutOfSync ?
		LOCTEXT("OutOfSync_WarnUser", "Game feature state got ouf ot sync - Set it to Active again to bring back Installed / Registered / Loaded buttons\n\nIt is likely to happen with 4.27 if State is changed outside of Game Feature Data Asset (via console commands or using GameFeatureSubsytem directly to toggle states)") :
		LOCTEXT("Active_PreventingEditing", "Deactivate the feature before editing the Game Feature Data");
}

EMGCGameFeaturePluginState FMGCGameFeatureDataDetailsCustomization::GetCurrentState() const
{
	// Ask subsystem if state has been updated from one of the switcher actions (Installed, Registered, Loaded, Active checkboxes)
	const EMGCGameFeaturePluginState PluginState = UMGCGameFeatureStateChangeSubsystem::Get().GetPluginState(PluginURL);
	if (PluginState != EMGCGameFeaturePluginState::UnknownStatus)
	{
		return PluginState;
	}

	// Fallback to using .uplugin JSON to determine initial state
	const EMGCBuiltInAutoState AutoState = DetermineBuiltInInitialFeatureState(PluginPtr->GetDescriptor().CachedJson, FString());
	return ConvertInitialFeatureStateToTargetState(AutoState);
}

void FMGCGameFeatureDataDetailsCustomization::ChangeDesiredState(const EMGCGameFeaturePluginState DesiredState)
{
	ErrorTextWidget->SetError(FText::GetEmpty());
	const TWeakPtr<FMGCGameFeatureDataDetailsCustomization> WeakThisPtr = StaticCastSharedRef<FMGCGameFeatureDataDetailsCustomization>(AsShared());

	UGameFeaturesSubsystem& Subsystem = UGameFeaturesSubsystem::Get();

	const EMGCGameFeaturePluginState CurrentState = GetCurrentState();
	if (DesiredState == EMGCGameFeaturePluginState::Active)
	{
		Subsystem.LoadAndActivateGameFeaturePlugin(
			PluginURL,
			FGameFeaturePluginLoadComplete::CreateStatic(&FMGCGameFeatureDataDetailsCustomization::OnOperationCompletedOrFailed, WeakThisPtr, EMGCGameFeaturePluginState::Active)
		);
	}
	else if (DesiredState == EMGCGameFeaturePluginState::Loaded)
	{
		if (CurrentState < EMGCGameFeaturePluginState::Loaded)
		{
			// TODO: 4.27 - LoadGameFeaturePlugin not implemented in engine yet, fallback to DeactivateGameFeaturePlugin
			ErrorTextWidget->SetError(FText::AsCultureInvariant(TEXT("TODO: 4.27 - LoadGameFeaturePlugin not implemented in engine yet, fallback to DeactivateGameFeaturePlugin")));
			// Subsystem.LoadGameFeaturePlugin(PluginURL, FGameFeaturePluginLoadComplete::CreateStatic(&FMGCGameFeatureDataDetailsCustomization::OnOperationCompletedOrFailed, WeakThisPtr));
			Subsystem.DeactivateGameFeaturePlugin(
				PluginURL,
				FGameFeaturePluginDeactivateComplete::CreateStatic(&FMGCGameFeatureDataDetailsCustomization::OnOperationCompletedOrFailed, WeakThisPtr, EMGCGameFeaturePluginState::Loaded)
			);
		}
		else
		{
			Subsystem.DeactivateGameFeaturePlugin(
				PluginURL,
				FGameFeaturePluginDeactivateComplete::CreateStatic(&FMGCGameFeatureDataDetailsCustomization::OnOperationCompletedOrFailed, WeakThisPtr, EMGCGameFeaturePluginState::Loaded)
			);
		}
	}
	else if (DesiredState == EMGCGameFeaturePluginState::Registered)
	{
		if (CurrentState >= EMGCGameFeaturePluginState::Loaded)
		{
			Subsystem.UnloadGameFeaturePlugin(
				PluginURL,
				FGameFeaturePluginDeactivateComplete::CreateStatic(&FMGCGameFeatureDataDetailsCustomization::OnOperationCompletedOrFailed, WeakThisPtr, EMGCGameFeaturePluginState::Registered),
				true
			);
		}
		else
		{
			//@TODO: No public transition from Installed..Registered is exposed yet
			ErrorTextWidget->SetError(FText::AsCultureInvariant(TEXT("TODO: No public transition from Installed..Registered is exposed in engine yet")));
		}
	}
	else if (DesiredState == EMGCGameFeaturePluginState::Installed)
	{
		//@TODO: No public transition from something greater than Installed to Installed is exposed yet
		//@TODO: Do we need to support unregistering?  If not, should remove this button
		Subsystem.UnloadGameFeaturePlugin(
			PluginURL,
			FGameFeaturePluginDeactivateComplete::CreateStatic(&FMGCGameFeatureDataDetailsCustomization::OnOperationCompletedOrFailed, WeakThisPtr, EMGCGameFeaturePluginState::Installed),
			false
		);
	}
}

EMGCBuiltInAutoState FMGCGameFeatureDataDetailsCustomization::DetermineBuiltInInitialFeatureState(TSharedPtr<FJsonObject> Descriptor, const FString& ErrorContext)
{
	EMGCBuiltInAutoState InitialState = EMGCBuiltInAutoState::Invalid;

	FString InitialFeatureStateStr;
	if (Descriptor->TryGetStringField(TEXT("BuiltInInitialFeatureState"), InitialFeatureStateStr))
	{
		if (InitialFeatureStateStr == TEXT("Installed"))
		{
			InitialState = EMGCBuiltInAutoState::Installed;
		}
		else if (InitialFeatureStateStr == TEXT("Registered"))
		{
			InitialState = EMGCBuiltInAutoState::Registered;
		}
		else if (InitialFeatureStateStr == TEXT("Loaded"))
		{
			InitialState = EMGCBuiltInAutoState::Loaded;
		}
		else if (InitialFeatureStateStr == TEXT("Active"))
		{
			InitialState = EMGCBuiltInAutoState::Active;
		}
		else
		{
			if (!ErrorContext.IsEmpty())
			{
				MGC_LOG(Error, TEXT("Game feature '%s' has an unknown value '%s' for BuiltInInitialFeatureState (expected Installed, Registered, Loaded, or Active); defaulting to Active."), *ErrorContext, *InitialFeatureStateStr);
			}
			InitialState = EMGCBuiltInAutoState::Active;
		}
	}
	else
	{
		// BuiltInAutoRegister. Default to true. If this is a built in plugin, should it be registered automatically (set to false if you intent to load late with LoadAndActivateGameFeaturePlugin)
		bool bBuiltInAutoRegister = true;
		Descriptor->TryGetBoolField(TEXT("BuiltInAutoRegister"), bBuiltInAutoRegister);

		// BuiltInAutoLoad. Default to true. If this is a built in plugin, should it be loaded automatically (set to false if you intent to load late with LoadAndActivateGameFeaturePlugin)
		bool bBuiltInAutoLoad = true;
		Descriptor->TryGetBoolField(TEXT("BuiltInAutoLoad"), bBuiltInAutoLoad);

		// The cooker will need to activate the plugin so that assets can be scanned properly
		bool bBuiltInAutoActivate = true;
		Descriptor->TryGetBoolField(TEXT("BuiltInAutoActivate"), bBuiltInAutoActivate);

		InitialState = EMGCBuiltInAutoState::Installed;
		if (bBuiltInAutoRegister)
		{
			InitialState = EMGCBuiltInAutoState::Registered;
			if (bBuiltInAutoLoad)
			{
				InitialState = EMGCBuiltInAutoState::Loaded;
				if (bBuiltInAutoActivate)
				{
					InitialState = EMGCBuiltInAutoState::Active;
				}
			}
		}

		if (!ErrorContext.IsEmpty())
		{
			//@TODO: Increase severity to a warning after changing existing features
			MGC_LOG(Log, TEXT("Game feature '%s' has no BuiltInInitialFeatureState key, using legacy BuiltInAutoRegister(%d)/BuiltInAutoLoad(%d)/BuiltInAutoActivate(%d) values to arrive at initial state."),
				*ErrorContext,
				bBuiltInAutoRegister ? 1 : 0,
				bBuiltInAutoLoad ? 1 : 0,
				bBuiltInAutoActivate ? 1 : 0);
		}
	}

	return InitialState;
}

EMGCGameFeaturePluginState FMGCGameFeatureDataDetailsCustomization::ConvertInitialFeatureStateToTargetState(EMGCBuiltInAutoState InInitialState)
{
	EMGCGameFeaturePluginState InitialState;
	switch (InInitialState)
	{
	default:
	case EMGCBuiltInAutoState::Invalid:
		InitialState = EMGCGameFeaturePluginState::UnknownStatus;
		break;
	case EMGCBuiltInAutoState::Installed:
		InitialState = EMGCGameFeaturePluginState::Installed;
		break;
	case EMGCBuiltInAutoState::Registered:
		InitialState = EMGCGameFeaturePluginState::Registered;
		break;
	case EMGCBuiltInAutoState::Loaded:
		InitialState = EMGCGameFeaturePluginState::Loaded;
		break;
	case EMGCBuiltInAutoState::Active:
		InitialState = EMGCGameFeaturePluginState::Active;
		break;
	}
	return InitialState;
}

void FMGCGameFeatureDataDetailsCustomization::OnOperationCompletedOrFailed(const UE::GameFeatures::FResult& Result, const TWeakPtr<FMGCGameFeatureDataDetailsCustomization> WeakThisPtr, EMGCGameFeaturePluginState CompletedState)
{
	if (Result.HasError())
	{
		const TSharedPtr<FMGCGameFeatureDataDetailsCustomization> StrongThis = WeakThisPtr.Pin();
		if (StrongThis.IsValid())
		{
			StrongThis->ErrorTextWidget->SetError(FText::AsCultureInvariant(Result.GetError()));
		}
	}
	else
	{
		const TSharedPtr<FMGCGameFeatureDataDetailsCustomization> StrongThis = WeakThisPtr.Pin();
		if (StrongThis.IsValid())
		{
			UMGCGameFeatureStateChangeSubsystem::Get().SetPluginState(StrongThis->PluginURL, CompletedState);
		}
	}
}

void FMGCGameFeatureDataDetailsCustomization::OnEditCommitted()
{
}

FReply FMGCGameFeatureDataDetailsCustomization::OnEditPluginClicked()
{
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
