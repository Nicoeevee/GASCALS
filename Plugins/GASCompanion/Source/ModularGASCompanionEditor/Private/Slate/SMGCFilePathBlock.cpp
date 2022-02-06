// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Slate/SMGCFilePathBlock.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Slate/MGCEditorStyle.h"

#define LOCTEXT_NAMESPACE "MGCFilePathBlock"

void SMGCFilePathBlock::Construct(const FArguments& InArgs)
{
	FText BrowseForFolderToolTipText;

	const bool bReadOnlyFolderPath = InArgs._ReadOnlyFolderPath;

	if (bReadOnlyFolderPath)
	{
		BrowseForFolderToolTipText = LOCTEXT("BrowseForFolderDisabled", "You cannot modify this location");
	}
	else
	{
		BrowseForFolderToolTipText = LOCTEXT("BrowseForFolder", "Browse for a folder");
	}

	const float DesiredWidth = 700.0f;
	const float DesiredRatioForPath = 2.0f;

	const float DerivedMinWidthForPath = DesiredWidth * DesiredRatioForPath / (DesiredRatioForPath + 1.0f);
	const float DerivedMinWidthForName = DesiredWidth - DerivedMinWidthForPath;

	ChildSlot
	[
		SNew(SHorizontalBox)
		// Folder input
		+SHorizontalBox::Slot()
		.FillWidth(DesiredRatioForPath)
		[
			SNew(SBox)
			.MinDesiredWidth(DerivedMinWidthForPath)
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SAssignNew(FolderPathTextBox, SEditableTextBox)
					.Text(InArgs._FolderPath)
					// Large right hand padding to make room for the browse button
					.Padding(FMargin(5.f, 3.f, 25.f, 3.f))
					.OnTextChanged(InArgs._OnFolderChanged)
					.OnTextCommitted(InArgs._OnFolderCommitted)
					.IsReadOnly(bReadOnlyFolderPath)
					.HintText(LOCTEXT("Folder", "Folder"))
					// .ForegroundColor(FSlateColor::UseForeground())
					.BackgroundColor(FMGCEditorStyle::Get().GetSlateColor("Color.Hover"))
					.ForegroundColor(FMGCEditorStyle::Get().GetSlateColor("Color.Foreground"))
				]

				+ SOverlay::Slot()
				.HAlign(HAlign_Right)
				.Padding(1)
				[
					SNew(SButton)
					.ButtonStyle(FMGCEditorStyle::Get(), "PluginPath.BrowseButton")
					.OnClicked(InArgs._OnBrowseForFolder)
					.ContentPadding(0)
					.ToolTipText(BrowseForFolderToolTipText)
					.Text(LOCTEXT("...", "..."))
					.IsEnabled(!bReadOnlyFolderPath)
				]
			]
		]
		// Name input
		+ SHorizontalBox::Slot()
		.Padding(FMargin(5.f, 0.f, 0.f, 0.f))
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.MinDesiredWidth(DerivedMinWidthForName)
			[
				SAssignNew(NameTextBox, SEditableTextBox)
				.Text(InArgs._Name)
				.Padding(FMargin(5.f, 3.f))
				.HintText(InArgs._NameHint)
				.OnTextChanged(InArgs._OnNameChanged)
				.OnTextCommitted(InArgs._OnNameCommitted)
				.BackgroundColor(FMGCEditorStyle::Get().GetSlateColor("Color.Hover"))
				.ForegroundColor(FMGCEditorStyle::Get().GetSlateColor("Color.Foreground"))
			]
		]
	];
}

void SMGCFilePathBlock::SetFolderPathError(const FText& ErrorText)
{
	if (FolderPathTextBox.IsValid())
	{
		FolderPathTextBox->SetError(ErrorText);
	}
}

void SMGCFilePathBlock::SetNameError(const FText& ErrorText)
{
	if (NameTextBox.IsValid())
	{
		NameTextBox->SetError(ErrorText);
	}
}

#undef LOCTEXT_NAMESPACE
