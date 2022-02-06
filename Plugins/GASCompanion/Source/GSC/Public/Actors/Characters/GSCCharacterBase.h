// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Core/Interfaces/GSCCompanionInterface.h"
#include "GameFramework/Character.h"
#include "GSCCharacterBase.generated.h"

class UGSCComboManagerComponent;
class UGSCAbilityQueueComponent;
class UGSCCoreComponent;
class UGSCAttributeSetBase;
class UGSCUWDebugAbilityQueue;

UCLASS()
class GASCOMPANION_API AGSCCharacterBase : public ACharacter, public IAbilitySystemInterface, public IGSCCompanionInterface
{
    GENERATED_BODY()

public:
    // Sets default values for this character's properties
    AGSCCharacterBase(const FObjectInitializer& ObjectInitializer);

	/** Companion Core Component */
	UPROPERTY(Category = "GAS Companion|Components", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UGSCCoreComponent* GSCCoreComponent;

	/** Ability Queue Component */
	UPROPERTY(Category = "GAS Companion|Components", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UGSCAbilityQueueComponent* GSCAbilityQueueComponent;

	/** Combo Manager Component */
	UPROPERTY(Category = "GAS Companion|Components", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UGSCComboManagerComponent* GSCComboComponent;

	// AttributeSets created in child classes
	UPROPERTY()
	TArray<const UAttributeSet*> AttributeSets;

	// Keep track of registered attribute sets to avoid adding it twice
	UPROPERTY()
	TArray<FString> RegisteredAttributeSetNames;

    // Implement IAbilitySystemInterface
    virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    // ~ IAbilitySystemInterface

    // Implement IGSCCompanionInterface
    virtual UGSCCoreComponent* GetCompanionCoreComponent() const override;
    virtual UGSCComboManagerComponent* GetComboManagerComponent() const override;
    virtual UGSCAbilityQueueComponent* GetAbilityQueueComponent() const override;
    // ~ IGSCCompanionInterface

    /**
    * Returns the ability system component for this character.
    *
    * Version of function in AbilitySystemGlobals that returns correct type
    */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GAS Companion|Abilities")
    virtual UGSCAbilitySystemComponent* GetASC();

	/**
	 * Returns the Debug Widget from HUD associated with this character (if any)
	 */
	virtual UGSCUWDebugAbilityQueue* GetDebugWidgetFromHUD();

    /**
     * Called when the Pawn is being restarted (usually by being possessed by a Controller)
     *
     * Note that this event will most likely trigger on first spawn also
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|Character")
    void OnRespawn();

    /** Overrides APawn:Restart() to be able to call the event above */
    virtual void Restart() override;

protected:

	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
};
