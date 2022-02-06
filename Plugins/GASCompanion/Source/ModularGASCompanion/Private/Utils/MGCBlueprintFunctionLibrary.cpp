// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Utils/MGCBlueprintFunctionLibrary.h"

#include "Abilities/MGCAbilityInputBindingComponent.h"

UMGCAbilityInputBindingComponent* UMGCBlueprintFunctionLibrary::GetAbilityInputBindingComponent(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// Fall back to a component search to better support BP-only actors
	return Actor->FindComponentByClass<UMGCAbilityInputBindingComponent>();
}
