// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "PluginWizard/MGCPluginWizardDefinition.h"

#include "ModularGASCompanionEditorTypes.h"
#include "Interfaces/IPluginManager.h"
#include "PluginWizard/MGCGameFeaturePluginTemplateDescription.h"

#define LOCTEXT_NAMESPACE "MGCNewPluginWizard"

FMGCPluginWizardDefinition::FMGCPluginWizardDefinition()
{
	PluginBaseDir = IPluginManager::Get().FindPlugin(TEXT("PluginBrowser"))->GetBaseDir();

	const FString PluginTemplateDir = IPluginManager::Get().FindPlugin(TEXT("GASCompanion"))->GetBaseDir() / TEXT("Templates");
	const TSharedPtr<FMGCPluginTemplateDescription> ContentOnlyTemplate = MakeShareable(new FMGCGameFeaturePluginTemplateDescription(
		LOCTEXT("PluginWizard_NewGFPContentOnlyLabel", "Game Feature (Content Only)"),
		LOCTEXT("PluginWizard_NewGFPContentOnlyDesc", "Create a new Game Feature Plugin."),
		PluginTemplateDir / TEXT("GameFeaturePluginContentOnly")
	));

	TemplateDefinitions.Add(ContentOnlyTemplate.ToSharedRef());
}

TSharedPtr<FMGCPluginTemplateDescription> FMGCPluginWizardDefinition::GetSelectedTemplateCustom() const
{
	return CurrentTemplateDefinition;
}

void FMGCPluginWizardDefinition::OnTemplateSelectionChangedCustom(TSharedPtr<FMGCPluginTemplateDescription> InSelectedItem, ESelectInfo::Type SelectInfo)
{
	CurrentTemplateDefinition = InSelectedItem;
}

bool FMGCPluginWizardDefinition::GetTemplateIconPathCustom(TSharedRef<FMGCPluginTemplateDescription> InTemplate, FString& OutIconPath) const
{
	bool bRequiresDefaultIcon = false;

	const FString TemplateFolderName = GetFolderForTemplate(InTemplate);

	OutIconPath = TemplateFolderName / TEXT("Resources/Icon128.png");
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*OutIconPath))
	{
		OutIconPath = PluginBaseDir / TEXT("Resources/DefaultIcon128.png");
		bRequiresDefaultIcon = true;
	}

	return bRequiresDefaultIcon;
}

const TArray<TSharedRef<FMGCPluginTemplateDescription>>& FMGCPluginWizardDefinition::GetTemplatesSourceCustom() const
{
	return TemplateDefinitions;
}

const TArray<TSharedRef<FPluginTemplateDescription>>& FMGCPluginWizardDefinition::GetTemplatesSource() const
{
	return PluginTemplateDefinitions;
}

void FMGCPluginWizardDefinition::ClearTemplateSelection()
{
	CurrentTemplateDefinition.Reset();
}

bool FMGCPluginWizardDefinition::HasValidTemplateSelection() const
{
	return CurrentTemplateDefinition.IsValid();
}

bool FMGCPluginWizardDefinition::HasModules() const
{
	const FString SourceFolderPath = GetPluginFolderPath() / TEXT("Source");
	return FPaths::DirectoryExists(SourceFolderPath);
}

bool FMGCPluginWizardDefinition::IsMod() const
{
	return false;
}

FText FMGCPluginWizardDefinition::GetInstructions() const
{
	return LOCTEXT("ChoosePluginTemplate", "Choose a template and then specify a name to create a new plugin.");
}

bool FMGCPluginWizardDefinition::GetPluginIconPath(FString& OutIconPath) const
{
	return GetTemplateIconPath(CurrentTemplateDefinition.ToSharedRef(), OutIconPath);
}

EHostType::Type FMGCPluginWizardDefinition::GetPluginModuleDescriptor() const
{
	EHostType::Type ModuleDescriptorType = EHostType::Runtime;

	if (CurrentTemplateDefinition.IsValid())
	{
		ModuleDescriptorType = CurrentTemplateDefinition->ModuleDescriptorType;
	}

	return ModuleDescriptorType;
}

ELoadingPhase::Type FMGCPluginWizardDefinition::GetPluginLoadingPhase() const
{
	ELoadingPhase::Type Phase = ELoadingPhase::Default;

	if (CurrentTemplateDefinition.IsValid())
	{
		Phase = CurrentTemplateDefinition->LoadingPhase;
	}

	return Phase;
}

bool FMGCPluginWizardDefinition::GetTemplateIconPath(TSharedRef<FPluginTemplateDescription> InTemplate, FString& OutIconPath) const
{
	bool bRequiresDefaultIcon = false;

	const FString TemplateFolderName = GetFolderForTemplate(InTemplate);

	OutIconPath = TemplateFolderName / TEXT("Resources/Icon128.png");
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*OutIconPath))
	{
		OutIconPath = PluginBaseDir / TEXT("Resources/DefaultIcon128.png");
		bRequiresDefaultIcon = true;
	}

	return bRequiresDefaultIcon;
}

FString FMGCPluginWizardDefinition::GetPluginFolderPath() const
{
	return GetFolderForTemplate(CurrentTemplateDefinition.ToSharedRef());
}

TArray<FString> FMGCPluginWizardDefinition::GetFoldersForSelection() const
{
	TArray<FString> SelectedFolders;

	if (CurrentTemplateDefinition.IsValid())
	{
		SelectedFolders.Add(GetFolderForTemplate(CurrentTemplateDefinition.ToSharedRef()));
	}

	return SelectedFolders;
}

void FMGCPluginWizardDefinition::PluginCreated(const FString& PluginName, bool bWasSuccessful) const
{
}

#if ENGINE_MAJOR_VERSION < 5
void FMGCPluginWizardDefinition::OnTemplateSelectionChanged(TArray<TSharedRef<FPluginTemplateDescription>> InSelectedItems, ESelectInfo::Type SelectInfo)
{
}
#else
TSharedPtr<FPluginTemplateDescription> FMGCPluginWizardDefinition::GetSelectedTemplate() const
{
	return TSharedPtr<FPluginTemplateDescription>();
}

void FMGCPluginWizardDefinition::OnTemplateSelectionChanged(TSharedPtr<FPluginTemplateDescription> InSelectedItem, ESelectInfo::Type SelectInfo)
{
}
#endif

ESelectionMode::Type FMGCPluginWizardDefinition::GetSelectionMode() const
{
	return ESelectionMode::Type::Single;
}

TArray<TSharedPtr<FPluginTemplateDescription>> FMGCPluginWizardDefinition::GetSelectedTemplates() const
{
	return {};
}

bool FMGCPluginWizardDefinition::AllowsEnginePlugins() const
{
	return false;
}

bool FMGCPluginWizardDefinition::CanContainContent() const
{
	return true;
}

FString FMGCPluginWizardDefinition::GetFolderForTemplate(const TSharedRef<FMGCPluginTemplateDescription> InTemplate) const
{
	return InTemplate->OnDiskPath;
}

FString FMGCPluginWizardDefinition::GetFolderForTemplate(const TSharedRef<FPluginTemplateDescription> InTemplate) const
{
	return InTemplate->OnDiskPath;
}

bool FMGCPluginWizardDefinition::GetTemplateIconPath(TSharedRef<FMGCPluginTemplateDescription> InTemplate, FString& OutIconPath) const
{
	bool bRequiresDefaultIcon = false;

	FString TemplateFolderName = GetFolderForTemplate(InTemplate);

	OutIconPath = TemplateFolderName / TEXT("Resources/Icon128.png");
	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*OutIconPath))
	{
		OutIconPath = PluginBaseDir / TEXT("Resources/DefaultIcon128.png");
		bRequiresDefaultIcon = true;
	}

	return bRequiresDefaultIcon;
}

#undef LOCTEXT_NAMESPACE
