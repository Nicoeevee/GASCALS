// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GSCAbilitySystemComponent.generated.h"

class UGSCGameplayAbility;

/**
 * Custom Ability System Component implementation
 *
 * @deprecated in favor of MGCAbilitySystemComponent
 */
UCLASS()
class GASCOMPANION_API UGSCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	/** Version of function in AbilitySystemGlobals that returns correct type */
	static UGSCAbilitySystemComponent* GetAbilitySystemComponentFromActor(const AActor* Actor, bool LookForComponent = false);

	/**
	 * Returns a list of currently active ability instances that match the given tags
	 *
	 * @param GameplayTagContainer The Ability Tags to search for
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	TArray<UGSCGameplayAbility*> GetActiveAbilitiesByTags(const FGameplayTagContainer& GameplayTagContainer) const;

	/**
	 * Returns a list of currently active ability instances that match the given class
	 *
	 * @param AbilityToSearch The Gameplay Ability Class to search for
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	TArray<UGSCGameplayAbility*> GetActiveAbilitiesByClass(TSubclassOf<UGameplayAbility> AbilityToSearch) const;

	/**
	 * Attempts to activate a **single** gameplay ability that matches the given tag and DoesAbilitySatisfyTagRequirements().
	 *
	 * It differs from GAS ASC TryActivateAbilitiesByTag which tries to activate *every* ability, whereas this version will pick a
	 * random one and attempt to activate it.
	 *
	 * @param AbilityTags Set of Gameplay Tags to search for
	 * @param ActivatedAbility The Gameplay Ability that was triggered on success
	 * @param bAllowRemoteActivation If true, it will remotely activate local/server abilities, if false it will only try to locally activate abilities.
	 * @return bSuccess Returns true if it thinks it activated, but it may return false positives due to failure later in activation.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	bool ActivateAbilityByTags(const FGameplayTagContainer AbilityTags, UGSCGameplayAbility*& ActivatedAbility, const bool bAllowRemoteActivation = true);

	/**
	 * Attempts to activate the ability that is passed in. This will check costs and requirements before doing so.
	 *
	 * It differs from GAS ASC TryActivateAbilityByClass by also returning the ActivatedAbility as an output parameter,
	 * if it thinks it activated.
	 *
	 * @param AbilityToActivate The Gameplay Ability Class to activate
	 * @param ActivatedAbility The Gameplay Ability that was triggered on success
	 * @param bAllowRemoteActivation If true, it will remotely activate local/server abilities, if false it will only try to locally activate abilities.
	 * @return bSuccess Returns true if it thinks it activated, but it may return false positives due to failure later in activation.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	bool ActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityToActivate,UGSCGameplayAbility*& ActivatedAbility, bool bAllowRemoteActivation = true);

	/** These are Delegate Callbacks register with this ASC to trigger corresponding events on the Owning Character (mainly for ability queuing) **/
	virtual void OnAbilityActivatedCallback(UGameplayAbility* Ability);
	virtual void OnAbilityFailedCallback(const UGameplayAbility* Ability, const FGameplayTagContainer& Tags);
	virtual void OnAbilityEndedCallback(UGameplayAbility* Ability);

	// -------------------------------------
	//	GameplayCue
	//	Can invoke GameplayCues without having to create GameplayEffects
	// -------------------------------------

	/** Invoke a gameplay cue on the owner ability system component */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities", meta=(GameplayTagFilter="GameplayCue"), DisplayName="Execute GameplayCue On Owner", meta=(ScriptName="ExecuteGameplayCue"))
	virtual void K2_ExecuteGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context);

	/** Invoke a gameplay cue on the owner ability system component, with extra parameters */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities", meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Execute GameplayCueWithParams On Owner", meta=(ScriptName = "ExecuteGameplayCueWithParams"))
	virtual void K2_ExecuteGameplayCueWithParams(FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/** Adds a persistent gameplay cue to the owner ability system component. Optionally will remove if ability ends */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities", meta=(GameplayTagFilter="GameplayCue"), DisplayName="Add GameplayCue To Owner", meta=(ScriptName="AddGameplayCue"))
	virtual void K2_AddGameplayCue(FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context);

	/** Adds a persistent gameplay cue to the owner ability system component. Optionally will remove if ability ends */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities", meta = (GameplayTagFilter = "GameplayCue"), DisplayName = "Add GameplayCueWithParams To Owner", meta=(ScriptName = "AddGameplayCueWithParams"))
	virtual void K2_AddGameplayCueWithParams(FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameter);

	/** Removes a persistent gameplay cue from the owner ability system component */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities", meta=(GameplayTagFilter="GameplayCue"), DisplayName="Remove GameplayCue From Owner", meta=(ScriptName="RemoveGameplayCue"))
	virtual void K2_RemoveGameplayCue(FGameplayTag GameplayCueTag);

	/** Removes any GameplayCue added on its own, i.e. not as part of a GameplayEffect. */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities", meta=(GameplayTagFilter="GameplayCue"), DisplayName="Remove All GameplayCues From Owner", meta=(ScriptName="RemoveAllGameplayCues"))
	virtual void K2_RemoveAllGameplayCues();

protected:
	// Make custom GameplayAbility a friend class to be able to call GetAttributeSubobject (for ignore gameplay cost)
	friend class UGSCGameplayAbility;
};
