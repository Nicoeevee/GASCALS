// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "UI/Widgets/CloneManager/SMissingMapping.h"
#include "SlateOptMacros.h"
#include "Editors/LaunchPad/GSCSampleManagerEditor.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "SMissingMapping"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SMissingMapping::Construct(const FArguments& InArgs)
{
	if (InArgs._MissingActionMappings.Num() > 0)
	{
		MissingActionMappings = InArgs._MissingActionMappings;
	}

	if (InArgs._MissingAxisMappings.Num() > 0)
	{
		MissingAxisMappings = InArgs._MissingAxisMappings;
	}

	const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);
	const float Padding = 5.0f;

	TSharedPtr<SWidget> MissingActionsWidget = SNullWidget::NullWidget;
	TSharedPtr<SWidget> MissingAxisWidget = SNullWidget::NullWidget;

	if (MissingActionMappings.Num() > 0)
	{
		MissingActionsWidget = CreateMissingActionsWidget();
	}

	if (MissingAxisMappings.Num() > 0)
	{
		MissingAxisWidget = CreateMissingAxisWidget();
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.Padding(10)
		.AutoHeight()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("SMissingMappingTitle", "Add Missing Input Bindings"))
			.Font(TitleFont)
			.ShadowOffset(FVector2D(1, 2))
			.ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.5f))
			.Justification(ETextJustify::Center)
		]
		+SVerticalBox::Slot()
		.Padding(Padding)
		.AutoHeight()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("SMissingMappingPromptKey", "The Project contains missing Input Mappings required by this sample. Do you want GAS Companion to create these missing input mappings for you in the project settings ?"))
		]
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(Padding)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			[
				MissingActionsWidget.ToSharedRef()
			]
			+SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			.Padding(0, 5, 0, 0)
			[
				MissingAxisWidget.ToSharedRef()
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(Padding)
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNullWidget::NullWidget
			]
			+SHorizontalBox::Slot()
			.Padding(Padding)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SMissingMappingOK", "OK"))
				.OnClicked_Raw(this, &SMissingMapping::OnOKButtonClicked)
			]
			+SHorizontalBox::Slot()
			.Padding(Padding)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SMissingMappingCancel", "Cancel"))
				.OnClicked_Raw(this, &SMissingMapping::OnCancelButtonClicked)
			]
		]
	];
}

TSharedPtr<SWidget> SMissingMapping::CreateMissingActionsWidget()
{
	TSharedPtr<SVerticalBox> ActionName = SNew(SVerticalBox);
	TSharedPtr<SVerticalBox> ActionKey = SNew(SVerticalBox);

	for (const FInputActionKeyMapping& MissingActionMapping : MissingActionMappings) {
		ActionName->AddSlot() [
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromName(MissingActionMapping.ActionName))
		];

		FString Value = MissingActionMapping.Key.ToString();
		if (MissingActionMapping.bShift) Value += " [Shift]";
		if (MissingActionMapping.bCtrl) Value += " [Ctrl]";
		if (MissingActionMapping.bCmd) Value += " [Cmd]";
		if (MissingActionMapping.bAlt) Value += " [Alt]";

		ActionKey->AddSlot() [
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromString(Value))
		];
	}

	return SNew(SBorder)
	.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
	.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.25f))
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("InputBindingActions", "The following Action mappings are missing and will be added:"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()[ActionName.ToSharedRef()]
				+SHorizontalBox::Slot()[ActionKey.ToSharedRef()]
			]
		]
	];
}

TSharedPtr<SWidget> SMissingMapping::CreateMissingAxisWidget()
{
	TSharedPtr<SVerticalBox> AxisName = SNew(SVerticalBox);
	TSharedPtr<SVerticalBox> AxisKey = SNew(SVerticalBox);
	TSharedPtr<SVerticalBox> AxisScale = SNew(SVerticalBox);

	for (const FInputAxisKeyMapping& MissingAxisMapping : MissingAxisMappings) {
		AxisName->AddSlot() [
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromName(MissingAxisMapping.AxisName))
		];
		AxisKey->AddSlot() [
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromString(MissingAxisMapping.Key.ToString()))
		];
		FString ScaleText = FString::Printf(TEXT("%0.2f"), MissingAxisMapping.Scale);
		AxisScale->AddSlot() [
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromString(ScaleText))
		];
	}

	return SNew(SBorder)
	.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
	.BorderBackgroundColor(FLinearColor(0, 0, 0, 0.25f))
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("InputBindingAxis", "The following Axis mappings are missing and will be added:"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()[AxisName.ToSharedRef()]
				+SHorizontalBox::Slot()[AxisKey.ToSharedRef()]
				+SHorizontalBox::Slot()[AxisScale.ToSharedRef()]
			]
		]
	];
}

void SMissingMapping::CloseContainingWindow()
{
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

FReply SMissingMapping::OnOKButtonClicked()
{
	bShouldBindInputs = true;
	CloseContainingWindow();
	return FReply::Handled();
}

FReply SMissingMapping::OnCancelButtonClicked()
{
	bShouldBindInputs = false;
	CloseContainingWindow();
	return FReply::Handled();
}

bool SMissingMapping::ShouldBindInputs() const
{
	return bShouldBindInputs;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
