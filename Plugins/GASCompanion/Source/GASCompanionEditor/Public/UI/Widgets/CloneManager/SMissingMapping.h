// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FInputActionKeyMapping;
struct FInputAxisKeyMapping;

class GASCOMPANIONEDITOR_API SMissingMapping : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMissingMapping)
	{}

	SLATE_ARGUMENT(TArray<FInputActionKeyMapping>, MissingActionMappings)
	SLATE_ARGUMENT(TArray<FInputAxisKeyMapping>, MissingAxisMappings)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	bool ShouldBindInputs() const;

protected:
	TArray<FInputActionKeyMapping> MissingActionMappings;
	TArray<FInputAxisKeyMapping> MissingAxisMappings;

	bool bShouldBindInputs = false;

	TSharedPtr<SWidget> CreateMissingActionsWidget();
	TSharedPtr<SWidget> CreateMissingAxisWidget();
	void CloseContainingWindow();

	FReply OnOKButtonClicked();
	FReply OnCancelButtonClicked();
};
