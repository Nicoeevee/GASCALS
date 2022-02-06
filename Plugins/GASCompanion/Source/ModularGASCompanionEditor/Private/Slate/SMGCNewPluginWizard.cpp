// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Slate/SMGCNewPluginWizard.h"

#include "DesktopPlatformModule.h"
#include "ModularGASCompanionEditorTypes.h"
#include "PluginUtils.h"
#include "SourceCodeNavigation.h"
#include "Framework/Notifications/NotificationManager.h"
#include "PluginWizard/MGCPluginWizardDefinition.h"
#include "Slate/MGCEditorStyle.h"
#include "Interfaces/IMainFrameModule.h"
#include "PluginWizard/MGCPluginUtils.h"
#include "Slate/SMGCFilePathBlock.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "MGCNewPluginWizard"

SMGCNewPluginWizard::SMGCNewPluginWizard()
	: bIsPluginPathValid(false)
	, bIsPluginNameValid(false)
	, bIsSelectedPathInEngine(false)
{
	AbsoluteGamePluginPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*FPaths::ProjectPluginsDir());
	FPaths::MakePlatformFilename(AbsoluteGamePluginPath);
	AbsoluteEnginePluginPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*FPaths::EnginePluginsDir());
	FPaths::MakePlatformFilename(AbsoluteEnginePluginPath);
}

void SMGCNewPluginWizard::Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab, TSharedPtr<FMGCPluginWizardDefinition> InPluginWizardDefinition)
{
	OwnerTab = InOwnerTab;

	PluginWizardDefinition = InPluginWizardDefinition;

	// Prepare to create the descriptor data field
	DescriptorData = NewObject<UMGCNewPluginDescriptorData>();
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
		DetailsViewArgs.bAllowFavoriteSystem = false;
		// DetailsViewArgs.bShowObjectLabel = false;
		DetailsViewArgs.bHideSelectionTip = true;
	}
	TSharedPtr<IDetailsView> DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);

	if (!PluginWizardDefinition.IsValid())
	{
		// PluginWizardDefinition = MakeShared<FMGCPluginWizardDefinition>(IsContentOnlyProject());
		PluginWizardDefinition = MakeShared<FMGCPluginWizardDefinition>();
	}
	check(PluginWizardDefinition.IsValid());

	// Ensure that nothing is selected in the plugin wizard definition
	PluginWizardDefinition->ClearTemplateSelection();

	IPluginWizardDefinition* WizardDef = PluginWizardDefinition.Get();

	// Check if the Plugin Wizard is trying to make mods instead of generic plugins. This will slightly change in 4.26
	if (PluginWizardDefinition->IsMod())
	{
		AbsoluteGamePluginPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*FPaths::ProjectModsDir());
		FPaths::MakePlatformFilename(AbsoluteGamePluginPath);
	}

	LastBrowsePath = AbsoluteGamePluginPath;
	PluginFolderPath = AbsoluteGamePluginPath;
	bIsPluginPathValid = true;

	// Create the list view and ensure that it exists
	GenerateListViewWidget();
	check(ListView.IsValid());

	TSharedPtr<SWidget> HeaderWidget = PluginWizardDefinition->GetCustomHeaderWidget();
	FText PluginNameTextHint = PluginWizardDefinition->IsMod() ? LOCTEXT("ModNameTextHint", "Mod Name") : LOCTEXT("PluginNameTextHint", "Plugin Name");

	const FMargin PaddingAmount(5.0f);

	TSharedRef<SVerticalBox> MainContent = SNew(SVerticalBox)
	+SVerticalBox::Slot()
	.Padding(PaddingAmount)
	.AutoHeight()
	[
		SNew(SHorizontalBox)

		// Custom header widget display
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(PaddingAmount)
		[
			HeaderWidget.IsValid() ? HeaderWidget.ToSharedRef() : SNullWidget::NullWidget
		]

		// Instructions
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(PaddingAmount)
		.HAlign(HAlign_Fill)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.Padding(PaddingAmount)
			.VAlign(VAlign_Center)
			.FillHeight(1.0f)
			[
				SNew(STextBlock)
				.Text(WizardDef, &IPluginWizardDefinition::GetInstructions)
				.AutoWrapText(true)
			]
		]
	]
	+SVerticalBox::Slot()
	.Padding(PaddingAmount)
	[
		// main list of plugins
		ListView.ToSharedRef()
	]
	+SVerticalBox::Slot()
	.AutoHeight()
	.Padding(PaddingAmount)
	.HAlign(HAlign_Center)
	[
		SAssignNew(FilePathBlock, SMGCFilePathBlock)
		.OnBrowseForFolder(this, &SMGCNewPluginWizard::OnBrowseButtonClicked)
		.FolderPath(this, &SMGCNewPluginWizard::GetPluginDestinationPath)
		.Name(this, &SMGCNewPluginWizard::GetCurrentPluginName)
		.NameHint(PluginNameTextHint)
		.OnFolderChanged(this, &SMGCNewPluginWizard::OnFolderPathTextChanged)
		.OnNameChanged(this, &SMGCNewPluginWizard::OnPluginNameTextChanged)
	];

	// Add the descriptor data object if it exists
	if (DescriptorData.IsValid() && DescriptorDetailView.IsValid())
	{
		DescriptorDetailView->SetObject(DescriptorData.Get());

		MainContent->AddSlot()
		.AutoHeight()
		.Padding(PaddingAmount)
		[
			DescriptorDetailView.ToSharedRef()
		];
	}

	MainContent->AddSlot()
	.AutoHeight()
	[
		SNew(SBox)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(PaddingAmount)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged(this, &SMGCNewPluginWizard::OnEnginePluginCheckboxChanged)
			.IsChecked(this, &SMGCNewPluginWizard::IsEnginePlugin)
			.Visibility_Lambda([=]()
			{
				FMGCPluginTemplateDescription* Template = PluginWizardDefinition->GetSelectedTemplateCustom().Get();
				return ((Template != nullptr) && Template->bCanBePlacedInEngine) ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.ToolTipText(LOCTEXT("EnginePluginButtonToolTip", "Toggles whether this plugin will be created in the current project or the engine directory."))
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EnginePluginCheckbox", "Is Engine Plugin"))
			]
		]
	];

	if (PluginWizardDefinition->CanShowOnStartup())
	{
		MainContent->AddSlot()
		.AutoHeight()
		.Padding(PaddingAmount)
		[
			SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.OnCheckStateChanged(WizardDef, &IPluginWizardDefinition::OnShowOnStartupCheckboxChanged)
				.IsChecked(WizardDef, &IPluginWizardDefinition::GetShowOnStartupCheckBoxState)
				.ToolTipText(LOCTEXT("ShowOnStartupToolTip", "Toggles whether this wizard will show when the editor is launched."))
				.Content()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ShowOnStartupCheckbox", "Show on Startup"))
				]
			]
		];
	}

	// Checkbox to show the plugin's content directory when the plugin is created
	MainContent->AddSlot()
	.AutoHeight()
	.Padding(PaddingAmount)
	[
		SNew(SBox)
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		[
			SAssignNew(ShowPluginContentDirectoryCheckBox, SCheckBox)
			.IsChecked(ECheckBoxState::Checked)
			.Visibility_Lambda([=]()
			{
				FMGCPluginTemplateDescription* Template = PluginWizardDefinition->GetSelectedTemplateCustom().Get();
				return ((Template != nullptr) && Template->bCanContainContent) ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.ToolTipText(LOCTEXT("ShowPluginContentDirectoryToolTip", "Shows the content directory after creation."))
			.Content()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ShowPluginContentDirectoryText", "Show Content Directory"))
			]
		]
	];

	FText CreateButtonLabel = PluginWizardDefinition->IsMod() ? LOCTEXT("CreateModButtonLabel", "Create Mod") : LOCTEXT("CreatePluginButtonLabel", "Create Plugin");

	MainContent->AddSlot()
	.AutoHeight()
	.Padding(5)
	.HAlign(HAlign_Right)
	[
		SNew(SButton)
		.ContentPadding(5)
		.TextStyle(FEditorStyle::Get(), "LargeText")
		.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
		.IsEnabled(this, &SMGCNewPluginWizard::CanCreatePlugin)
		.HAlign(HAlign_Center)
		.Text(CreateButtonLabel)
		.OnClicked(this, &SMGCNewPluginWizard::OnCreatePluginClicked)
	];

	ChildSlot
	[
		MainContent
	];
}

void SMGCNewPluginWizard::OnFolderPathTextChanged(const FText& InText)
{
	PluginFolderPath = InText.ToString();
	FPaths::MakePlatformFilename(PluginFolderPath);
	ValidateFullPluginPath();
}

TSharedRef<ITableRow> SMGCNewPluginWizard::OnGenerateTemplateRow(TSharedRef<FMGCPluginTemplateDescription> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	const float PaddingAmount = FMGCEditorStyle::Get().GetFloat("PluginTile.Padding");
	const float ThumbnailImageSize = FMGCEditorStyle::Get().GetFloat("PluginTile.ThumbnailImageSize");

	GeneratePluginTemplateDynamicBrush(InItem);

	return SNew(STableRow< TSharedRef<FMGCPluginTemplateDescription> >, OwnerTable)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.Padding(PaddingAmount)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(PaddingAmount)
				[
					SNew(SHorizontalBox)

					// Template thumbnail image
					+ SHorizontalBox::Slot()
					.Padding(PaddingAmount)
					.AutoWidth()
					[
						SNew(SBox)
						.WidthOverride(ThumbnailImageSize)
						.HeightOverride(ThumbnailImageSize)
						[
							SNew(SImage)
							.Image(InItem->PluginIconDynamicImageBrush.IsValid() ? InItem->PluginIconDynamicImageBrush.Get() : nullptr)
						]
					]

					// Template name and description
					+ SHorizontalBox::Slot()
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(PaddingAmount)
						[
							SNew(STextBlock)
							.Text(InItem->Name)
							.TextStyle(FMGCEditorStyle::Get(), "PluginTile.NameText")
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(PaddingAmount)
						[
							SNew(SRichTextBlock)
							.Text(InItem->Description)
							.TextStyle(FMGCEditorStyle::Get(), "PluginTile.DescriptionText")
							.AutoWrapText(true)
						]
					]
				]
			]
		];

}

TSharedRef<ITableRow> SMGCNewPluginWizard::OnGenerateTemplateTile(TSharedRef<FMGCPluginTemplateDescription> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	const float PaddingAmount = FMGCEditorStyle::Get().GetFloat("PluginTile.Padding");
	const float ThumbnailImageSize = FMGCEditorStyle::Get().GetFloat("PluginTile.ThumbnailImageSize");

	GeneratePluginTemplateDynamicBrush(InItem);

	return SNew(STableRow< TSharedRef<FPluginTemplateDescription> >, OwnerTable)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("NoBorder"))
			.Padding(PaddingAmount)
			.ToolTipText(InItem->Description)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(PaddingAmount)
				[
					SNew(SVerticalBox)

					// Template thumbnail image
					+ SVerticalBox::Slot()
					.Padding(PaddingAmount)
					.AutoHeight()
					[
						SNew(SBox)
						.WidthOverride(ThumbnailImageSize)
						.HeightOverride(ThumbnailImageSize)
						[
							SNew(SImage)
							.Image(InItem->PluginIconDynamicImageBrush.IsValid() ? InItem->PluginIconDynamicImageBrush.Get() : nullptr)
						]
					]

					// Template name
					+ SVerticalBox::Slot()
					.Padding(PaddingAmount)
					.FillHeight(1.0)
					.VAlign(VAlign_Center)
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.Padding(PaddingAmount)
						.HAlign(HAlign_Center)
						.FillWidth(1.0)
						[
							SNew(STextBlock)
							.Text(InItem->Name)
							.TextStyle(FMGCEditorStyle::Get(), "PluginTile.DescriptionText")
							.AutoWrapText(true)
							.Justification(ETextJustify::Center)
						]
					]
				]
			]
		];
}

void SMGCNewPluginWizard::OnTemplateSelectionChanged(TSharedPtr<FMGCPluginTemplateDescription> InItem, ESelectInfo::Type SelectInfo)
{
	// Forward the set of selected items to the plugin wizard definition
	TSharedPtr<FMGCPluginTemplateDescription> SelectedItem;

	if (ListView.IsValid())
	{
		if (ListView->GetSelectedItems().Num() > 0)
		{
			SelectedItem = ListView->GetSelectedItems()[0];
		}
	}

	if (PluginWizardDefinition.IsValid())
	{
		const TSharedPtr<FMGCPluginTemplateDescription> OldSelectedItem = PluginWizardDefinition->GetSelectedTemplateCustom();

		if (OldSelectedItem != SelectedItem)
		{
			if (OldSelectedItem.IsValid())
			{
				OldSelectedItem->UpdatePathWhenTemplateUnselected(PluginFolderPath);
			}

			PluginWizardDefinition->OnTemplateSelectionChangedCustom(SelectedItem, SelectInfo);

			if (SelectedItem.IsValid())
			{
				SelectedItem->UpdatePathWhenTemplateSelected(PluginFolderPath);
			}
		}
	}

	ValidateFullPluginPath();
}

void SMGCNewPluginWizard::OnPluginNameTextChanged(const FText& InText)
{
	PluginNameText = InText;
	ValidateFullPluginPath();
}

FReply SMGCNewPluginWizard::OnBrowseButtonClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		void* ParentWindowWindowHandle = NULL;

		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
		const TSharedPtr<SWindow>& MainFrameParentWindow = MainFrameModule.GetParentWindow();
		if (MainFrameParentWindow.IsValid() && MainFrameParentWindow->GetNativeWindow().IsValid())
		{
			ParentWindowWindowHandle = MainFrameParentWindow->GetNativeWindow()->GetOSWindowHandle();
		}

		FString FolderName;
		const FString Title = LOCTEXT("NewPluginBrowseTitle", "Choose a plugin location").ToString();
		const bool bFolderSelected = DesktopPlatform->OpenDirectoryDialog(
			FSlateApplication::Get().FindBestParentWindowHandleForDialogs(AsShared()),
			Title,
			LastBrowsePath,
			FolderName
			);

		if (bFolderSelected)
		{
			LastBrowsePath = FolderName;
			OnFolderPathTextChanged(FText::FromString(FolderName));
		}
	}

	return FReply::Handled();
}

void SMGCNewPluginWizard::ValidateFullPluginPath()
{
	// Check for issues with path
	bIsPluginPathValid = false;
	bool bIsNewPathValid = true;
	FText FolderPathError;

	if (!FPaths::ValidatePath(GetPluginDestinationPath().ToString(), &FolderPathError))
	{
		bIsNewPathValid = false;
	}

	if (bIsNewPathValid)
	{
		bool bFoundKnownPath = false;
		bool bPathIsInEngine = false;
		FString AbsolutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForWrite(*GetPluginDestinationPath().ToString());
		FPaths::MakePlatformFilename(AbsolutePath);

		if (AbsolutePath.StartsWith(AbsoluteGamePluginPath))
		{
			bFoundKnownPath = true;
			bPathIsInEngine = false;
		}
		else if (AbsolutePath.StartsWith(AbsoluteEnginePluginPath))
		{
			bFoundKnownPath = true;
			bPathIsInEngine = true;
		}
		else
		{
			// Path is neither engine nor game, so this path will be added to the additional plugin directories for
			// the project when the plugin is created
		}

		bIsSelectedPathInEngine = bPathIsInEngine;

		// Run this path by the current template to make sure it's OK with it
		TSharedPtr<FMGCPluginTemplateDescription> Template = PluginWizardDefinition->GetSelectedTemplateCustom();
		if (Template.IsValid() && bIsNewPathValid)
		{
			if (bFoundKnownPath && bPathIsInEngine && !Template->bCanBePlacedInEngine)
			{
				bIsNewPathValid = false;
				if (FApp::IsInstalled())
				{
					FolderPathError = LOCTEXT("TemplateCannotBeInEngine_InstalledBuild", "Plugins created in installed builds cannot be placed in the engine folder");
				}
				else
				{
					FolderPathError = LOCTEXT("TemplateCannotBeInEngine", "Plugins created from this template cannot be placed in the engine folder");
				}
			}
		}

		if (bIsNewPathValid && Template.IsValid())
		{
			bIsNewPathValid = Template->ValidatePathForPlugin(AbsolutePath, /*out*/ FolderPathError);
		}
	}

	bIsPluginPathValid = bIsNewPathValid;
	FilePathBlock->SetFolderPathError(FolderPathError);

	// Check for issues with name. Fail silently if text is empty
	FText PluginNameError;
	bIsPluginNameValid = !GetCurrentPluginName().IsEmpty() && FPluginUtils::ValidateNewPluginNameAndLocation(GetCurrentPluginName().ToString(), PluginFolderPath, &PluginNameError);
	FilePathBlock->SetNameError(PluginNameError);
}

bool SMGCNewPluginWizard::CanCreatePlugin() const
{
	return bIsPluginPathValid && bIsPluginNameValid && PluginWizardDefinition->HasValidTemplateSelection();
}

FText SMGCNewPluginWizard::GetPluginDestinationPath() const
{
	return FText::FromString(PluginFolderPath);
}

FText SMGCNewPluginWizard::GetCurrentPluginName() const
{
	return PluginNameText;
}

ECheckBoxState SMGCNewPluginWizard::IsEnginePlugin() const
{
	return bIsSelectedPathInEngine ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SMGCNewPluginWizard::OnEnginePluginCheckboxChanged(ECheckBoxState NewCheckedState)
{
	const bool bNewEnginePluginState = NewCheckedState == ECheckBoxState::Checked;
	if (bIsSelectedPathInEngine != bNewEnginePluginState)
	{
		bIsSelectedPathInEngine = bNewEnginePluginState;
		if (bIsSelectedPathInEngine)
		{
			PluginFolderPath = AbsoluteEnginePluginPath;
		}
		else
		{
			PluginFolderPath = AbsoluteGamePluginPath;
		}

		ValidateFullPluginPath();
	}
}

FReply SMGCNewPluginWizard::OnCreatePluginClicked()
{
	if (!ensure(!PluginFolderPath.IsEmpty() && !PluginNameText.IsEmpty()))
	{
		// Don't even try to assemble the path or else it may be relative to the binaries folder!
		return FReply::Unhandled();
	}

	TSharedPtr<FMGCPluginTemplateDescription> Template = PluginWizardDefinition->GetSelectedTemplateCustom();
	if (!ensure(Template.IsValid()))
	{
		return FReply::Unhandled();
	}


	const FString PluginName = PluginNameText.ToString();
	const bool bHasModules = PluginWizardDefinition->HasModules();

	FMGCNewPluginParamsWithDescriptor CreationParams;
	CreationParams.TemplateFolders = PluginWizardDefinition->GetFoldersForSelection();
	CreationParams.Descriptor.bCanContainContent = Template->bCanContainContent;

	if (bHasModules)
	{
		CreationParams.Descriptor.Modules.Add(FModuleDescriptor(*PluginName, PluginWizardDefinition->GetPluginModuleDescriptor(), PluginWizardDefinition->GetPluginLoadingPhase()));
	}

	CreationParams.Descriptor.FriendlyName = PluginName;
	CreationParams.Descriptor.Version = 1;
	CreationParams.Descriptor.VersionName = TEXT("1.0");
	CreationParams.Descriptor.Category = TEXT("Other");

	PluginWizardDefinition->GetPluginIconPath(/*out*/ CreationParams.PluginIconPath);
	if (DescriptorData.IsValid())
	{
		CreationParams.Descriptor.CreatedBy = DescriptorData->CreatedBy;
		CreationParams.Descriptor.CreatedByURL = DescriptorData->CreatedByURL;
		CreationParams.Descriptor.Description = DescriptorData->Description;
		CreationParams.Descriptor.bIsBetaVersion = DescriptorData->bIsBetaVersion;
	}

	FPluginUtils::FMountPluginParams MountParams;
	MountParams.bEnablePluginInProject = true;
	MountParams.bUpdateProjectPluginSearchPath = true;
	MountParams.bSelectInContentBrowser = ShowPluginContentDirectoryCheckBox->IsChecked();

	Template->CustomizeDescriptorBeforeCreation(CreationParams.Descriptor);

	FText FailReason;
	TSharedPtr<IPlugin> NewPlugin = FMGCPluginUtils::CreateAndMountNewPlugin(PluginName, PluginFolderPath, CreationParams, MountParams, FailReason);
	const bool bSucceeded = NewPlugin.IsValid();

	PluginWizardDefinition->PluginCreated(PluginName, bSucceeded);

	if (bSucceeded)
	{
		// Let the template create additional assets / modify state after creation
		Template->OnPluginCreated(NewPlugin);

		// Notify that a new plugin has been created
		// FPluginBrowserModule& PluginBrowserModule = FPluginBrowserModule::Get();
		// PluginBrowserModule.BroadcastNewPluginCreated();

		FNotificationInfo Info(FText::Format(LOCTEXT("PluginCreatedSuccessfully", "'{0}' was created successfully."), FText::FromString(PluginName)));
		Info.bUseThrobber = false;
		Info.ExpireDuration = 8.0f;
		FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(SNotificationItem::CS_Success);

		OwnerTab.Pin()->RequestCloseTab();

		if (bHasModules)
		{
			FSourceCodeNavigation::OpenModuleSolution();
		}

		return FReply::Handled();
	}
	else
	{
		const FText Title = LOCTEXT("UnableToCreatePlugin", "Unable to create plugin");
		FMessageDialog::Open(EAppMsgType::Ok, FailReason, &Title);
		return FReply::Unhandled();
	}
}

void SMGCNewPluginWizard::GenerateListViewWidget()
{
	// Get the source of the templates to use for the list view
	const TArray<TSharedRef<FMGCPluginTemplateDescription>>& TemplateSource = PluginWizardDefinition->GetTemplatesSourceCustom();

	ListView = SNew(SListView<TSharedRef<FMGCPluginTemplateDescription>>)
		.SelectionMode(ESelectionMode::Single)
		.ListItemsSource(&TemplateSource)
		.OnGenerateRow(this, &SMGCNewPluginWizard::OnGenerateTemplateRow)
		.OnSelectionChanged(this, &SMGCNewPluginWizard::OnTemplateSelectionChanged);
}

void SMGCNewPluginWizard::GeneratePluginTemplateDynamicBrush(TSharedRef<FMGCPluginTemplateDescription> InItem)
{
	if (!InItem->PluginIconDynamicImageBrush.IsValid())
	{
		// Plugin thumbnail image
		FString Icon128FilePath;
		PluginWizardDefinition->GetTemplateIconPathCustom(InItem, Icon128FilePath);

		const FName BrushName(*Icon128FilePath);
		const FIntPoint Size = FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(BrushName);
		if ((Size.X > 0) && (Size.Y > 0))
		{
			InItem->PluginIconDynamicImageBrush = MakeShareable(new FSlateDynamicImageBrush(BrushName, FVector2D(Size.X, Size.Y)));
		}
	}
}

#undef LOCTEXT_NAMESPACE
