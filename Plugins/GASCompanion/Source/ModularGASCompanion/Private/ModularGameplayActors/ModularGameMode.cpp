// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "ModularGameplayActors/ModularGameMode.h"
#include "ModularGameplayActors/ModularGameState.h"
#include "ModularGameplayActors/ModularPlayerController.h"
#include "ModularGameplayActors/ModularPlayerState.h"
#include "ModularGameplayActors/ModularPawn.h"

AModularGameModeBase::AModularGameModeBase()
{
	GameStateClass = AModularGameStateBase::StaticClass();
	PlayerControllerClass = AModularPlayerController::StaticClass();
	PlayerStateClass = AModularPlayerState::StaticClass();
	DefaultPawnClass = AModularPawn::StaticClass();
}

AModularGameMode::AModularGameMode()
{
	GameStateClass = AModularGameState::StaticClass();
	PlayerControllerClass = AModularPlayerController::StaticClass();
	PlayerStateClass = AModularPlayerState::StaticClass();
	DefaultPawnClass = AModularPawn::StaticClass();
}
