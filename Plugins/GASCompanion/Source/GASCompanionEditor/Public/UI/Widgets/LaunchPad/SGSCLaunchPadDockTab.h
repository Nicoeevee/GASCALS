// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FGSCLaunchPadItemInfo;

class GASCOMPANIONEDITOR_API SGSCLaunchPadDockTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGSCLaunchPadDockTab)
	{}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

protected:
	float ItemHeight = 190.f;
	bool bCanCloseTab = true;

	TArray<FGSCLaunchPadItemInfo> Items;
	TArray<TSharedPtr<FGSCLaunchPadItemInfo>> TileItems;

	TSharedRef<class ITableRow> GenerateItemWidget(TSharedPtr<FGSCLaunchPadItemInfo> InItem, const TSharedRef<class STableViewBase>& OwnerTable);

	void OnStartPluginInstall();
	void OnEndPluginInstall();
	bool CanCloseTab() const;
};
