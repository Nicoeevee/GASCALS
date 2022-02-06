// Copyright 2020 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSCEditorTypes.h"


/**
 * Provides a way for runtime module to work with the editor module without adding a reference to it.
 */
class GASCOMPANION_API IGSCEditorInterface
{
public:
	virtual ~IGSCEditorInterface() {};

	static TSharedPtr<IGSCEditorInterface> Get();
	static void Set(TSharedPtr<IGSCEditorInterface> InInstance);

	virtual void ResetLaunchPadItems() = 0;
	virtual void RegisterLaunchPadItem(const FString Title, const FString Subtitle, const FString Description, const FString Image, TArray<FGSCLaunchPadItemAction> Actions) = 0;

private:
	static TSharedPtr<IGSCEditorInterface> Instance;
};
