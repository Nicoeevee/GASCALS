// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/GSCUserWidget.h"
#include "GameplayEffectTypes.h"

#include "GSCUWHud.generated.h"

struct FGameplayAbilitySpecHandle;
class UProgressBar;
class UTextBlock;
class UGSCCoreComponent;
class AGSCCharacterBase;

USTRUCT(BlueprintType)
struct FGSCGameplayEffectUIData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	float StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	float TotalDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	float ExpectedEndTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	int32 StackCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	int32 StackLimitCount;

	FGSCGameplayEffectUIData(const float StartTime, const float TotalDuration, const float ExpectedEndTime, const int32 StackCount, const int32 StackLimitCount)
		: StartTime(StartTime),
		  TotalDuration(TotalDuration),
		  ExpectedEndTime(ExpectedEndTime),
		  StackCount(StackCount),
		  StackLimitCount(StackLimitCount)
	{
	}

	FGSCGameplayEffectUIData(): StartTime(0), TotalDuration(0), ExpectedEndTime(0), StackCount(0), StackLimitCount(0)
	{
	}
};


UCLASS(Abstract, Blueprintable)
class GASCOMPANION_API UGSCUWHud : public UGSCUserWidget
{
	GENERATED_BODY()

public:

	/**
	 * Runs initialization logic for this UserWidget related to HUD and interaction with Ability System Component.
	 *
	 * - Setup refs needed for this UserWidget to react to ASC delegates (OwningPawn / OwningAbilitySystemComponent)
	 * - Setup refs for OwnerActor and CompanionCoreComponent (if owner actor has any)
	 * - Init Stats (Health / Stamina / Mana with base values on Owning ASC at the time of invocation)
	 * - Setup ASC delegates to react to various events (optionally can be disabled for older pre 3.0.0 code that used to manage those events with Companion Core Component)
	 *
	 * Called from NativeConstruct, but exposed there if other classes needs to run initialization logic again to update references.
	 */
	virtual void InitializeHUD(bool bRegisterDelegates = true);

	/**
	 * Setup AbilitySystemComponent from OwningPlayerPawn.
	 *
	 * Called from NativeConstruct, but exposed there if other classes needs to update OwningPawn and OwningAbilitySystemComponent if HUD widget was created before ASC was ready
	 */
	virtual void SetupAbilitySystemFromOwningPlayerPawn();

	/** Init widget with attributes from owner character **/
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual void InitFromCharacter();

	/** Updates bound health widgets to reflect the new max health change */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetMaxHealth(float MaxHealth);

	/** Updates bound health widgets to reflect the new health change */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetHealth(float Health);

	/** Updates bound stamina progress bar with the new percent */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetHealthPercentage(float HealthPercentage);

	/** Updates bound stamina widgets to reflect the new max stamina change */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetMaxStamina(float MaxStamina);

	/** Updates bound stamina widgets to reflect the new stamina change */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetStamina(float Stamina);

	/** Updates bound health progress bar with the new percent */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetStaminaPercentage(float StaminaPercentage);

	/** Updates bound mana widgets to reflect the new max mana change */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetMaxMana(float MaxMana);

	/** Updates bound mana widgets to reflect the new mana change */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetMana(float Mana);

	/** Updates bound mana progress bar with the new percent */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|UI")
	virtual void SetManaPercentage(float ManaPercentage);

	// Generic Attribute setter - broadcast change to Blueprint event below
	virtual void HandleAttributeChange(FGameplayAttribute Attribute, float NewValue, float OldValue);

	// called by CCC whenever a gameplay effect is added or removed
	virtual void HandleGameplayEffectStackChange(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, int32 NewStackCount, int32 OldStackCount);

	// called by CCC whenever a gameplay effect time is changed
	virtual void HandleGameplayEffectTimeChange(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, float NewStartTime, float NewDuration);

	// called by CCC whenever a gameplay effect is added
	virtual void HandleGameplayEffectAdded(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle);

	// called by CCC whenever a gameplay effect is removed
	virtual void HandleGameplayEffectRemoved(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle);

	// called by CCC whenever a gameplay tag is added or removed
	virtual void HandleGameplayTagChange(FGameplayTag GameplayTag, int32 NewTagCount);

	// called by CCC whenever an ability with valid cooldown tags is committed (cooldown start)
	virtual void HandleCooldownStart(UGameplayAbility* Ability, const FGameplayTagContainer CooldownTags, float TimeRemaining, float Duration);

	// called by CCC whenever an cooldown tag stack changes, and stack count is 0 (cooldown end)
	virtual void HandleCooldownEnd(UGameplayAbility* Ability, FGameplayTag CooldownTag, float Duration);

	/** Event triggered by Companion Core Component whenever an attribute value is changed */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnAttributeChange(FGameplayAttribute Attribute, float NewValue, float OldValue);

	/** Event triggered by Companion Core Component whenever a gameplay effect is added / removed */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectStackChange(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, int32 NewStackCount, int32 OldStackCount);

	/** Event triggered by Companion Core Component whenever a gameplay effect time is changed (for instance when duration is refreshed) */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectTimeChange(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, float NewStartTime, float NewDuration);

	/** Event triggered by Companion Core Component whenever a gameplay effect is added */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectAdded(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, FGSCGameplayEffectUIData EffectData);

	/** Event triggered by Companion Core Component whenever a gameplay effect is removed */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectRemoved(FGameplayTagContainer AssetTags, FGameplayTagContainer GrantedTags, FActiveGameplayEffectHandle ActiveHandle, FGSCGameplayEffectUIData EffectData);

	/** Event triggered by Companion Core Component whenever a tag is added or removed (but not if just count is increased. Only for 'new' and 'removed' events) */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayTagChange(FGameplayTag GameplayTag, int32 NewTagCount);

	/** Event triggered by Companion Core component when an ability with a valid cooldown is committed and cooldown is applied */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnCooldownStart(UGameplayAbility* Ability, const FGameplayTagContainer CooldownTags, float TimeRemaining, float Duration);

	/** Event triggered by Companion Core Component when a cooldown gameplay tag is removed, meaning cooldown expired */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnCooldownEnd(UGameplayAbility* Ability, FGameplayTag CooldownTag, float Duration);

	/** Returns the current value of an attribute (base value) from owning player Ability System (if it has any). That is, the value of the attribute with no stateful modifiers */
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual float GetAttributeValue(FGameplayAttribute Attribute) const;

protected:
	UPROPERTY()
	UAbilitySystemComponent* OwningAbilitySystemComponent;

	UPROPERTY()
	APawn* OwningPawn;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "GAS Companion|UI")
	UTextBlock* HealthText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "GAS Companion|UI")
	UProgressBar* HealthProgressBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "GAS Companion|UI")
	UTextBlock* StaminaText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "GAS Companion|UI")
	UProgressBar* StaminaProgressBar = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "GAS Companion|UI")
	UTextBlock* ManaText = nullptr;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "GAS Companion|UI")
	UProgressBar* ManaProgressBar = nullptr;

	//~ Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~ End UUserWidget interface

	/** Register listeners for AbilitySystemComponent (Attributes, GameplayEffects / Tags, Cooldowns, ...) */
	void RegisterAbilitySystemDelegates();

	/** Clear all delegates for AbilitySystemComponent bound to this UserWidget */
	void ShutdownAbilitySystemComponentListeners() const;

	/** Triggered by ASC and handle / broadcast Attributes change */
	virtual void OnAttributeChanged(const FOnAttributeChangeData& Data);

	/** Triggered by ASC when GEs are added */
	virtual void OnActiveGameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);

	/** Triggered by ASC when GEs stack count changes */
	virtual void OnActiveGameplayEffectStackChanged(FActiveGameplayEffectHandle ActiveHandle, int32 NewStackCount, int32 PreviousStackCount);

	/** Triggered by ASC when GEs stack count changes */
	virtual void OnActiveGameplayEffectTimeChanged(FActiveGameplayEffectHandle ActiveHandle, float NewStartTime, float NewDuration);

	/** Triggered by ASC when any GEs are removed */
	virtual void OnAnyGameplayEffectRemoved(const FActiveGameplayEffect& EffectRemoved);

	/** Trigger by ASC when any gameplay tag is added or removed (but not if just count is increased. Only for 'new' and 'removed' events) */
	virtual void OnAnyGameplayTagChanged(FGameplayTag GameplayTag, int32 NewCount);

	/** Trigger by ASC when an ability is committed (cost / cooldown are applied)  */
	virtual void OnAbilityCommitted(UGameplayAbility *ActivatedAbility);

	/** Trigger by ASC when a cooldown tag is changed (new or removed)  */
	virtual void OnCooldownGameplayTagChanged(const FGameplayTag GameplayTag, int32 NewCount, FGameplayAbilitySpecHandle AbilitySpecHandle, float Duration);

private:
	/** Array of active GE handle bound to delegates that will be fired when the count for the key tag changes to or away from zero */
	TArray<FActiveGameplayEffectHandle> GameplayEffectAddedHandles;

	/** Array of tags bound to delegates that will be fired when the count for the key tag changes to or away from zero */
	TArray<FGameplayTag> GameplayTagBoundToDelegates;

	static FString GetAttributeFormatString(float BaseValue, float MaxValue);
	static FGSCGameplayEffectUIData GetGameplayEffectUIData(FActiveGameplayEffectHandle ActiveHandle);
};
