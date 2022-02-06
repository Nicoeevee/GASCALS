// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Characters/GSCCharacterBase.h"
#include "Player/GSCPlayerState.h"

#include "GSCPlayerCharacter.generated.h"

/**
 *
 */
UCLASS()
class GASCOMPANION_API AGSCPlayerCharacter : public AGSCCharacterBase
{
	GENERATED_BODY()

public:

	AGSCPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void PossessedBy(AController* NewController) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Client only
	virtual void OnRep_PlayerState() override;

protected:

	/**
	 * Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race
	 * condition. The PlayerController might call ClientRestart (which calls SetupPlayerInputComponent)
	 * before the PlayerState is repped to the client so the PlayerState would be null in SetupPlayerInputComponent.
	 *
	 * Conversely, the PlayerState might be repped before the PlayerController calls ClientRestart
	 * so the Actor's InputComponent would be null in OnRep_PlayerState.
	 */
	void BindASCInput();
	bool bASCInputBound = false;

	virtual void SetupAbilitySystemComponent(AGSCPlayerState* PS, const bool bShouldSetupAbilityActor);
};
