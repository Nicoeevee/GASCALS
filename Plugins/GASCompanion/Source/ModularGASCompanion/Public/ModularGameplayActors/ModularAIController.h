// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "AIController.h"
#include "ModularAIController.generated.h"

/** Minimal class that supports extension by game feature plugins */
UCLASS(Blueprintable)
class MODULARGASCOMPANION_API AModularAIController : public AAIController
{
	GENERATED_BODY()

public:
	//~ Begin AActor Interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface
};
