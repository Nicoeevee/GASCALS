// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/PlayerState.h"
#include "UI/GSCUWHud.h"

#include "GSCPlayerState.generated.h"

/**
 *
 */
UCLASS()
class GASCOMPANION_API AGSCPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AGSCPlayerState();

	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	TArray<const UAttributeSet*> AttributeSets;

	// Keep track of registered attribute sets to avoid adding it twice
	UPROPERTY()
	TArray<FString> RegisteredAttributeSetNames;

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;


protected:
	void SetupAttributeSets();

};
