// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "UI/Widgets/CloneManager/SMissingGameplayTags.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "SMissingGameplayTags"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SMissingGameplayTags::Construct(const FArguments& InArgs)
{
	if (InArgs._MissingGameplayTags.Num() > 0)
	{
		MissingGameplayTags = InArgs._MissingGameplayTags;
	}

	const FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle("Bold", 16);
	const float Padding = 5.0f;

	const TSharedPtr<SWidget> MissingGameplayTagsWidget = CreateMissingGameplayTagsWidget();

	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.Padding(10)
		.AutoHeight()
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("SMissingGameplayTagsTitle", "Add Missing Gameplay Tags"))
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
			.Text(LOCTEXT("SMissingGameplayTagsPromptKey", "The Project contains missing Gameplay Tags required by this sample. Do you want GAS Companion to create these missing Gameplay Tags for you in the project settings ?"))
		]
		+SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(Padding)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			.HAlign(HAlign_Fill)
			[
				MissingGameplayTagsWidget.ToSharedRef()
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
				.Text(LOCTEXT("SMissingGameplayTagsOK", "OK"))
				.OnClicked_Raw(this, &SMissingGameplayTags::OnOKButtonClicked)
			]
			+SHorizontalBox::Slot()
			.Padding(Padding)
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("SMissingGameplayTagsCancel", "Cancel"))
				.OnClicked_Raw(this, &SMissingGameplayTags::OnCancelButtonClicked)
			]
		]
	];
}

bool SMissingGameplayTags::ShouldRegisterGameplayTags() const
{
	return bShouldRegisterGameplayTags;
}

TSharedPtr<SWidget> SMissingGameplayTags::CreateMissingGameplayTagsWidget()
{
	TSharedPtr<SVerticalBox> TagsVerticalBox = SNew(SVerticalBox);

	for (FString MissingGameplayTag : MissingGameplayTags)
	{
		TagsVerticalBox->AddSlot()[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(FText::FromString(MissingGameplayTag))
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
			.Text(LOCTEXT("GameplayTagsMissing", "The following Gameplay Tags are missing and will be added:"))
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBorder)
			[
				TagsVerticalBox.ToSharedRef()
			]
		]
	];
}

void SMissingGameplayTags::CloseContainingWindow()
{
	TSharedPtr<SWindow> ContainingWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
	if (ContainingWindow.IsValid())
	{
		ContainingWindow->RequestDestroyWindow();
	}
}

FReply SMissingGameplayTags::OnOKButtonClicked()
{
	bShouldRegisterGameplayTags = true;
	CloseContainingWindow();
	return FReply::Handled();
}

FReply SMissingGameplayTags::OnCancelButtonClicked()
{
	bShouldRegisterGameplayTags = false;
	CloseContainingWindow();
	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
