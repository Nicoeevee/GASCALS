// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Editors/LaunchPad/GSCSampleManagerEditor.h"

#include "GameplayTagsEditorModule.h"
#include "GameplayTagsManager.h"
#include "Components/BillboardComponent.h"
#include "Core/Logging/GASCompanionEditorLog.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "GameFramework/InputSettings.h"
#include "Interfaces/IMainFrameModule.h"
#include "Misc/MessageDialog.h"
#include "UI/Widgets/CloneManager/SMissingGameplayTags.h"
#include "UI/Widgets/CloneManager/SMissingMapping.h"
#include "UI/Widgets/CloneManager/SMissingAttributeSets.h"
#include "UObject/ConstructorHelpers.h"

#define LOCTEXT_NAMESPACE "AGSCCustomInputConfigBinder"

AGSCSampleManagerEditor::AGSCSampleManagerEditor()
{
	bIsEditorOnlyActor = true;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	RootComponent = SceneRoot;

	UBillboardComponent* BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("BillboardComponent");
	BillboardComponent->SetSprite(GetBillboardSprite());
	BillboardComponent->SetupAttachment(RootComponent);
	BillboardComponent->bHiddenInGame = true;

}

TArray<FInputActionKeyMapping> AGSCSampleManagerEditor::GetMissingActionMappings()
{
	TArray<FInputActionKeyMapping> MissingActionMappings;
	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	for (const FInputActionKeyMapping& ActionMapping : ActionMappings)
	{
		if (!InputSettings->GetActionMappings().Contains(ActionMapping))
		{
			MissingActionMappings.Add(ActionMapping);
		}
	}

	return MissingActionMappings;
}

TArray<FInputAxisKeyMapping> AGSCSampleManagerEditor::GetMissingAxisMappings()
{
	TArray<FInputAxisKeyMapping> MissingAxisMappings;
	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	for (const FInputAxisKeyMapping& ActionMapping : AxisMappings)
	{
		if (!InputSettings->GetAxisMappings().Contains(ActionMapping))
		{
			MissingAxisMappings.Add(ActionMapping);
		}
	}

	return MissingAxisMappings;
}

TArray<FString> AGSCSampleManagerEditor::GetMissingGameplayTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	TArray<FString> MissingGameplayTagStrings;
	for (FString GameplayTag : GameplayTags)
	{
		FText ErrorText;
		FString FixedString;
		if (!Manager.IsValidGameplayTagString(GameplayTag, &ErrorText, &FixedString))
		{
			EDITOR_LOG(Error, TEXT("Failed to validate gameplay tag %s: %s, try %s instead!"), *GameplayTag, *ErrorText.ToString(), *FixedString)
			continue;
		}

		if (!Manager.IsDictionaryTag(FName(*GameplayTag)))
		{
			MissingGameplayTagStrings.Add(GameplayTag);
		}
	}

	return MissingGameplayTagStrings;
}

bool AGSCSampleManagerEditor::HasMissingActionMappings()
{
	const TArray<FInputActionKeyMapping> MissingActionMappings = GetMissingActionMappings();
	const TArray<FInputAxisKeyMapping> MissingAxisMappings = GetMissingAxisMappings();

	return MissingActionMappings.Num() > 0 || MissingAxisMappings.Num() > 0;
}

void AGSCSampleManagerEditor::AddMissingActionMapping()
{
	EDITOR_LOG(Log, TEXT("AddMissingActionMapping"));

	if (!HasMissingActionMappings())
	{
		// Already up to date
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("AddMissingActionMapping", "No missing Action mappings.")
		);
		return;
	}

	TSharedRef<SWindow> MissingInputWindow = SNew(SWindow)
		.Title(LOCTEXT("AddMissingActionMappingModalTitle", "Input Binding Registration"))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(770, 455))
		.MinWidth(720)
		.MinHeight(405);

	const TSharedRef<SMissingMapping> MissingMappingContent = SNew(SMissingMapping)
		.MissingActionMappings(GetMissingActionMappings())
		.MissingAxisMappings(GetMissingAxisMappings());

	MissingInputWindow->SetContent(MissingMappingContent);

	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		FSlateApplication::Get().AddModalWindow(MissingInputWindow, MainFrame.GetParentWindow());
	}

	const bool bShouldBindInputs = MissingMappingContent->ShouldBindInputs();

	if (!bShouldBindInputs)
	{
		EDITOR_LOG(Display, TEXT("User rejected binding missing inputs"))
		return;
	}

	if (ActionMappings.Num() > 0 || AxisMappings.Num() > 0)
	{
		UInputSettings* InputSettings = UInputSettings::GetInputSettings();
		for (const FInputActionKeyMapping& ActionMapping : ActionMappings)
		{
			InputSettings->AddActionMapping(ActionMapping, false);
		}

		for (const FInputAxisKeyMapping& AxisMapping : AxisMappings)
		{
			InputSettings->AddAxisMapping(AxisMapping, false);
		}

		InputSettings->ForceRebuildKeymaps();
		InputSettings->SaveKeyMappings();
		InputSettings->UpdateDefaultConfigFile();

		FEditorDelegates::OnActionAxisMappingsChanged.Broadcast();
	}

	FMessageDialog::Open(EAppMsgType::Ok, EAppReturnType::Ok, LOCTEXT("AddMissingActionMappingResult", "Project Action Mappings settings successfully updated."));
}

bool AGSCSampleManagerEditor::HasMissingGameplayTags()
{
	return GetMissingGameplayTags().Num() > 0;
}

bool AGSCSampleManagerEditor::AddMissingGameplayTags()
{
	EDITOR_LOG(Log, TEXT("AddMissingGameplayTags"));

	if (!HasMissingGameplayTags())
	{
		// Already up to date
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("NoMissingGameplayTags", "No missing Gameplay Tags.")
		);

		return true;
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("AddMissingGameplayTagsModalTitle", "Missing Gameplay Tags Registration"))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(770, 455))
		.MinWidth(720)
		.MinHeight(405);

	const TArray<FString> MissingGameplayTags = GetMissingGameplayTags();
	const TSharedRef<SMissingGameplayTags> WindowContent = SNew(SMissingGameplayTags)
		.MissingGameplayTags(MissingGameplayTags);

	Window->SetContent(WindowContent);

	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		FSlateApplication::Get().AddModalWindow(Window, MainFrame.GetParentWindow());
	}

	if (!WindowContent->ShouldRegisterGameplayTags())
	{
		EDITOR_LOG(Display, TEXT("User rejected registration of missing gameplay tags"))
		return false;
	}

	IGameplayTagsEditorModule& GameplayTagsEditorModule = IGameplayTagsEditorModule::Get();
	for (FString GameplayTag : MissingGameplayTags)
	{
		EDITOR_LOG(Display, TEXT("Register missing Gameplay Tag %s"), *GameplayTag)
		GameplayTagsEditorModule.AddNewGameplayTagToINI(GameplayTag, "GASCompanion: Example map required gameplay tag registered by LaunchPad");
	}

	FMessageDialog::Open(
		EAppMsgType::Ok,
		EAppReturnType::Ok,
		LOCTEXT("AddMissingGameplayTagsModalTitleResult", "Project Gameplay Tags settings successfully updated.")
	);

	return true;
}

bool AGSCSampleManagerEditor::HasMissingAttributeSets()
{
	const TArray<TSubclassOf<UAttributeSet>> MissingAttributeSets = GetMissingAttributeSets();
	return MissingAttributeSets.Num() > 0;
}

bool AGSCSampleManagerEditor::AddMissingAttributeSets()
{

	EDITOR_LOG(Log, TEXT("AddMissingAttributeSets"));

	if (!HasMissingAttributeSets())
	{
		// Already up to date
		FMessageDialog::Open(
			EAppMsgType::Ok,
			EAppReturnType::Ok,
			LOCTEXT("NoMissingAttributeSets", "No missing AttributeSets.")
		);

		return true;
	}

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(LOCTEXT("AddMissingAttributeSetsModalTitle", "Missing AttributeSets Registration"))
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SizingRule(ESizingRule::UserSized)
		.ClientSize(FVector2D(770, 455))
		.MinWidth(720)
		.MinHeight(405);

	const TArray<TSubclassOf<UAttributeSet>> MissingAttributeSets = GetMissingAttributeSets();
	const TSharedRef<SMissingAttributeSets> WindowContent = SNew(SMissingAttributeSets)
		.MissingAttributeSets(MissingAttributeSets);

	Window->SetContent(WindowContent);

	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
		FSlateApplication::Get().AddModalWindow(Window, MainFrame.GetParentWindow());
	}

	if (!WindowContent->ShouldRegisterAttributeSets())
	{
		EDITOR_LOG(Display, TEXT("User rejected registration of missing AttributeSets"))
		return false;
	}

	UGSCDeveloperSettings* Settings = GetMutableDefault<UGSCDeveloperSettings>();
	Settings->PlayerStateAttributeSets.Append(MissingAttributeSets);
	Settings->SaveConfig();

	FMessageDialog::Open(
		EAppMsgType::Ok,
		EAppReturnType::Ok,
		LOCTEXT("AddMissingAttributeSetsModalTitleResult", "Project AttributeSets settings successfully updated.")
	);

	return true;
}

TArray<TSubclassOf<UAttributeSet>> AGSCSampleManagerEditor::GetMissingAttributeSets()
{
	const UGSCDeveloperSettings* Settings = GetDefault<UGSCDeveloperSettings>();
	const TArray<TSubclassOf<UGSCAttributeSetBase>> PlayerStateAttributeSets = Settings->PlayerStateAttributeSets;

	TArray<TSubclassOf<UAttributeSet>> MissingAttributeSets;
	for (TSubclassOf<UAttributeSet> AttributeSetClass : AttributeSets)
	{
		if (!PlayerStateAttributeSets.Contains(AttributeSetClass))
		{
			MissingAttributeSets.Add(AttributeSetClass);
		}
	}

	return MissingAttributeSets;
}

UTexture2D* AGSCSampleManagerEditor::GetBillboardSprite()
{
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UTexture2D> SpriteTexture;
		FName ID_Misc;
		FText NAME_Misc;
		FConstructorStatics()
			: SpriteTexture(TEXT("/GASCompanion/EditorResources/S_GSC_Icon"))
			  , ID_Misc(TEXT("Misc"))
			  , NAME_Misc(NSLOCTEXT("SpriteCategory", "Misc", "Misc" ))
		{}
	};

	static FConstructorStatics ConstructorStatics;
	return ConstructorStatics.SpriteTexture.Object;
}

#undef LOCTEXT_NAMESPACE
