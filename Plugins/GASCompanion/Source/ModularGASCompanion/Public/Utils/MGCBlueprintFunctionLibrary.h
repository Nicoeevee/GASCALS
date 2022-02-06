// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MGCBlueprintFunctionLibrary.generated.h"

class UMGCAbilityInputBindingComponent;

/**
* Ability specific blueprint library
*
* Note: Specific to ModularGASCompanion module to not add cross module dependencies between GASCompanion and ModularGASCompanion, but this blueprint lib may get merged into the main one
* if and when ModularGASCompanion runtime module is moved to main module for ue5 release.
*/
UCLASS()
class MODULARGASCOMPANION_API UMGCBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	* Tries to find an ability input binding component on the actor
	*/
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Components")
	static UMGCAbilityInputBindingComponent* GetAbilityInputBindingComponent(const AActor* Actor);
};
