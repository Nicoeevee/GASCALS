// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "UI/Widgets/LaunchPad/SGSCLaunchPadDockTab.h"
#include "SlateOptMacros.h"
#include "Core/Logging/GASCompanionEditorLog.h"
#include "Editors/LaunchPad/GSCLaunchPad.h"
#include "UI/Styling/GSCEditorStyle.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Editors/LaunchPad/GSCLaunchPadData.h"
#include "UI/Widgets/LaunchPad/SGSCLaunchPadItem.h"

#define LOCTEXT_NAMESPACE "SGSCLaunchPadDockTab"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SGSCLaunchPadDockTab::Construct(const FArguments& InArgs)
{
	Items = GSCLaunchPadData::CreateItems();
	for (FGSCLaunchPadItemInfo Item : Items)
	{
		TileItems.Add(MakeShared<FGSCLaunchPadItemInfo>(Item));
	}

	const FSlateFontInfo TitleStyle = FGSCEditorStyle::Get().GetFontStyle(FGSCEditorStyle::LaunchPadPageTitleKey);

	ChildSlot
	[
		SNew(SVerticalBox)
        +SVerticalBox::Slot()
        .Padding(5)
        .AutoHeight()
        [
        	SNew(SBorder)
        	.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
        	.BorderBackgroundColor(FLinearColor(0.75f, 0.75f, 0.75f))
        	[
        		SNew(STextBlock)
        		.Text(LOCTEXT("LaunchPadTitleText", "GAS Companion - Launch Pad"))
        		.Font(TitleStyle)
        		.Justification(ETextJustify::Center)
        		.ShadowOffset(FVector2D(1, 2))
        		.ShadowColorAndOpacity(FLinearColor(0, 0, 0, 0.75f))
        	]
        ]

        +SVerticalBox::Slot()
		.Padding(16)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT(
				"LaunchPadDescription",
				"Here you can explore the various example maps included in the plugin.\n\nSome have dependencies on other (free) marketplace assets to work properly, required to be installed / imported in your project before opening the map."
			))
			.Justification(ETextJustify::Left)
		]

        +SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
        	SNew(SScrollBox)
        	+SScrollBox::Slot()
        	[
				SNew(SListView<TSharedPtr<FGSCLaunchPadItemInfo>>)
				.ListItemsSource(&TileItems)
				.OnGenerateRow(this, &SGSCLaunchPadDockTab::GenerateItemWidget)
				.SelectionMode(ESelectionMode::None)
				.ItemHeight(ItemHeight)
        	]
        ]

        +SVerticalBox::Slot()
		.Padding(16)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT(
				"GameIconsCredits",
				"Icons licensed under CC BY 3.0 (game-icons.net - authors: Skoll, Lorc, sbed, Lord Berandas and Delapouite)"
			))
			// .Font(TitleStyle)
			.Justification(ETextJustify::Center)
		]
	];
}

TSharedRef<ITableRow> SGSCLaunchPadDockTab::GenerateItemWidget(const TSharedPtr<FGSCLaunchPadItemInfo> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FGSCLaunchPadItemInfo>>, OwnerTable)
	[
		SNew(SGSCLaunchPadItem)
		.OnStartPluginInstall(this, &SGSCLaunchPadDockTab::OnStartPluginInstall)
		.OnEndPluginInstall(this, &SGSCLaunchPadDockTab::OnEndPluginInstall)
		.CardItem(InItem)
		.CardPadding(8.f)
		.CardHeight(ItemHeight)
	];
}

void SGSCLaunchPadDockTab::OnStartPluginInstall()
{
	bCanCloseTab = false;

	TSharedPtr<SDockTab> DockTab = FGlobalTabmanager::Get()->FindExistingLiveTab(GSCLaunchPad::GSCLaunchPadWindowID);
	if (DockTab.IsValid())
	{
		EDITOR_LOG(Verbose, TEXT("End plugin install, disable tab"))
		DockTab->SetCanCloseTab(SDockTab::FCanCloseTab::CreateRaw(this, &SGSCLaunchPadDockTab::CanCloseTab));
		DockTab->SetEnabled(false);
	}
}

void SGSCLaunchPadDockTab::OnEndPluginInstall()
{
	bCanCloseTab = true;
	EDITOR_LOG(Verbose, TEXT("End plugin install, close tab"))
	GSCLaunchPad::Close();
}

bool SGSCLaunchPadDockTab::CanCloseTab() const
{
	return bCanCloseTab;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
