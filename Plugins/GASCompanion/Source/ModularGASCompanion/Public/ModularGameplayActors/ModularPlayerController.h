// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "GameFramework/PlayerController.h"
#include "ModularPlayerController.generated.h"

/** Minimal class that supports extension by game feature plugins */
UCLASS(Blueprintable)
class MODULARGASCOMPANION_API AModularPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	//~ Begin AActor interface
	virtual void PreInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor interface

	//~ Begin APlayerController interface
	virtual void ReceivedPlayer() override;
	virtual void PlayerTick(float DeltaTime) override;
	//~ End APlayerController interface

protected:

	//~ Begin APlayerController interface
	virtual void OnPossess(APawn* InPawn) override;
	//~ End APlayerController interface
};
