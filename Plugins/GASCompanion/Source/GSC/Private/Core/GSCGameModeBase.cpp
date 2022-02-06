// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Core/GSCGameModeBase.h"


#include "Player/GSCPlayerController.h"
#include "Player/GSCPlayerState.h"
#include "Player/GSCHUD.h"

AGSCGameModeBase::AGSCGameModeBase()
{
	// Ensures default player state is our GAS based one
	PlayerStateClass = AGSCPlayerState::StaticClass();

	// As well as our Base PlayerController
	PlayerControllerClass = AGSCPlayerController::StaticClass();

	// And HUD
	HUDClass = AGSCHUD::StaticClass();
}
