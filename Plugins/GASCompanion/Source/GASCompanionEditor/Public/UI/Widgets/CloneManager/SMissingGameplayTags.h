// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 *
 */
class GASCOMPANIONEDITOR_API SMissingGameplayTags : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMissingGameplayTags)
	{}

	SLATE_ARGUMENT(TArray<FString>, MissingGameplayTags)
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	bool ShouldRegisterGameplayTags() const;

protected:

	TArray<FString> MissingGameplayTags;

	bool bShouldRegisterGameplayTags = false;

	TSharedPtr<SWidget> CreateMissingGameplayTagsWidget();
	void CloseContainingWindow();

	FReply OnOKButtonClicked();
	FReply OnCancelButtonClicked();
};
