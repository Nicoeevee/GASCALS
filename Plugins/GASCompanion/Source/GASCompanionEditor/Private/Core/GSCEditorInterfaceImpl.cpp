// Copyright 2020 Mickael Daniel. All Rights Reserved.


#include "Core/GSCEditorInterfaceImpl.h"

#include "Editors/LaunchPad/GSCLaunchPadData.h"

void FGSCEditorInterfaceImpl::ResetLaunchPadItems()
{
	GSCLaunchPadData::RegisteredItems.Empty();
}

void FGSCEditorInterfaceImpl::RegisterLaunchPadItem(const FString Title, const FString Subtitle, const FString Description, const FString Image, const TArray<FGSCLaunchPadItemAction> Actions)
{
	GSCLaunchPadData::RegisterItem(Title, Subtitle, Description, Image, Actions);
}
