// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Characters/GSCCharacterBase.h"
#include "GSCAICharacter.generated.h"

/**
 * Specialized AI character class, with additional features like UI StatusBar, AbilitySystem initialization on Character etc.
 *
 * It differs from PlayerCharacter mainly by not relying on PlayerState for ASC
 */
UCLASS()
class GASCOMPANION_API AGSCAICharacter : public AGSCCharacterBase
{
	GENERATED_BODY()

public:
	AGSCAICharacter(const FObjectInitializer& ObjectInitializer);

	// Overrides this to create AttributeSet instead of doing it in constructor
	// https://answers.unrealengine.com/questions/944127/duplicating-a-pawn-that-use-gameplayablilities-plu.html
	virtual void PreInitializeComponents() override;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Create and register attribute sets */
	void SetupAttributeSets();
};
