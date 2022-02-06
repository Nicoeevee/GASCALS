// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Player/GSCHUD.h"
#include "Blueprint/UserWidget.h"
#include "UI/GSCUWDebugAbilityQueue.h"
#include "UI/GSCUWDebugComboWidget.h"
#include "GSCLog.h"
#include "UI/GSCUWHud.h"

AGSCHUD::AGSCHUD(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InitDefaultWidgetClasses();
}

UUserWidget* AGSCHUD::CreateHUDWidget()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		GSC_LOG(Error, TEXT("AGSCHUD:CreateHUDWidget() Failed to get owner Player Controller"));
		return nullptr;
	}

	if (!HUDWidgetClass)
	{
        GSC_LOG(Error, TEXT("AGSCHUD:CreateHUDWidget() Failed to load HUDWidgetClass, not creating widget. Please configure it in your HUD Class or Blueprint"));
		return nullptr;
	}

	UUserWidget* Widget = CreateWidget(PC, HUDWidgetClass);
	HUDWidget = Widget;

	// APawn* OwningPawn = GetOwningPawn();
	// UGSCUWHud* CompanionHUD = Cast<UGSCUWHud>(Widget);
	// if (IsValid(OwningPawn) && CompanionHUD)
	// {
	// 	CompanionHUD->SetOwnerActor(OwningPawn);
	// 	CompanionHUD->InitFromCharacter();
	// }

	return Widget;
}

void AGSCHUD::ShowHUDWidget() const
{
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}
}

void AGSCHUD::HideHUDWidget()
{
	if (HUDWidget)
	{
		HUDWidget->RemoveFromViewport();
	}
}

UUserWidget* AGSCHUD::GetHUDWidget() const
{
	return HUDWidget;
}

UUserWidget* AGSCHUD::CreateAbilityQueueWidget()
{
	GSC_LOG(Log, TEXT("AGSCHUD::CreateAbilityQueueWidget()"))
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		GSC_LOG(Log, TEXT("AGSCHUD::CreateAbilityQueueWidget() No Player Controller"))
		return nullptr;
	}

	if (!AbilityQueueWidgetClass)
	{
        GSC_LOG(Warning, TEXT("AGSCHUD:CreateAbilityQueueWidget() Failed to load AbilityQueueWidgetClass, not creating widget. Please configure it in your HUD Class or Blueprint"));
	}

	UGSCUWDebugAbilityQueue* Widget = Cast<UGSCUWDebugAbilityQueue>(CreateWidget(PC, AbilityQueueWidgetClass));
	if (!Widget)
	{
		return nullptr;
	}

	Widget->SetOwnerActor(GetOwningPawn());

	DebugWidget = Widget;

	GSC_LOG(Log, TEXT("AGSCHUD::CreateAbilityQueueWidget() Created Widget %s"), *Widget->GetName())
	return Widget;
}

void AGSCHUD::ShowAbilityQueueWidget()
{
	if (!DebugWidget)
	{
		CreateAbilityQueueWidget();
	}

	if (DebugWidget)
	{
		DebugWidget->AddToViewport();
	}
}

void AGSCHUD::HideAbilityQueueWidget()
{
	if (DebugWidget)
	{
		DebugWidget->RemoveFromViewport();
		DebugWidget = nullptr;
	}
}

UUserWidget* AGSCHUD::GetAbilityQueueWidget()
{
	return DebugWidget;
}

bool AGSCHUD::IsAbilityQueueWidgetVisible()
{
	if (!DebugWidget)
	{
		return false;
	}

	return DebugWidget->IsVisible();
}

UUserWidget* AGSCHUD::CreateComboWidget()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return nullptr;
	}

	if (!ComboWidgetClass)
	{
		GSC_LOG(Warning, TEXT("AGSCHUD:CreateHUDWidget() Failed to load ComboDebugWidgetClass, not creating widget. Please configure it in your HUD Class or Blueprint"));
		return nullptr;
	}

	// UUserWidget* Widget = CreateWidget(PC, ComboDebugWidgetClass);
	UGSCUWDebugComboWidget* Widget = Cast<UGSCUWDebugComboWidget>(CreateWidget(PC, ComboWidgetClass));
	if (!Widget)
	{
		return nullptr;
	}

	Widget->SetOwnerActor(GetOwningPawn());
	ComboDebugWidget = Widget;


	return Widget;
}

void AGSCHUD::ShowComboWidget()
{
	if (!ComboDebugWidget)
	{
		CreateComboWidget();
	}

	if (ComboDebugWidget)
	{
		ComboDebugWidget->AddToViewport();
	}
}

void AGSCHUD::HideComboWidget()
{
	if (ComboDebugWidget)
	{
		ComboDebugWidget->RemoveFromViewport();
		ComboDebugWidget = nullptr;
	}
}

void AGSCHUD::ToggleComboWidget()
{
	if (IsComboWidgetVisible())
	{
		HideComboWidget();
	}
	else
	{
		ShowComboWidget();
	}
}

UUserWidget* AGSCHUD::GetComboWidget()
{
	return ComboDebugWidget;
}

bool AGSCHUD::IsComboWidgetVisible()
{
	if (!ComboDebugWidget)
	{
		return false;
	}

	return ComboDebugWidget->IsVisible();
}


void AGSCHUD::InitDefaultWidgetClasses()
{
	GSC_LOG(Log, TEXT("AGSCHUD:InitDefaultWidgetClasses()"));

	HUDWidgetClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/GASCompanion/UI/WB_HUD_Minimalist.WB_HUD_Minimalist_C"));
	if (!HUDWidgetClass)
	{
		GSC_LOG(Error, TEXT("AGSCHUD:InitDefaultWidgetClasses() Failed to load HUDWidgetClass. If it was moved, please update the reference location in C++."));
	}

	AbilityQueueWidgetClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/GASCompanion/UI/WB_DebugAbilityQueue.WB_DebugAbilityQueue_C"));
	if (!AbilityQueueWidgetClass)
	{
		GSC_LOG(Error, TEXT("AGSCHUD:InitDefaultWidgetClasses() Failed to load AbilityQueueWidgetClass. If it was moved, please update the reference location in C++."));
	}

	ComboWidgetClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/GASCompanion/UI/WB_Debug_Combo.WB_Debug_Combo_C"));
	if (!AbilityQueueWidgetClass)
	{
		GSC_LOG(Error, TEXT("AGSCHUD:InitDefaultWidgetClasses() Failed to load ComboWidgetClass. If it was moved, please update the reference location in C++."));
	}
}
