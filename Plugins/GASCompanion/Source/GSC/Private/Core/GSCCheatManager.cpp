// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Core/GSCCheatManager.h"
#include "Blueprint/UserWidget.h"
#include "Actors/Characters/GSCCharacterBase.h"
#include "Player/GSCHUD.h"
#include "GSCLog.h"

void UGSCCheatManager::GSC_AbilityQueueDebug()
{

	GSC_LOG(Display, TEXT("Toggle Ability Queue System Debug Widget"))

	APlayerController* PC = Cast<APlayerController>(GetOuterAPlayerController());
	if (!PC)
	{
		GSC_LOG(Warning, TEXT("UGSCCheatManager:CreateDebugWidget() No Player Controller"))
		return;
	}

	AGSCHUD* HUD = Cast<AGSCHUD>(PC->GetHUD());
	if (!HUD)
	{
		GSC_LOG(Warning, TEXT("UGSCCheatManager:CreateDebugWidget() Cannot get HUD"))
		return;
	}

	if (HUD->IsAbilityQueueWidgetVisible())
	{
		HUD->HideAbilityQueueWidget();
	}
	else
	{
		HUD->ShowAbilityQueueWidget();
	}
}

void UGSCCheatManager::GSC_ShowDebugAbilitySystem() const
{
	ExecuteConsoleCommand("showdebug AbilitySystem");
}

void UGSCCheatManager::GSC_AbilitySystemNextCategory() const
{
	ExecuteConsoleCommand("AbilitySystem.Debug.NextCategory");
}

void UGSCCheatManager::GSC_OpenComboDebug()
{
	GSC_LOG(Display, TEXT("Toggle Combo Debug Widget"))

	APlayerController* PC = Cast<APlayerController>(GetOuterAPlayerController());
	if (!PC)
	{
		GSC_LOG(Warning, TEXT("UFDCheatManager:GSC_OpenComboDebug() No Player Controller"))
		return;
	}

	AGSCHUD* HUD = Cast<AGSCHUD>(PC->GetHUD());
	if (!HUD)
	{
		GSC_LOG(Warning, TEXT("UFDCheatManager:GSC_OpenComboDebug() Cannot get HUD"))
		return;
	}

	if (HUD->IsComboWidgetVisible())
	{
		HUD->HideComboWidget();
	}
	else
	{
		HUD->ShowComboWidget();
	}
}

void UGSCCheatManager::ExecuteConsoleCommand(const FString Command) const
{
	APlayerController* PC = Cast<APlayerController>(GetOuterAPlayerController());
	if (!PC)
	{
		GSC_VLOG(this, Log, TEXT("UGSCCheatManager:ExecuteConsoleCommand() No Player Controller"))
		return;
	}

	PC->ConsoleCommand(Command, true);
}

