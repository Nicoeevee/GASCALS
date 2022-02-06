// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerInput.h"

#include "GSCExampleMapManager.generated.h"

class UAttributeSet;

UCLASS()
class GASCOMPANION_API AGSCExampleMapManager : public AActor
{
	GENERATED_BODY()

public:
	/**
	* List of Action Mappings required to run the template / sample.
	*
	* Define here the list of action mappings and their associated key input.
	*/
	UPROPERTY(EditAnywhere, Category="Bindings")
	TArray<FInputActionKeyMapping> ActionMappings;

	/**
	* List of Action Mappings required to run the template / sample.
	*
	* Define here the list of axis mappings and their associated axis input.
	*/
	UPROPERTY(EditAnywhere, Category="Bindings")
	TArray<FInputAxisKeyMapping> AxisMappings;

	/**
	* List of GameplayTags required to run the template / sample.
	*
	* Define here the list of GameplayTags Strings in the form of "A.B.C".
	*
	* When the map loads, the user will be asked if these tags can be created for him.
	*/
	UPROPERTY(EditAnywhere, Category="Required Gameplay Tags")
	TArray<FString> GameplayTags;

	/**
	* List of Attribute Sets required to run the template / sample.
	*
	* Define here the list of AttributeSets that are meant to be configured in GAS Companion Project's Settings.
	*
	* When the map loads, the user will be asked if these AttributeSets can be registered for him.
	*/
	UPROPERTY(EditAnywhere, Category="Required Attribute Sets")
	TArray<TSubclassOf<UAttributeSet>> AttributeSets;
};
