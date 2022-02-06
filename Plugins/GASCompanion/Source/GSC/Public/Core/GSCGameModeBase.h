// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GSCGameModeBase.generated.h"

/**
 * Game Mode to use or subclass that sets Player State to GSCPlayerState class
 */
UCLASS()
class GASCOMPANION_API AGSCGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGSCGameModeBase();
};
