// Copyright 2021 Mickael Daniel. All Rights Reserved.


#pragma once

#include "CoreMinimal.h"
#include "GSCEditorTypes.generated.h"

class AGSCExampleMapManager;

UENUM()
enum class EGSCLaunchPadActionType : uint8
{
	OpenSample,
	Documentation,
	InstallExamplesPlugin,
	EnableExamplesPlugin
};

UENUM()
enum class EGSCLaunchPadDependencyType : uint8
{
	Module,
	Content
};

USTRUCT()
struct FGSCLaunchPadItemDependency
{
	GENERATED_BODY()


	UPROPERTY()
	FString Name;

	UPROPERTY()
	FString Path;

	UPROPERTY()
	FString Url;

	UPROPERTY()
	EGSCLaunchPadDependencyType Type;

	FGSCLaunchPadItemDependency(): Type()
	{
	}

	FGSCLaunchPadItemDependency(const FString& Name, const FString& Path, const FString& Url, const EGSCLaunchPadDependencyType Type)
		: Name(Name),
		  Path(Path),
		  Url(Url),
		  Type(Type)
	{
	}
};

USTRUCT()
struct FGSCLaunchPadItemAction
{
	GENERATED_BODY()

	UPROPERTY()
	EGSCLaunchPadActionType Type;

	UPROPERTY()
	FString Path;

	UPROPERTY()
	FString Folder;

	UPROPERTY()
	TArray<FGSCLaunchPadItemDependency> Dependencies;

	UPROPERTY()
	TSubclassOf<AGSCExampleMapManager> SampleManager;

	FGSCLaunchPadItemAction(): Type()
	{
	}

	FGSCLaunchPadItemAction(const EGSCLaunchPadActionType Type, const FString& Path, const FString& Folder, const TArray<FGSCLaunchPadItemDependency>& Dependencies, const TSubclassOf<AGSCExampleMapManager>& SampleManager)
		: Type(Type),
		  Path(Path),
		  Folder(Folder),
		  Dependencies(Dependencies),
		  SampleManager(SampleManager)
	{
	}

	FGSCLaunchPadItemAction(const EGSCLaunchPadActionType Type, const FString& Path)
		: Type(Type),
		  Path(Path)
	{
	}
};
