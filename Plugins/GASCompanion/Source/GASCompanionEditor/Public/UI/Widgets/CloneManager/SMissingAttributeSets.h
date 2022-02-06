// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UAttributeSet;

/**
 *
 */
class GASCOMPANIONEDITOR_API SMissingAttributeSets : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMissingAttributeSets)
	{}

	SLATE_ARGUMENT(TArray<TSubclassOf<UAttributeSet>>, MissingAttributeSets)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	bool ShouldRegisterAttributeSets() const;

protected:
	TArray<TSubclassOf<UAttributeSet>> MissingAttributeSets;

	bool bShouldRegisterAttributeSets = false;

	TSharedPtr<SWidget> CreateMissingAttributeSetsWidget();
	void CloseContainingWindow();

	FReply OnOKButtonClicked();
	FReply OnCancelButtonClicked();
};
