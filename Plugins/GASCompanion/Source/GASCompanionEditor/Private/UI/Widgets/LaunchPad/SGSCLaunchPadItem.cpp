// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "UI/Widgets/LaunchPad/SGSCLaunchPadItem.h"
#include "ContentBrowserModule.h"
#include "FileHelpers.h"
#include "GameProjectGenerationModule.h"
#include "IContentBrowserSingleton.h"
#include "Core/Logging/GASCompanionEditorLog.h"
#include "UI/Styling/GSCEditorStyle.h"
#include "Widgets/Input/SHyperlink.h"
#include "SlateOptMacros.h"
#include "Editors/LaunchPad/GSCLaunchPad.h"
#include "Interfaces/IHttpResponse.h"
#include "IUATHelperModule.h"
#include "Core/Editor/GSCEditorTypes.h"
#include "Editors/LaunchPad/GSCLaunchPadData.h"
#include "Editors/LaunchPad/GSCSampleManagerEditor.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Interfaces/IProjectManager.h"
#include "Misc/MessageDialog.h"
#include "SettingsEditor/Public/ISettingsEditorModule.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "SGSCLaunchPadCardItem"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGSCLaunchPadItem::Construct(const FArguments& InArgs)
{
	CardItem = InArgs._CardItem;
	CardPadding = InArgs._CardPadding != 0.f ? InArgs._CardPadding : CardPadding;
	CardHeight = InArgs._CardHeight != 0.f ? InArgs._CardHeight : CardHeight;
	OnStartPluginInstall = InArgs._OnStartPluginInstall;
	OnEndPluginInstall = InArgs._OnEndPluginInstall;

	const FString Title = CardItem.IsValid() ? CardItem->Title : "";
	const FString SubTitle = CardItem.IsValid() ? CardItem->SubTitle : "";
	const FString Description = CardItem.IsValid() ? CardItem->Description : "";

	TArray<FGSCLaunchPadItemAction> Actions;
	if (CardItem.IsValid())
	{
		Actions = CardItem->Actions;
	}

	const FSlateIcon SlateIcon = FSlateIcon(FGSCEditorStyle::GetStyleSetName(), FName(CardItem->Image));
	const FSlateBrush* Brush = SlateIcon.GetIcon();
	const FVector2D ImageSize = Brush->GetImageSize();
	// const float ImageSizeX = ImageSize.X;
	const float ImageSizeX = 186.f;
	// const float ImageSizeY = ImageSize.Y;
	const float ImageSizeY = 186.f;

	const FSlateFontInfo TitleStyle = FGSCEditorStyle::Get().GetFontStyle("GASCompanionEditor.LaunchPad.Title");
	const FSlateFontInfo SubTitleStyle = FGSCEditorStyle::Get().GetFontStyle("GASCompanionEditor.LaunchPad.SubTitle");
	bool bHasDependencies = false;

    TSharedPtr<SVerticalBox> VBox = SNew(SVerticalBox);
	TSharedPtr<SHorizontalBox> ActionsHorizontalBox = SNew(SHorizontalBox);

	TSharedPtr<SHorizontalBox> DependenciesHorizontalBox = SNew(SHorizontalBox)
	+SHorizontalBox::Slot()
	.Padding(0.f, 0.f, 4.f, 0.f)
	.VAlign(VAlign_Top)
	.AutoWidth()
	[
		SNew(STextBlock)
		.TextStyle(FGSCEditorStyle::Get(), "Text.Bold")
		.Text(LOCTEXT("LaunchPadItemDependencies", "Dependencies:"))
	];

	if (!Title.IsEmpty())
	{
		VBox->AddSlot()
		.Padding(0, 8.f, 0, 0)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Font(TitleStyle)
			.Justification(ETextJustify::Left)
			.AutoWrapText(true)
			.Text(FText::FromString(Title))
		];
	}

	if (!SubTitle.IsEmpty())
	{
		VBox->AddSlot()
		.Padding(0, 8.f, 0, 0)
		.AutoHeight()
		[
			SNew(STextBlock)
			.Font(SubTitleStyle)
			.Justification(ETextJustify::Left)
			.AutoWrapText(true)
			.Text(FText::FromString(SubTitle))
		];
	}

	if (Actions.Num() > 0)
	{
		int32 ActionIdx = 0;
		for (FGSCLaunchPadItemAction Action : Actions)
		{
			ActionIdx++;

			ActionsHorizontalBox->AddSlot()
			.Padding(ActionIdx == 1 ? 0.f : 8.f, 0.f)
			.AutoWidth()
			[
				SNew(SHyperlink)
				.Style(FEditorStyle::Get(), "Common.GotoNativeCodeHyperlink")
				.OnNavigate(this, &SGSCLaunchPadItem::OnActionClicked, Action, Title)
				.Text(GetActionText(Action))
				.ToolTipText(GetActionTooltip(Action))
			];

			if (Action.Dependencies.Num() > 0)
			{
				bHasDependencies = true;
				int32 DependenciesIdx = 0;
				for (FGSCLaunchPadItemDependency Dependency : Action.Dependencies)
				{
					DependenciesIdx++;

					bool bIsDependencyInstalled = IsDependencyInstalled(Dependency);
					const FName InstalledTextStyle = bIsDependencyInstalled ? FName("Text.Badass") : FName("Text.Tomato");

					// No token definition matched the stream at its current position - fatal error
					FFormatOrderedArguments Args;
					Args.Add(bIsDependencyInstalled ? LOCTEXT("LaunchPadDependencyInstalled", "Installed") : LOCTEXT("LaunchPadDependencyNotInstalled", "Missing"));
					const FText InstalledText = FText::Format(LOCTEXT("LaunchPadDependencyStatus", "({0})"), Args);

					if (DependenciesIdx != 1)
					{
						DependenciesHorizontalBox->AddSlot()
						.Padding(0.f, 0.f, 2.f, 0.f)
						.VAlign(VAlign_Top)
						.AutoWidth()
						[
							SNew(STextBlock)
							.Text(FText::FromString(" / "))
						];
					}

					DependenciesHorizontalBox->AddSlot()
					.Padding(0.f, 0.f, 2.f, 0.f)
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(SHyperlink)
						.Style(FEditorStyle::Get(), "Common.GotoNativeCodeHyperlink")
						.OnNavigate(this, &SGSCLaunchPadItem::OnDependencyClicked, Dependency)
						.Text(FText::FromString(Dependency.Name))
						.ToolTipText(LOCTEXT("LaunchPadDependencyClick", "Click to open the install URL for this asset"))
					];

					DependenciesHorizontalBox->AddSlot()
					.Padding(0.f, 0.f, 2.f, 0.f)
					.VAlign(VAlign_Top)
					.AutoWidth()
					[
						SNew(STextBlock)
						.TextStyle(FGSCEditorStyle::Get(), InstalledTextStyle)
						.Text(InstalledText)
						.ToolTipText(LOCTEXT("LaunchPadDependencyClick", "Click to open the install URL for this asset"))
					];
				}

			}
		}
	}

	if (!Description.IsEmpty())
	{
		TSharedPtr<STextBlock> DescriptionTextBlock = SNew(STextBlock)
          .AutoWrapText(true)
          .Justification(ETextJustify::Left)
          .Text(FText::FromString(Description));

		if (bHasDependencies)
		{
			VBox->AddSlot()
			.Padding(0, 16.f)
			.AutoHeight()
			[
				DescriptionTextBlock.ToSharedRef()
			];

			VBox->AddSlot()
			.Padding(0)
			.FillHeight(1.f)
			[
				DependenciesHorizontalBox.ToSharedRef()
			];
		}
		else
		{
			VBox->AddSlot()
			.Padding(0, 16.f, 0, 0)
			.FillHeight(1.0f)
			[
				DescriptionTextBlock.ToSharedRef()
			];
		}
	}

	if (Actions.Num() > 0)
	{
		VBox->AddSlot()
		.Padding(0, 8.f)
		.AutoHeight()
		[
			ActionsHorizontalBox.ToSharedRef()
		];
	}

	ChildSlot
	[
		SNew(SBox)
		.Padding(FMargin(CardPadding))
		.IsEnabled(this, &SGSCLaunchPadItem::IsItemEnabled)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.MaxWidth(ImageSizeX)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.MaxHeight(ImageSizeY)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(ImageSizeX)
						.HeightOverride(ImageSizeY)
						[
							SNew(SImage)
							.Image(Brush)
						]
					]
				]

				+SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(8.f, 0.f)
				[
					VBox.ToSharedRef()
				]
			]
		]
	];
}

void SGSCLaunchPadItem::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	// Every few seconds, the class name/path is checked for validity in case the disk contents changed and the location is now valid or invalid.
	// After class creation, periodic checks are disabled to prevent a brief message indicating that the class you created already exists.
	// This feature is re-enabled if the user did not restart and began editing parameters again.
	if (!bPreventPeriodicUnzipCompletionCheck && (InCurrentTime > LastPeriodicUnzipCompletionCheckTime + PeriodicUnzipCompletionCheckFrequency))
	{
		CheckUnzipCompletion();
	}
}

FText SGSCLaunchPadItem::GetActionText(const FGSCLaunchPadItemAction Action)
{
	return Action.Type == EGSCLaunchPadActionType::OpenSample ? LOCTEXT("OpenClick", "Open Map") :
		Action.Type == EGSCLaunchPadActionType::Documentation ? LOCTEXT("DocumentationClick", "Documentation") :
		Action.Type == EGSCLaunchPadActionType::InstallExamplesPlugin ? LOCTEXT("InstallPluginClick", "Install Plugin") :
		Action.Type == EGSCLaunchPadActionType::EnableExamplesPlugin ? LOCTEXT("EnablePluginClick", "Enable Plugin") :
		FText::FromString("");
}

FText SGSCLaunchPadItem::GetActionTooltip(const FGSCLaunchPadItemAction Action)
{
	const FString ActionPath = Action.Path;

	if (Action.Type == EGSCLaunchPadActionType::OpenSample)
	{
		return LOCTEXT("OpenTooltip", "Click to open the sample map into your project");
	}

	if (Action.Type == EGSCLaunchPadActionType::Documentation)
	{
		FFormatOrderedArguments Args;
		Args.Add(ActionPath.IsEmpty() ? FText::FromString("") : FText::FromString("(" + ActionPath + """)"));
		return FText::Format(LOCTEXT("DocumentationTooltip", "Click to view online documentation for this sample {0}"), Args);
	}

	if (Action.Type == EGSCLaunchPadActionType::InstallExamplesPlugin)
	{
		FFormatOrderedArguments Args;
		Args.Add(FText::FromString(ActionPath));
		return FText::Format(LOCTEXT("InstallPluginTooltip", "Click to install the plugin as a Project plugin\n\n({0})"), Args);
	}

	if (Action.Type == EGSCLaunchPadActionType::EnableExamplesPlugin)
	{
		return LOCTEXT("EnablePluginTooltip", "Click to enable the plugin (Editor restart will be required)");
	}

	return FText::FromString("");
}

void SGSCLaunchPadItem::OnActionClicked(const FGSCLaunchPadItemAction Action, const FString Title)
{
	EDITOR_LOG(Verbose, TEXT("Clicked action: %s"), *Action.Path);
	if (Action.Type == EGSCLaunchPadActionType::OpenSample)
	{
		OpenSample(Action, Title);
		return;
	}

	if (Action.Type == EGSCLaunchPadActionType::Documentation)
	{
		FPlatformProcess::LaunchURL(*Action.Path, nullptr, nullptr);
		return;
	}

	if (Action.Type == EGSCLaunchPadActionType::InstallExamplesPlugin)
	{
		FText FailReason;
		if (!InstallPlugin(Action.Path, FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			EDITOR_LOG(Error, TEXT("Install plugin failed: %s"), *FailReason.ToString())
		}
	}

	if (Action.Type == EGSCLaunchPadActionType::EnableExamplesPlugin)
	{
		FText FailReason;
		if (!EnableExamplesPlugin(FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			EDITOR_LOG(Error, TEXT("Enable plugin failed: %s"), *FailReason.ToString())
		}
	}
}

bool SGSCLaunchPadItem::IsItemEnabled() const
{
	return !bIsInstallingPlugin;
}

void SGSCLaunchPadItem::OnDependencyClicked(const FGSCLaunchPadItemDependency Dependency)
{
	const FString Url = Dependency.Url;
	if (!Url.IsEmpty())
	{
		FPlatformProcess::LaunchURL(*Url, nullptr, nullptr);
	}
}

bool SGSCLaunchPadItem::OpenSample(const FGSCLaunchPadItemAction Action, const FString Title)
{
	const FString Path = Action.Path;
	const TArray<FGSCLaunchPadItemDependency> Dependencies = Action.Dependencies;

	EDITOR_LOG(Verbose, TEXT("OpenSample %s"), *Path);
	if (!FPackageName::IsValidLongPackageName(Path))
	{
		EDITOR_LOG(Error, TEXT("Cannot OpenSample. Invalid source package name %s"), *Path);
		return false;
	}

	TArray<FGSCLaunchPadItemDependency> MissingDependencies;
	TArray<FString> MissingDependencyNames;
	for (const FGSCLaunchPadItemDependency Dependency : Dependencies)
	{
		EDITOR_LOG(Verbose, TEXT("Check dependency - %s: %s"), *Dependency.Name, *Dependency.Path);

		if (!IsDependencyInstalled(Dependency))
		{
			MissingDependencies.Add(Dependency);
			MissingDependencyNames.Add(Dependency.Name);
		}
	}

	if (MissingDependencies.Num() > 0)
	{
		const FString MissingDependenciesText = FString::Join(MissingDependencyNames, TEXT(", "));
		EDITOR_LOG(Error, TEXT("Cannot OpenSample. Invalid dependencies: %s"), *MissingDependenciesText);

		FString Message = FString::Printf(TEXT("The map \"%s\" can't be opened because of missing dependencies.\n"), *Title);
		Message += TEXT("\n\nMake sure to install / import the following dependencies before opening the map:\n\n");

		for (FString& Str: MissingDependencyNames)
		{
			Message += FString::Printf(TEXT("\n  %s"), *Str);
		}

		const FText MessageText = FText::FromString(Message);
		const FText TitleText = LOCTEXT("LaunchPadMissingDependenciesWarning", "Missing Required Dependencies");

		FMessageDialog::Open(EAppMsgType::Ok, MessageText, &TitleText);
		return false;
	}

	// If there are any unsaved changes to the current level, see if the user wants to save those first.
	if (!FEditorFileUtils::SaveDirtyPackages(true, true, true))
	{
		// The current level has pending changes and the user doesn't want to abandon it yet
		EDITOR_LOG(Error, TEXT("Cannot OpenSample. current level has pending changes and the user doesn't want to abandon it yet %s"), *Path);
		return false;
	}

	// Check for missing gameplay tags, inputs and attribute sets
	const TSubclassOf<AGSCExampleMapManager> ManagerClass = Action.SampleManager;
	bool bSatisfiesGameplayTagRequirements = true;
	bool bSatisfiesAttributeSetsRequirements = true;
	if (ManagerClass)
	{
		AGSCExampleMapManager* SampleManager = Cast<AGSCExampleMapManager>(ManagerClass->GetDefaultObject());
		if (SampleManager)
		{
			AGSCSampleManagerEditor* SampleManagerEditor = NewObject<AGSCSampleManagerEditor>();
			if (SampleManagerEditor)
			{
				SampleManagerEditor->ActionMappings = SampleManager->ActionMappings;
				SampleManagerEditor->AxisMappings = SampleManager->AxisMappings;
				SampleManagerEditor->GameplayTags = SampleManager->GameplayTags;
				SampleManagerEditor->AttributeSets = SampleManager->AttributeSets;

				TArray<FString> GameplayTags = SampleManager->GameplayTags;
				if (SampleManagerEditor->HasMissingActionMappings())
				{
					EDITOR_LOG(Display, TEXT("Missing action mappings, show dialog (%s)"), *SampleManager->GetName())
					SampleManagerEditor->AddMissingActionMapping();
				}

				if (SampleManagerEditor->HasMissingGameplayTags()) {
					EDITOR_LOG(Display, TEXT("Missing gameplay tags, show dialog (%s)"), *SampleManager->GetName())
					bSatisfiesGameplayTagRequirements = SampleManagerEditor->AddMissingGameplayTags();
				}

				if (SampleManagerEditor->HasMissingAttributeSets()) {
					EDITOR_LOG(Display, TEXT("Missing attribute sets, show dialog (%s)"), *SampleManager->GetName())
					bSatisfiesAttributeSetsRequirements = SampleManagerEditor->AddMissingAttributeSets();
				}
			}
		}
	}

	if (!bSatisfiesGameplayTagRequirements)
	{
		EDITOR_LOG(Warning, TEXT("Does not satisfies gameplay tag requirements, prevent map opening"))

		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("NOKSatisfiesGameplayTagRequirements", "Missing GameplayTags for the map, please ensure those tags are available.")
		);

		return false;
	}

	if (!bSatisfiesAttributeSetsRequirements)
	{
		EDITOR_LOG(Warning, TEXT("Does not satisfies attribute sets requirements, prevent map opening"))

		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("NOKSatisfiesdAttributeSetsRequirements", "Missing Attribute Sets for the map, please ensure those AttributeSets are registered in Project's Settings.")
		);

		return false;
	}

	// Load the level
	EDITOR_LOG(Display, TEXT("Load the level %s"), *Path)

	const FString FileToOpen = FPackageName::LongPackageNameToFilename(Path, FPackageName::GetMapPackageExtension());
	const bool bLoadAsTemplate = false;
	const bool bShowProgress = true;
	if (!FEditorFileUtils::LoadMap(FileToOpen, bLoadAsTemplate, bShowProgress))
	{
		EDITOR_LOG(Error, TEXT("Cannot OpenSample. Load map failed %s"), *Path);
		return false;
	}

	if (!Action.Folder.IsEmpty())
	{
		TArray<FString> Folders;
		Folders.Add(Action.Folder);

		FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		ContentBrowserModule.Get().SyncBrowserToFolders(Folders);
		ContentBrowserModule.Get().FocusPrimaryContentBrowser(false);
	}

	GSCLaunchPad::Close();

	return true;
}

bool SGSCLaunchPadItem::IsDependencyInstalled(const FGSCLaunchPadItemDependency Dependency)
{
	if (Dependency.Type == EGSCLaunchPadDependencyType::Module && FModuleManager::Get().IsModuleLoaded(FName(*Dependency.Path)))
	{
		return true;
	}

	if (Dependency.Type == EGSCLaunchPadDependencyType::Content && FPackageName::DoesPackageExist(Dependency.Path, nullptr, nullptr))
	{
		return true;
	}

	return false;
}

bool SGSCLaunchPadItem::EnableExamplesPlugin(FText& OutFailReason)
{
	bool bSuccess = IProjectManager::Get().SetPluginEnabled("GASCompanionExamples", true, OutFailReason);

	if (bSuccess && IProjectManager::Get().IsCurrentProjectDirty())
	{
		FGameProjectGenerationModule::Get().TryMakeProjectFileWriteable(FPaths::GetProjectFilePath());
		bSuccess = IProjectManager::Get().SaveCurrentProjectToDisk(OutFailReason);
	}

	if (bSuccess)
	{
		ISettingsEditorModule* SettingsEditorModule = FModuleManager::GetModulePtr<ISettingsEditorModule>("SettingsEditor");
		if (SettingsEditorModule)
		{
			SettingsEditorModule->OnApplicationRestartRequired();
		}
	}

	return bSuccess;
}

bool SGSCLaunchPadItem::InstallPlugin(const FString URL, FText& OutFailReason)
{
	EDITOR_LOG(Verbose, TEXT("Install Plugin URL: %s into %s"), *URL, *PluginZipDestination)
	bPreventPeriodicUnzipCompletionCheck = false;
	bIsInstallingPlugin = true;

	OnStartPluginInstall.ExecuteIfBound();


	// Check if plugin zip file was already downloaded
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*PluginZipDestination))
	{
		return UnzipFile(PluginZipDestination, PluginDestination, OutFailReason);
	}

	DownloadFile(URL, PluginZipDestination, OutFailReason);

	return true;
}

bool SGSCLaunchPadItem::DownloadFile(const FString& URL, const FString Destination, FText& OutFailReason)
{
	if (URL.IsEmpty())
	{
		OutFailReason = LOCTEXT("InstallPluginFailURLEmpty", "Provided download URL is empty");
		return false;
	}

	FScopedSlowTask DownloadTask(2.f, LOCTEXT("DownloadTask", "Downloading LaunchPad Examples Plugin"));
	DownloadTask.MakeDialog();

	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();

	EDITOR_LOG(Verbose, TEXT("Download URL: %s"), *URL)
	HttpRequest->SetVerb("GET");
	HttpRequest->SetURL(URL);
	HttpRequest->SetTimeout(5.0f);
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &SGSCLaunchPadItem::OnProcessRequestComplete, Destination);
	HttpRequest->OnRequestProgress().BindRaw(this, &SGSCLaunchPadItem::OnRequestProgress);

	DownloadTask.EnterProgressFrame(1.0f);

	FNotificationInfo Info(LOCTEXT("DownloadTask", "Downloading LaunchPad Examples Plugin ... Please wait."));
	Info.Image = nullptr;
	Info.FadeInDuration = 0.2f;
	Info.ExpireDuration = 3.0f;
	Info.FadeOutDuration = 1.0f;
	Info.bUseThrobber = true;
	Info.bUseSuccessFailIcons = true;
	Info.bUseLargeFont = true;
	Info.bFireAndForget = false;
	Info.bAllowThrottleWhenFrameRateIsLow = false;
	DownloadNotification = FSlateNotificationManager::Get().AddNotification(Info);
	DownloadNotification->SetCompletionState(SNotificationItem::CS_Pending);

	// Process the request
	HttpRequest->ProcessRequest();

	return true;
}

void SGSCLaunchPadItem::OnRequestProgress(const FHttpRequestPtr Request, const int32 BytesSent, const int32 BytesReceived)
{
	const FHttpResponsePtr Response = Request->GetResponse();
	if (!Response.IsValid())
	{
		return;
	}

	const int32 FullSize = Response->GetContentLength();
	const float PercentDone = ((float)BytesReceived / (float)FullSize) * 100;

	if (DownloadNotification.IsValid())
	{
		SNotificationItem* NotificationItem = DownloadNotification.Get();
		const FText Text = FText::Format(LOCTEXT("DownloadTaskPercent", "{0}% Downloading LaunchPad Examples Plugin"), FMath::Floor(PercentDone));
		NotificationItem->SetText(Text);
	}
}

void SGSCLaunchPadItem::OnProcessRequestComplete(FHttpRequestPtr Request, const FHttpResponsePtr Response, const bool bSuccess, const FString Destination)
{
	if (!Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()) || !bSuccess)
	{
		EDITOR_LOG(Error, TEXT("OnProcessRequestComplete Download failed: %d"), Response->GetResponseCode())
		if (DownloadNotification.IsValid())
		{
			SNotificationItem* NotificationItem = DownloadNotification.Get();
			NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
			NotificationItem->SetText(FText::Format(LOCTEXT("DownloadError", "Download Failed. Error Code: {0}"), Response->GetResponseCode()));
			NotificationItem->ExpireAndFadeout();
		}

		bIsInstallingPlugin = false;

		OnEndPluginInstall.ExecuteIfBound();

		return;
	}

	if (DownloadNotification.IsValid())
	{
		SNotificationItem* NotificationItem = DownloadNotification.Get();
		NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
		NotificationItem->SetText(LOCTEXT("DownloadComplete", "Download Complete"));
		NotificationItem->ExpireAndFadeout();
	}

	// Save file
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Create directory if need be
	FString Path, Filename, Extension;
	FPaths::Split(Destination, Path, Filename, Extension);
	if (!PlatformFile.DirectoryExists(*Path))
	{
		if (!PlatformFile.CreateDirectoryTree(*Path))
		{
			EDITOR_LOG(Error, TEXT("OnProcessRequestComplete Failed to create directory: %s"), *Path)
			return;
		}
	}

	IFileHandle* FileHandle = PlatformFile.OpenWrite(*Destination);
	if (!FileHandle)
	{
		EDITOR_LOG(Error, TEXT("OnProcessRequestComplete Failed to create directory: %s"), *Path)
		return;
	}

	const bool bWrite = FileHandle->Write(Response->GetContent().GetData(), Response->GetContentLength());
	if (!bWrite)
	{
		EDITOR_LOG(Error, TEXT("OnProcessRequestComplete Failed to write file"))
	}

	delete FileHandle;

	if (bWrite)
	{
		FText FailReason;
		if (!UnzipFile(Destination, PluginDestination, FailReason))
		{
			FMessageDialog::Open(EAppMsgType::Ok, FailReason);
			EDITOR_LOG(Error, TEXT("Failed to unzip file: %s"), *FailReason.ToString())
			return;
		}
	}
}

bool SGSCLaunchPadItem::UnzipFile(const FString Archive, const FString ExtractPath, FText& OutFailReason)
{
#if PLATFORM_WINDOWS
	FText PlatformName = LOCTEXT("PlatformName_Windows", "Windows");
#elif PLATFORM_MAC
	FText PlatformName = LOCTEXT("PlatformName_Mac", "Mac");
#elif PLATFORM_LINUX
	FText PlatformName = LOCTEXT("PlatformName_Linux", "Linux");
#else
	FText PlatformName = LOCTEXT("PlatformName_Other", "Other OS");
#endif

	// Ensure path is full rather than relative (for macs)
	const FString ExtractAbsolutePath = FPaths::ConvertRelativePathToFull(ExtractPath);
	const FString ArchivePath = FPaths::ConvertRelativePathToFull(Archive);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.DirectoryExists(*ExtractAbsolutePath))
	{
		OutFailReason = FText::FromString(FString::Printf(TEXT("Directory already exists: %s"), *ExtractAbsolutePath));
		return false;
	}


	EDITOR_LOG(Verbose, TEXT("Try to unzip file %s to %s"), *ArchivePath, *ExtractAbsolutePath)
	const FString CommandLine = FString::Printf(TEXT("ZipUtils -archive=\"%s\" -extract=\"%s\""), *ArchivePath, *ExtractAbsolutePath);
	EDITOR_LOG(Verbose, TEXT("Command %s"), *CommandLine)

	IUATHelperModule::Get().CreateUatTask(
		CommandLine,
		PlatformName,
		LOCTEXT("ZipTaskName", "Unzipping file"),
		LOCTEXT("ZipTaskShortName", "Unzip Project Task"),
		FEditorStyle::GetBrush(TEXT("MainFrame.CookContent")),
		OnUnzipComplete()
	);

	return true;
}

TFunction<void(FString, double)> SGSCLaunchPadItem::OnUnzipComplete()
{
	return [=](const FString Result, const double ElapsedTime)
	{
		EDITOR_LOG(Verbose, TEXT("OnUnzipComplete %s %d"), *Result, ElapsedTime)
		bNeedsEnablePlugin = true;
	};
}

void SGSCLaunchPadItem::CheckUnzipCompletion()
{
	LastPeriodicUnzipCompletionCheckTime = FSlateApplication::Get().GetCurrentTime();
	if (!bNeedsEnablePlugin)
	{
		return;
	}

	FText FailReason;
	if (!EnableExamplesPlugin(FailReason))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FailReason);
		EDITOR_LOG(Error, TEXT("Failed to enable plugin: %s"), *FailReason.ToString())
	}

	bNeedsEnablePlugin = false;
	bIsInstallingPlugin = false;


	// Delete zip file
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (PlatformFile.FileExists(*PluginZipDestination))
	{
		const bool bDeleteFile = PlatformFile.DeleteFile(*PluginZipDestination);
		EDITOR_LOG(Verbose, TEXT("Deleted file %s"), bDeleteFile ? TEXT("true") : TEXT("false"))
	}

	OnEndPluginInstall.ExecuteIfBound();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
