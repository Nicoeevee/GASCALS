// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "UI/Widgets/CloneManager/SMissingAttributeSets.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "SSMissingAttributeSets"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMissingAttributeSets::Construct(const FArguments& InArgs)
{
	if (InArgs._MissingAttributeSets.Num() > 0)
	{
		MissingAttributeSets = InArgs._MissingAttributeSets;
	}

	const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);
	const float Padding = 5.0f;

	const TSharedPtr<SWidget> MissingAttributeSetsWidget = CreateMissingAttributeSetsWidget();

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.Padding(10)
		.AutoHeight()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("SMissingAttributeSetsTitle", "Add Missing Attribute Sets"))
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
			.Text(LOCTEXT("SMissingAttributeSetsPromptKey", "This LaunchPad map make use of custom AttributeSets which needs to be registered in GAS Companion Project's Settings. Do you want GAS Companion to register these missing AttributeSets for you in the project settings ?"))
		]
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(Padding)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			[
				MissingAttributeSetsWidget.ToSharedRef()
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
				.Text(LOCTEXT("SMissingAttributeSetsOK", "OK"))
				.OnClicked_Raw(this, &SMissingAttributeSets::OnOKButtonClicked)
			]
			+SHorizontalBox::Slot()
			.Padding(Padding)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SMissingAttributeSetsCancel", "Cancel"))
				.OnClicked_Raw(this, &SMissingAttributeSets::OnCancelButtonClicked)
			]
		]
	];
}

bool SMissingAttributeSets::ShouldRegisterAttributeSets() const
{
	return bShouldRegisterAttributeSets;
}

TSharedPtr<SWidget> SMissingAttributeSets::CreateMissingAttributeSetsWidget()
{
	TSharedPtr<SVerticalBox> AttributeSetsVerticalBox = SNew(SVerticalBox);

	for (const TSubclassOf<UAttributeSet> MissingAttributeSet : MissingAttributeSets)
	{
		AttributeSetsVerticalBox->AddSlot()[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromString(MissingAttributeSet->GetName()))
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
			.Text(LOCTEXT("AttributeSetsMissing", "The following Attribute Sets are missing and will be registered in Project's Settings:"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			[
				AttributeSetsVerticalBox.ToSharedRef()
			]
		]
	];
}

void SMissingAttributeSets::CloseContainingWindow()
{
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

FReply SMissingAttributeSets::OnOKButtonClicked()
{
	bShouldRegisterAttributeSets = true;
	CloseContainingWindow();
	return FReply::Handled();
}

FReply SMissingAttributeSets::OnCancelButtonClicked()
{
	bShouldRegisterAttributeSets = false;
	CloseContainingWindow();
	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
