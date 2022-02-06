// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Misc/ScopedSlowTask.h"
#include "HttpModule.h"
#include "Widgets/SCompoundWidget.h"

struct FGSCLaunchPadItemAction;
struct FGSCLaunchPadItemDependency;
struct FGSCLaunchPadItemInfo;
class UGameplayEffect;
class UGameplayAbility;

class GASCOMPANIONEDITOR_API SGSCLaunchPadItem : public SCompoundWidget
{
public:
	DECLARE_DELEGATE(FOnStartPluginInstall);
	DECLARE_DELEGATE(FOnEndPluginInstall);

	SLATE_BEGIN_ARGS(SGSCLaunchPadItem)
	{}

	SLATE_ARGUMENT(TSharedPtr<FGSCLaunchPadItemInfo>, CardItem)
	SLATE_ARGUMENT(float, CardPadding)
	SLATE_ARGUMENT(float, CardHeight)

	SLATE_EVENT(FOnStartPluginInstall, OnStartPluginInstall)
	SLATE_EVENT(FOnEndPluginInstall, OnEndPluginInstall)

	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

protected:
	TSharedPtr<SNotificationItem> DownloadNotification;

	/** Delegate for when plugin install is started */
	FOnStartPluginInstall OnStartPluginInstall;

	/** Delegate for when plugin install is ended */
	FOnEndPluginInstall OnEndPluginInstall;

	/** The last time that the unzip processing was checked for completion. */
	double LastPeriodicUnzipCompletionCheckTime = 0;

	/** The frequency in seconds for zip checks while the dialog is idle. */
	double PeriodicUnzipCompletionCheckFrequency = 2;

	/** Periodic checks for unzip completion */
	bool bPreventPeriodicUnzipCompletionCheck = false;

	bool bNeedsEnablePlugin = false;
	bool bIsInstallingPlugin = false;

	const FString ProjectPluginsDir = FPaths::ProjectPluginsDir();
	const FString PluginZipDestination = FPaths::ProjectPluginsDir() / "GASCompanionExamples.zip";
	const FString PluginDestination = FPaths::ProjectPluginsDir() / "GASCompanionExamples";

	TSharedPtr<FGSCLaunchPadItemInfo> CardItem;

	float CardPadding = 8.f;
	float CardHeight = 190.f;

	static FText GetActionText(FGSCLaunchPadItemAction Action);
	static FText GetActionTooltip(FGSCLaunchPadItemAction Action);

	void OnActionClicked(const FGSCLaunchPadItemAction Action, const FString Title);
	bool IsItemEnabled() const;
	void OnDependencyClicked(const FGSCLaunchPadItemDependency Dependency);

	bool OpenSample(const FGSCLaunchPadItemAction Action, const FString Title);

	static bool IsDependencyInstalled(const FGSCLaunchPadItemDependency Dependency);
	bool EnableExamplesPlugin(FText& OutFailReason);

	bool InstallPlugin(const FString URL, FText& OutFailReason);
	bool DownloadFile(const FString& URL, const FString Destination, FText& OutFailReason);
	void OnRequestProgress(FHttpRequestPtr Request, int32 BytesSent, int32 BytesReceived);
	void OnProcessRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, const FString Destination);

	bool UnzipFile(const FString Archive, const FString ExtractPath, FText& OutFailReason);
	TFunction<void(FString, double)> OnUnzipComplete();

	void CheckUnzipCompletion();
};
