// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Player/GSCPlayerController.h"
#include "Core/GSCCheatManager.h"
#include "GSCLog.h"
#include "Actors/Characters/GSCCharacterBase.h"
#include "Player/GSCPlayerState.h"
#include "Player/GSCHUD.h"
#include "UI/GSCUWHud.h"

AGSCPlayerController::AGSCPlayerController()
{
	CheatClass = UGSCCheatManager::StaticClass();
}

void AGSCPlayerController::CreateHUD()
{
	// Only create HUD for local player
	if (!IsLocalPlayerController())
	{
		return;
	}

	// Only create once
	if (UIHUDWidget)
	{
		// But (re)set owner and init widget from its attributes
		UIHUDWidget->InitializeHUD();
		return;
	}

	AGSCHUD* HUD = Cast<AGSCHUD>(GetHUD());
	if (!HUD)
	{
		GSC_LOG(Warning, TEXT("AGSCPlayerController::CreateHUD() Couldn't get or cast to AGSCHUD"))
		return;
	}

	UIHUDWidget = Cast<UGSCUWHud>(HUD->CreateHUDWidget());
	if (!UIHUDWidget)
	{
		GSC_LOG(Warning, TEXT("AGSCPlayerController::CreateHUD() Couldn't cast HUD Widget to UGSCUWHud"))
		return;
	}

	// Actually add to viewport now
	UIHUDWidget->AddToViewport();
}

void AGSCPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	CreateHUD();
}

void AGSCPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AGSCPlayerState* PS = GetPlayerState<AGSCPlayerState>();
	if (PS)
	{
		// Init ASC with PS (Owner) and our new Pawn (AvatarActor)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, InPawn);
	}
}
