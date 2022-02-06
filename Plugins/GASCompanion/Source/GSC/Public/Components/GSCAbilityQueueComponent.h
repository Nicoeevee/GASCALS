// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GSCAbilityQueueComponent.generated.h"

class UGSCUWDebugAbilityQueue;
class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS(BlueprintType, Blueprintable, ClassGroup=("GAS Companion"), meta=(BlueprintSpawnableComponent) )
class GASCOMPANION_API UGSCAbilityQueueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UGSCAbilityQueueComponent();

	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Components")
	APawn* OwnerPawn;

	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Components")
	UAbilitySystemComponent* OwnerAbilitySystemComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS Companion|Ability Queue System")
	bool bAbilityQueueEnabled = true;

	/** Setup GetOwner to character and sets references for ability system component and the owner itself. */
	void SetupOwner();

    /**
     * Set the bAbilityQueue to true and opens the ability queue system for activation
     */
    void OpenAbilityQueue();

    /**
     * Set the bAbilityQueue to false which prevents the ability queue system to activate
     */
    void CloseAbilityQueue();

    /**
     * Updates the Allowed Abilities for the ability queue system
     */
    void UpdateAllowedAbilitiesForAbilityQueue(TArray<TSubclassOf<UGameplayAbility>> AllowedAbilities);

	/**
	 * Updates the bQueueAllowAllAbilities which prevents the check for queued abilities to be within the QueuedAllowedAbilities array
	 */
	void SetAllowAllAbilitiesForAbilityQueue(bool bAllowAllAbilities);

	bool IsAbilityQueueOpened() const;

	bool IsAllAbilitiesAllowedForAbilityQueue() const;

    const UGameplayAbility* GetCurrentQueuedAbility() const;

    TArray<TSubclassOf<UGameplayAbility>> GetQueuedAllowedAbilities() const;

	/**
	* Called when an ability is ended for the owner actor.
	*
	* The native implementation handles the ability queuing system, and invoke related BP event.
	*/
	void OnAbilityEnded(const UGameplayAbility* InAbility);

	/**
	* Called when an ability failed to activated for the owner actor, passes along the failed ability
	* and a tag explaining why.
	*
	* The native implementation handles the ability queuing system, and invoke related BP event.
	*/
	void OnAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& ReasonTags);

	/**
	 * Returns the Debug Widget from HUD associated with this character (if any)
	 */
	virtual UGSCUWDebugAbilityQueue* GetDebugWidgetFromHUD();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/** Ability Queue System */

	bool bAbilityQueueOpened = false;
	bool bAllowAllAbilitiesForAbilityQueue = false;

	UPROPERTY()
	const UGameplayAbility* QueuedAbility = nullptr;
	TArray<TSubclassOf<UGameplayAbility>> QueuedAllowedAbilities;

	/**
	* Reset all variables involved in the Ability Queue System to their original default values.
	*/
	virtual void ResetAbilityQueueState();

	/**
	* Notify Debug Ability Queue Widget by updating its allowed abilities
	*/
	virtual void UpdateDebugWidgetAllowedAbilities();
};
