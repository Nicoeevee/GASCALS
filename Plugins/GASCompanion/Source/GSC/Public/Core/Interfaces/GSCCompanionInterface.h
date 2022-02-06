// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GSCCompanionInterface.generated.h"

class UGSCAbilityQueueComponent;
class UGSCAbilitySystemComponent;
class UGSCComboManagerComponent;
class UGSCCoreComponent;
class UAttributeSet;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGSCCompanionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that expose access to companion components
 *
 * Deprecated in 4.27 (3.0.0)
 */
class GASCOMPANION_API IGSCCompanionInterface
{
	GENERATED_BODY()

public:
	/** Returns the companion core component to use for this actor. */
	virtual UGSCCoreComponent* GetCompanionCoreComponent() const = 0;

	/** Returns the combo manager component to use for this actor. */
	virtual UGSCComboManagerComponent* GetComboManagerComponent() const = 0;

	/** Returns the ability queue component to use for this actor. */
	virtual UGSCAbilityQueueComponent* GetAbilityQueueComponent() const = 0;

	/** Returns the ability queue component to use for this actor. */
	UE_DEPRECATED(4.27, "GetAttributeSets() is deprecated and will be removed in a future release.")
	virtual TArray<const UAttributeSet*> GetAttributeSets() const { return {}; };

	/** Returns the companion core component to use for this actor. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, DisplayName="GetCompanionCoreComponent", Category="GAS Companion")
	UGSCCoreComponent* K2_GetCompanionCoreComponent() const;
};
