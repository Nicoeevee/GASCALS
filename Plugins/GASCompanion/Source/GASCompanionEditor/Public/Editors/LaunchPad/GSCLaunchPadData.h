// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Editor/GSCEditorTypes.h"
#include "GSCLaunchPadData.generated.h"

USTRUCT()
struct FGSCLaunchPadItemInfo {
	GENERATED_BODY()

	UPROPERTY()
	FString Title;

	UPROPERTY()
	FString SubTitle;

	UPROPERTY()
	FString Description;

	UPROPERTY()
	FString Image;

	UPROPERTY()
	TArray<FGSCLaunchPadItemAction> Actions;

	FGSCLaunchPadItemInfo(const FString& Title, const FString& Description, const FString& Image)
	: Title(Title),
	  Description(Description),
	  Image(Image)
	{}

	FGSCLaunchPadItemInfo(const FString& Title, const FString& SubTitle, const FString& Description, const FString& Image, const TArray<FGSCLaunchPadItemAction>& Actions)
		: Title(Title),
		  SubTitle(SubTitle),
		  Description(Description),
		  Image(Image),
		  Actions(Actions)
	{
	}

	FGSCLaunchPadItemInfo() {}
};

/**
 * Generates Data Structure for LaunchPad items
 */
class GASCOMPANIONEDITOR_API GSCLaunchPadData
{
public:

	static TArray<FGSCLaunchPadItemInfo> RegisteredItems;

	static bool IsExamplesPluginEnabled();
	static bool IsExamplesPluginInstalled();

	static TArray<FGSCLaunchPadItemInfo> CreateItems();
	static FGSCLaunchPadItemInfo CreateItem(const FString Title, const FString Subtitle, const FString Description, const FString Image, TArray<FGSCLaunchPadItemAction> Actions);

	static void RegisterItem(const FString Title, const FString Subtitle, const FString Description, const FString Image, TArray<FGSCLaunchPadItemAction> Actions);

	/** Returns GAS Companion plugin version from .uplugin file */
	static FString GetPluginVersion();

	/** Returns the expected plugin download URL for GASCompanionExamples for LaunchPad */
	static FString GetExamplesPluginDownloadURL();
};
