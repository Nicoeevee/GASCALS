// Copyright 2020 Mickael Daniel. All Rights Reserved.


#include "Core/Editor/GSCEditorInterface.h"

TSharedPtr<IGSCEditorInterface> IGSCEditorInterface::Instance;

// Add default functionality here for any IGSCEditorInterface functions that are not pure virtual.
TSharedPtr<IGSCEditorInterface> IGSCEditorInterface::Get()
{
	return Instance;
}

void IGSCEditorInterface::Set(const TSharedPtr<IGSCEditorInterface> InInstance)
{
	Instance = InInstance;
}
