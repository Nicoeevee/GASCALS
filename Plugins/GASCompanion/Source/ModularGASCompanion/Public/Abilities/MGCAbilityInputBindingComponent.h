// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "ModularGASCompanionTypes.h"
#include "Input/MGCPlayerControlsComponent.h"
#include "MGCAbilityInputBindingComponent.generated.h"

USTRUCT()
struct FMGCAbilityInputBinding
{
	GENERATED_BODY()

	int32  InputID = 0;
	uint32 OnPressedHandle = 0;
	uint32 OnReleasedHandle = 0;
	TArray<FGameplayAbilitySpecHandle> BoundAbilitiesStack;

	EMGCAbilityTriggerEvent TriggerEvent = EMGCAbilityTriggerEvent::Started;
};

/**
 * Modular pawn component that hooks up enhanced input to the ability system input logic
 *
 * Extends from MGCPlayerControlsComponent, so if your Pawn is dealing with Abilities use this component instead.
 */
UCLASS(ClassGroup="ModularGASCompanion", meta=(BlueprintSpawnableComponent))
class MODULARGASCOMPANION_API UMGCAbilityInputBindingComponent : public UMGCPlayerControlsComponent
{
	GENERATED_BODY()

public:
	//~ Begin UPlayerControlsComponent interface
	virtual void SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent) override;
	virtual void ReleaseInputComponent(AController* OldController) override;
	//~ End UPlayerControlsComponent interface

	/**
	 * Updates the Ability Input Binding Component registered bindings or create a new one for the passed in Ability Handle.
	 *
	 * @param InputAction The Enhanced InputAction to bind to
	 * @param TriggerEvent The trigger type to use for the ability pressed handle. The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button.
	 * @param AbilityHandle The Gameplay Ability Spec handle to setup binding for (handle returned when granting abilities manually with MGCAbilitySystemComponent)
	 */
	UFUNCTION(BlueprintCallable, Category = "Modular GAS Companion|Abilities")
	void SetInputBinding(UInputAction* InputAction, EMGCAbilityTriggerEvent TriggerEvent, FGameplayAbilitySpecHandle AbilityHandle);

	/** Given a Gameplay Ability Spec handle (handle returned when granting abilities manually with MGCAbilitySystemComponent), clears up and reset the previously registered binding for that ability.  */
	UFUNCTION(BlueprintCallable, Category = "Modular GAS Companion|Abilities")
	void ClearInputBinding(FGameplayAbilitySpecHandle AbilityHandle);

	/** Given an Enhanced Input Action, clears up input binding delegates (On Pressed and Released) and resets any abilities' (that were bound to that action) InputId to none. */
	UFUNCTION(BlueprintCallable, Category = "Modular GAS Companion|Abilities")
	void ClearAbilityBindings(UInputAction* InputAction);

	/**
	 * Given a Gameplay Ability, returns the bound InputAction from mapped abilities (previously bound abilities) that matches the Ability Spec InputID.
	 *
	 * Designed to be called from within a Gameplay Ability event graph, passing self reference for the Gameplay Ability parameter.
	 */
	UFUNCTION(BlueprintPure, Category = "Modular GAS Companion|Abilities")
	UInputAction* GetBoundInputActionForAbility(UPARAM(ref) const UGameplayAbility* Ability);

	/** Internal helper to return InputAction from MappedAbilities that match Ability Spec InputID */
	UInputAction* GetBoundInputActionForAbilitySpec(const FGameplayAbilitySpec* AbilitySpec) const;

private:
	UPROPERTY(transient)
	UAbilitySystemComponent* AbilityComponent;

	UPROPERTY(transient)
	TMap<UInputAction*, FMGCAbilityInputBinding> MappedAbilities;

	void ResetBindings();
	void RunAbilitySystemSetup();

	/**
	 * Runs on press / release, and updates inputs ID for specs based on mapped abilities.
	 *
	 * Needs to run everytime to handle the issue with lost inputID when playing as client after first PIE session if BP containing ASC is compiled in Editor. */
	void UpdateAbilitySystemBindings(UAbilitySystemComponent* AbilitySystemComponent);

	void OnAbilityInputPressed(UInputAction* InputAction);
	void OnAbilityInputReleased(UInputAction* InputAction);

	void RemoveEntry(UInputAction* InputAction);

	FGameplayAbilitySpec* FindAbilitySpec(FGameplayAbilitySpecHandle Handle) const;
	void TryBindAbilityInput(UInputAction* InputAction, FMGCAbilityInputBinding& AbilityInputBinding);
};
