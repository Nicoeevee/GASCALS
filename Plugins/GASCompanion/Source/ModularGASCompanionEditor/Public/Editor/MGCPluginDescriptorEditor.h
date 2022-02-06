// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"

class UMGCPluginMetadaObject;
class IPlugin;
class SWindow;
class SWidget;

/**
 * Creates a window to edit a .uplugin file
 */
class FMGCPluginDescriptorEditor
{
public:
	static void OpenEditorWindow(TSharedRef<IPlugin> PluginToEdit, TSharedPtr<SWidget> ParentWidget, FSimpleDelegate OnEditCommitted);

private:
	static FReply OnEditPluginFinished(UMGCPluginMetadaObject* MetadataObject, TSharedPtr<IPlugin> Plugin, FSimpleDelegate OnEditCommitted, TWeakPtr<SWindow> WeakWindow);
};

