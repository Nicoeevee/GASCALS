// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

class FMGCEditorStyle
{
public:

	static void Initialize();

	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	static const ISlateStyle& Get();

	static FName GetStyleSetName();

	static FName LaunchPadTitleKey;
	static FName LaunchPadPageTitleKey;
	static FName LaunchPadItemTitleKey;
	static FName LaunchPadItemSubTitleKey;

private:

	static TSharedRef<class FSlateStyleSet> Create();
	static TSharedRef<class FSlateStyleSet> CreateStyle(FName StyleSetName);
	static TSharedPtr<class FSlateStyleSet> StyleInstance;

	static FSlateBrush ToFSlateBrush(const FSlateColor& InColor);
};
