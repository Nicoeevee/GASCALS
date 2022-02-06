// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "UObject/Object.h"
#include "MGCPluginMetadaObject.generated.h"

class FMGCGameFeaturePluginMetadataCustomization;
class IPlugin;
struct FPluginReferenceDescriptor;
struct FPluginDescriptor;

/**
 * 4.27 specific
 *
 * Description of a plugin editor extension
 */
// struct FMGCPluginEditorExtension : public IDetailCustomization, public TSharedFromThis<FMGCPluginEditorExtension>
class FMGCPluginEditorExtension : public IDetailCustomization
{
public:
	// virtual ~FMGCPluginEditorExtension() override {}
	virtual void CommitEdits(FPluginDescriptor& Descriptor) = 0;
};

/**
 * 4.27 - Specifically done for 4.27 and provide fallback for plugin editing feature,
 * as UPluginMetadataObject is either missing some APIs or not exposed to other modules
 *
 * We use this object to display plugin reference properties using details view.
 */
USTRUCT()
struct FMGCPluginReferenceMetadata
{
	GENERATED_BODY()

public:

	/** Name of the dependency plugin */
	UPROPERTY(EditAnywhere, Category = "Plugin Reference")
	FString Name;

	/** Whether the dependency plugin is optional meaning it will be silently ignored if not present */
	UPROPERTY(EditAnywhere, Category = "Plugin Reference")
	bool bOptional = false;

	/** Whether the dependency plugin should be enabled by default */
	UPROPERTY(EditAnywhere, Category = "Plugin Reference")
	bool bEnabled = true;

	/**
	 * Populate the fields of this object from an existing descriptor.
	 */
	void PopulateFromDescriptor(const FPluginReferenceDescriptor& InDescriptor);

	/**
	 * Copy the metadata fields into a plugin descriptor.
	 */
	void CopyIntoDescriptor(FPluginReferenceDescriptor& OutDescriptor) const;
};

/**
 * 4.27 - Specifically done for 4.27 and provide fallback for plugin editing feature,
 * as UPluginMetadataObject is either missing some APIs or not exposed to other modules
 *
 * We use this object to display plugin properties using details view.
 */
UCLASS()
class MODULARGASCOMPANIONEDITOR_API UMGCPluginMetadaObject : public UObject
{
	GENERATED_BODY()

public:

	/* Default constructor */
	UMGCPluginMetadaObject(const FObjectInitializer& ObjectInitializer);

	/** Path to this this plugin's icon */
	FString TargetIconPath;

	/** Version number for the plugin.  The version number must increase with every version of the plugin, so that the system
	can determine whether one version of a plugin is newer than another, or to enforce other requirements.  This version
	number is not displayed in front-facing UI.  Use the VersionName for that. */
	UPROPERTY(VisibleAnywhere, Category = Details)
	int32 Version;

	/** Name of the version for this plugin.  This is the front-facing part of the version number.  It doesn't need to match
	the version number numerically, but should be updated when the version number is increased accordingly. */
	UPROPERTY(EditAnywhere, Category = Details)
	FString VersionName;

	/** Friendly name of the plugin */
	UPROPERTY(EditAnywhere, Category = Details)
	FString FriendlyName;

	/** Description of the plugin */
	UPROPERTY(EditAnywhere, Category = Details)
	FString Description;

	/** The category that this plugin belongs to */
	UPROPERTY(EditAnywhere, Category = Details)
	FString Category;

	/** The company or individual who created this plugin.  This is an optional field that may be displayed in the user interface. */
	UPROPERTY(EditAnywhere, Category = Details)
	FString CreatedBy;

	/** Hyperlink URL string for the company or individual who created this plugin.  This is optional. */
	UPROPERTY(EditAnywhere, Category = Details)
	FString CreatedByURL;

	/** Documentation URL string. */
	UPROPERTY(EditAnywhere, Category = Details)
	FString DocsURL;

	/** Marketplace URL string. */
	UPROPERTY(EditAnywhere, Category = Details)
	FString MarketplaceURL;

	/** Support URL/email for this plugin. Email addresses must be prefixed with 'mailto:' */
	UPROPERTY(EditAnywhere, Category = Details)
	FString SupportURL;

	/** Can this plugin contain content? */
	UPROPERTY(EditAnywhere, Category = Details)
	bool bCanContainContent;

	/** Marks the plugin as beta in the UI */
	UPROPERTY(EditAnywhere, Category = Details)
	bool bIsBetaVersion;

	/** Plugins used by this plugin */
	UPROPERTY(EditAnywhere, Category = Dependencies)
	TArray<FMGCPluginReferenceMetadata> Plugins;

	/** Plugin this proxy object was constructed from */
	TWeakPtr<IPlugin> SourcePlugin;

	/** Editing extensions */
	// TArray<TSharedPtr<FMGCPluginEditorExtension>> Extensions;
	TArray<FMGCGameFeaturePluginMetadataCustomization*> Extensions;

	/**
	 * Populate the fields of this object from an existing descriptor.
	 */
	// void PopulateFromDescriptor(const FPluginDescriptor& InDescriptor);

	/**
	 * Populate the fields of this object from an existing descriptor.
	 */
	void PopulateFromPlugin(TSharedPtr<IPlugin> InPlugin);

	/**
	 * Copy the metadata fields into a plugin descriptor.
	 */
	void CopyIntoDescriptor(FPluginDescriptor& OutDescriptor) const;
};
