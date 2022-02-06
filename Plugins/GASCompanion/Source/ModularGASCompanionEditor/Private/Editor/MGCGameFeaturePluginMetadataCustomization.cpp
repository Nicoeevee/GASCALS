// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Editor/MGCGameFeaturePluginMetadataCustomization.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "Dom/JsonValue.h"
#include "PluginDescriptor.h"
#include "Interfaces/IPluginManager.h"
#include "Slate/SMGCGameFeatureStateWidget.h"

#include "Dom/JsonObject.h"
#include "Editor/MGCGameFeatureDataDetailsCustomization.h"
#include "ModularGASCompanionLog.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"

#define LOCTEXT_NAMESPACE "GameFeatures"

//////////////////////////////////////////////////////////////////////////
// FGameFeaturePluginMetadataCustomization

TSharedRef<IDetailCustomization> FMGCGameFeaturePluginMetadataCustomization::MakeInstance()
{
	return MakeShareable(new FMGCGameFeaturePluginMetadataCustomization);
}

void FMGCGameFeaturePluginMetadataCustomization::CustomizeDetailsForPlugin(FMGCPluginEditingContext& InPluginContext, IDetailLayoutBuilder& DetailBuilder)
{
	Plugin = InPluginContext.PluginBeingEdited;

	const FString DescriptorFileName = Plugin->GetDescriptorFileName();

	TSharedPtr<FJsonObject> JsonObject;
	FText FailReason;
	if (!GetPluginJson(JsonObject, FailReason))
	{
		MGC_LOG(Error, TEXT("%s"), *FailReason.ToString())
	}

	const EMGCBuiltInAutoState AutoState = DetermineBuiltInInitialFeatureState(JsonObject, FString());
	InitialState = ConvertInitialFeatureStateToTargetState(AutoState);

	IDetailCategoryBuilder& TopCategory = DetailBuilder.EditCategory("Game Features", FText::GetEmpty(), ECategoryPriority::Important);

	FDetailWidgetRow& ControlRow = TopCategory.AddCustomRow(LOCTEXT("ControlSearchText", "Plugin State Control"))
		.NameContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("InitialState", "Initial State"))
			.Font(DetailBuilder.GetDetailFont())
		]
		.ValueContent()
		[
			SNew(SMGCGameFeatureStateWidget)
			.ToolTipText(LOCTEXT("DefaultStateSwitcherTooltip", "Change the default initial state of this game feature"))
			.CurrentState(this, &FMGCGameFeaturePluginMetadataCustomization::GetDefaultState)
			.OnStateChanged(this, &FMGCGameFeaturePluginMetadataCustomization::ChangeDefaultState)
		];
}

void FMGCGameFeaturePluginMetadataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if(Objects.Num() == 1 && Objects[0].IsValid())
	{
		UMGCPluginMetadaObject* PluginMetadata = Cast<UMGCPluginMetadaObject>(Objects[0].Get());
		check(PluginMetadata);

		// Run any external customizations
		PluginMetadata->Extensions.Reset();
		FMGCPluginEditingContext PluginEditingContext;
		PluginEditingContext.PluginBeingEdited = PluginMetadata->SourcePlugin.Pin();

		// const TSharedPtr<FMGCGameFeaturePluginMetadataCustomization> Extension = MakeShareable(this);
		// if (Extension.IsValid())
		// {
		// 	PluginMetadata->Extensions.Add(Extension);
		// }

		PluginMetadata->Extensions.Add(this);

		CustomizeDetailsForPlugin(PluginEditingContext, DetailBuilder);
	}
}

void FMGCGameFeaturePluginMetadataCustomization::CommitEdits(FPluginDescriptor& Descriptor)
{
	MGC_LOG(Verbose, TEXT("FMGCGameFeaturePluginMetadataCustomization CommitEdits"))

	FString StateStr;
	switch (InitialState)
	{
	case EMGCGameFeaturePluginState::Installed:
		StateStr = TEXT("Installed");
		break;
	case EMGCGameFeaturePluginState::Registered:
		StateStr = TEXT("Registered");
		break;
	case EMGCGameFeaturePluginState::Loaded:
		StateStr = TEXT("Loaded");
		break;
	case EMGCGameFeaturePluginState::Active:
		StateStr = TEXT("Active");
		break;
	default:
		break;
	}

	if (ensure(!StateStr.IsEmpty()))
	{
		const TSharedRef<FJsonObject> PluginJsonObject = MakeShared<FJsonObject>();
		UpdateJson(*PluginJsonObject, Descriptor);
		PluginJsonObject->SetField(TEXT("BuiltInInitialFeatureState"), MakeShared<FJsonValueString>(StateStr));

		FString JsonText;
		// Write the contents of the descriptor to a string. Make sure the writer is destroyed so that the contents are flushed to the string.
		const TSharedRef<TJsonWriter<>> WriterRef = TJsonWriterFactory<>::Create(&JsonText);
		TJsonWriter<>& Writer = WriterRef.Get();
		FJsonSerializer::Serialize(PluginJsonObject, Writer);
		Writer.Close();

#if WITH_EDITOR
		Descriptor.CachedJson = PluginJsonObject;
#endif

		const FString FileName = Plugin->GetDescriptorFileName();
		MGC_LOG(Verbose, TEXT("Commit Edits: Try to write to %s"), *FileName);
		if (!FFileHelper::SaveStringToFile(JsonText, *FileName))
		{
			const FText FailReason = FText::Format(LOCTEXT("FailedToWriteDescriptorFile", "Failed to write plugin descriptor file '{0}'. Perhaps the file is Read-Only?"), FText::FromString(FileName));
			MGC_LOG(Error, TEXT("%s"), *FailReason.ToString());
		}
	}
}

EMGCBuiltInAutoState FMGCGameFeaturePluginMetadataCustomization::DetermineBuiltInInitialFeatureState(const TSharedPtr<FJsonObject> Descriptor, const FString& ErrorContext)
{
	EMGCBuiltInAutoState InitialState = EMGCBuiltInAutoState::Invalid;

	FString InitialFeatureStateStr;
	if (Descriptor->TryGetStringField(TEXT("BuiltInInitialFeatureState"), InitialFeatureStateStr))
	{
		if (InitialFeatureStateStr == TEXT("Installed"))
		{
			InitialState = EMGCBuiltInAutoState::Installed;
		}
		else if (InitialFeatureStateStr == TEXT("Registered"))
		{
			InitialState = EMGCBuiltInAutoState::Registered;
		}
		else if (InitialFeatureStateStr == TEXT("Loaded"))
		{
			InitialState = EMGCBuiltInAutoState::Loaded;
		}
		else if (InitialFeatureStateStr == TEXT("Active"))
		{
			InitialState = EMGCBuiltInAutoState::Active;
		}
		else
		{
			if (!ErrorContext.IsEmpty())
			{
				MGC_LOG(Error, TEXT("Game feature '%s' has an unknown value '%s' for BuiltInInitialFeatureState (expected Installed, Registered, Loaded, or Active); defaulting to Active."), *ErrorContext, *InitialFeatureStateStr);
			}
			InitialState = EMGCBuiltInAutoState::Active;
		}
	}
	else
	{
		// BuiltInAutoRegister. Default to true. If this is a built in plugin, should it be registered automatically (set to false if you intent to load late with LoadAndActivateGameFeaturePlugin)
		bool bBuiltInAutoRegister = true;
		Descriptor->TryGetBoolField(TEXT("BuiltInAutoRegister"), bBuiltInAutoRegister);

		// BuiltInAutoLoad. Default to true. If this is a built in plugin, should it be loaded automatically (set to false if you intent to load late with LoadAndActivateGameFeaturePlugin)
		bool bBuiltInAutoLoad = true;
		Descriptor->TryGetBoolField(TEXT("BuiltInAutoLoad"), bBuiltInAutoLoad);

		// The cooker will need to activate the plugin so that assets can be scanned properly
		bool bBuiltInAutoActivate = true;
		Descriptor->TryGetBoolField(TEXT("BuiltInAutoActivate"), bBuiltInAutoActivate);

		InitialState = EMGCBuiltInAutoState::Installed;
		if (bBuiltInAutoRegister)
		{
			InitialState = EMGCBuiltInAutoState::Registered;
			if (bBuiltInAutoLoad)
			{
				InitialState = EMGCBuiltInAutoState::Loaded;
				if (bBuiltInAutoActivate)
				{
					InitialState = EMGCBuiltInAutoState::Active;
				}
			}
		}

		if (!ErrorContext.IsEmpty())
		{
			//@TODO: Increase severity to a warning after changing existing features
			MGC_LOG(Log, TEXT("Game feature '%s' has no BuiltInInitialFeatureState key, using legacy BuiltInAutoRegister(%d)/BuiltInAutoLoad(%d)/BuiltInAutoActivate(%d) values to arrive at initial state."),
				*ErrorContext,
				bBuiltInAutoRegister ? 1 : 0,
				bBuiltInAutoLoad ? 1 : 0,
				bBuiltInAutoActivate ? 1 : 0);
		}
	}

	return InitialState;
}

EMGCGameFeaturePluginState FMGCGameFeaturePluginMetadataCustomization::ConvertInitialFeatureStateToTargetState(EMGCBuiltInAutoState InInitialState)
{
	EMGCGameFeaturePluginState InitialState;
	switch (InInitialState)
	{
	default:
	case EMGCBuiltInAutoState::Invalid:
		InitialState = EMGCGameFeaturePluginState::UnknownStatus;
		break;
	case EMGCBuiltInAutoState::Installed:
		InitialState = EMGCGameFeaturePluginState::Installed;
		break;
	case EMGCBuiltInAutoState::Registered:
		InitialState = EMGCGameFeaturePluginState::Registered;
		break;
	case EMGCBuiltInAutoState::Loaded:
		InitialState = EMGCGameFeaturePluginState::Loaded;
		break;
	case EMGCBuiltInAutoState::Active:
		InitialState = EMGCGameFeaturePluginState::Active;
		break;
	}
	return InitialState;
}

EMGCGameFeaturePluginState FMGCGameFeaturePluginMetadataCustomization::GetDefaultState() const
{
	return InitialState;
}

void FMGCGameFeaturePluginMetadataCustomization::ChangeDefaultState(const EMGCGameFeaturePluginState DesiredState)
{
	InitialState = DesiredState;
}

void FMGCGameFeaturePluginMetadataCustomization::UpdateJson(FJsonObject& JsonObject, FPluginDescriptor& Descriptor) const
{
	// JsonObject.SetNumberField(TEXT("FileVersion"), EProjectDescriptorVersion::Latest);
	JsonObject.SetNumberField(TEXT("FileVersion"), 3);
	JsonObject.SetNumberField(TEXT("Version"), Descriptor.Version);
	JsonObject.SetStringField(TEXT("VersionName"), Descriptor.VersionName);
	JsonObject.SetStringField(TEXT("FriendlyName"), Descriptor.FriendlyName);
	JsonObject.SetStringField(TEXT("Description"), Descriptor.Description);
	JsonObject.SetStringField(TEXT("Category"), Descriptor.Category);
	JsonObject.SetStringField(TEXT("CreatedBy"), Descriptor.CreatedBy);
	JsonObject.SetStringField(TEXT("CreatedByURL"), Descriptor.CreatedByURL);
	JsonObject.SetStringField(TEXT("DocsURL"), Descriptor.DocsURL);
	JsonObject.SetStringField(TEXT("MarketplaceURL"), Descriptor.MarketplaceURL);
	JsonObject.SetStringField(TEXT("SupportURL"), Descriptor.SupportURL);

	if (Descriptor.EngineVersion.Len() > 0)
	{
		JsonObject.SetStringField(TEXT("EngineVersion"), Descriptor.EngineVersion);
	}
	else
	{
		JsonObject.RemoveField(TEXT("EngineVersion"));
	}

	if (Descriptor.EditorCustomVirtualPath.Len() > 0)
	{
		JsonObject.SetStringField(TEXT("EditorCustomVirtualPath"), Descriptor.EditorCustomVirtualPath);
	}
	else
	{
		JsonObject.RemoveField(TEXT("EditorCustomVirtualPath"));
	}

	if (Descriptor.EnabledByDefault != EPluginEnabledByDefault::Unspecified)
	{
		JsonObject.SetBoolField(TEXT("EnabledByDefault"), (Descriptor.EnabledByDefault == EPluginEnabledByDefault::Enabled));
	}
	else
	{
		JsonObject.RemoveField(TEXT("EnabledByDefault"));
	}

	JsonObject.SetBoolField(TEXT("CanContainContent"), Descriptor.bCanContainContent);
	JsonObject.SetBoolField(TEXT("IsBetaVersion"), Descriptor.bIsBetaVersion);
	JsonObject.SetBoolField(TEXT("IsExperimentalVersion"), Descriptor.bIsExperimentalVersion);
	JsonObject.SetBoolField(TEXT("Installed"), Descriptor.bInstalled);

	if (Descriptor.SupportedTargetPlatforms.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> SupportedTargetPlatformValues;
		for (const FString& SupportedTargetPlatform : Descriptor.SupportedTargetPlatforms)
		{
			SupportedTargetPlatformValues.Add(MakeShareable(new FJsonValueString(SupportedTargetPlatform)));
		}
		JsonObject.SetArrayField(TEXT("SupportedTargetPlatforms"), SupportedTargetPlatformValues);
	}
	else
	{
		JsonObject.RemoveField(TEXT("SupportedTargetPlatforms"));
	}

	if (Descriptor.SupportedPrograms.Num() > 0)
	{
		TArray<TSharedPtr<FJsonValue>> SupportedProgramValues;
		for (const FString& SupportedProgram : Descriptor.SupportedPrograms)
		{
			SupportedProgramValues.Add(MakeShareable(new FJsonValueString(SupportedProgram)));
		}
		JsonObject.SetArrayField(TEXT("SupportedPrograms"), SupportedProgramValues);
	}
	else
	{
		JsonObject.RemoveField(TEXT("SupportedPrograms"));
	}

	if (Descriptor.bIsPluginExtension)
	{
		JsonObject.SetBoolField(TEXT("bIsPluginExtension"), Descriptor.bIsPluginExtension);
	}
	else
	{
		JsonObject.RemoveField(TEXT("bIsPluginExtension"));
	}

	FModuleDescriptor::UpdateArray(JsonObject, TEXT("Modules"), Descriptor.Modules);

	FLocalizationTargetDescriptor::UpdateArray(JsonObject, TEXT("LocalizationTargets"), Descriptor.LocalizationTargets);

	if (Descriptor.bRequiresBuildPlatform)
	{
		JsonObject.SetBoolField(TEXT("RequiresBuildPlatform"), Descriptor.bRequiresBuildPlatform);
	}
	else
	{
		JsonObject.RemoveField(TEXT("RequiresBuildPlatform"));
	}

	if (Descriptor.bIsHidden)
	{
		JsonObject.SetBoolField(TEXT("Hidden"), Descriptor.bIsHidden);
	}
	else
	{
		JsonObject.RemoveField(TEXT("Hidden"));
	}

	if (Descriptor.bExplicitlyLoaded)
	{
		JsonObject.SetBoolField(TEXT("ExplicitlyLoaded"), Descriptor.bExplicitlyLoaded);
	}
	else
	{
		JsonObject.RemoveField(TEXT("ExplicitlyLoaded"));
	}

	Descriptor.PreBuildSteps.UpdateJson(JsonObject, TEXT("PreBuildSteps"));
	Descriptor.PostBuildSteps.UpdateJson(JsonObject, TEXT("PostBuildSteps"));

	FPluginReferenceDescriptor::UpdateArray(JsonObject, TEXT("Plugins"), Descriptor.Plugins);
}

bool FMGCGameFeaturePluginMetadataCustomization::GetPluginJson(TSharedPtr<FJsonObject>& OutJsonObject, FText& FailReason) const
{
	const FString DescriptorFileName = Plugin->GetDescriptorFileName();

	FString PluginJon;
	if (!FFileHelper::LoadFileToString(PluginJon, *DescriptorFileName))
	{
		FailReason = FText::Format(LOCTEXT("FailedToLoadDescriptorFile", "Failed to open descriptor file '{0}'"), FText::FromString(DescriptorFileName));
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(PluginJon);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		FailReason = FText::Format(LOCTEXT("FailedToReadDescriptorFile", "Failed to read file '{0}'"), FText::FromString(DescriptorFileName));
		return false;
	}

	OutJsonObject = JsonObject;

	return true;
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
