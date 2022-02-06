// Copyright 2020 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Editor/GSCEditorInterface.h"

class GASCOMPANIONEDITOR_API FGSCEditorInterfaceImpl : public IGSCEditorInterface
{
public:
	virtual void ResetLaunchPadItems() override;
	virtual void RegisterLaunchPadItem(const FString Title, const FString Subtitle, const FString Description, const FString Image, TArray<FGSCLaunchPadItemAction> Actions) override;
};
