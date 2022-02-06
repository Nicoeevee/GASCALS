// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GSCPlayerController.generated.h"

class UGSCUWHud;

/**
 * Player Controller class to setup HUD UserWidget class
 */
UCLASS()
class GASCOMPANION_API AGSCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGSCPlayerController();
	virtual void CreateHUD();

	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|UI")
	UGSCUWHud* UIHUDWidget;

	virtual void OnRep_PlayerState() override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
};
