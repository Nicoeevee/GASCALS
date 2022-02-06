// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "GameFramework/GameMode.h"
#include "ModularGameMode.generated.h"

/** Pair this with a ModularGameStateBase */
UCLASS(Blueprintable)
class MODULARGASCOMPANION_API AModularGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AModularGameModeBase();
};

/** Pair this with a ModularGameState */
UCLASS(Blueprintable)
class MODULARGASCOMPANION_API AModularGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AModularGameMode();
};
