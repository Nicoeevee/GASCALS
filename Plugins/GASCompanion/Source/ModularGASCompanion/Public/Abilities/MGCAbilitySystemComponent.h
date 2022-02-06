// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ModularGASCompanionTypes.h"
#include "MGCAbilitySystemComponent.generated.h"

class UMGCAbilityInputBindingComponent;
class UInputAction;
class UGSCComboManagerComponent;

USTRUCT(BlueprintType)
struct FMGCAbilityInputMapping
{
	GENERATED_BODY()

	/** Type of ability to grant */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	TSubclassOf<UGameplayAbility> Ability;

	/** Input action to bind the ability to, if any (can be left unset) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	UInputAction* InputAction = nullptr;

	/**
	 * The EnhancedInput Trigger Event type to use for the ability activation on pressed handle.
	 *
	 * ---
	 *
	 * The most common trigger types are likely to be Started for actions that happen once, immediately upon pressing a button,
	 * and Triggered for continuous actions that happen every frame while holding an input
	 *
	 * Warning: The Triggered value should only be used for Input Actions that you know only trigger once. If your action
	 * triggered event happens on every tick, this might lead to issues with ability activation (since you'll be
	 * trying to activate abilities every frame). When in doubt, use the default Started value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability, meta=(EditCondition = "InputAction != nullptr", EditConditionHides))
	EMGCAbilityTriggerEvent TriggerEvent = EMGCAbilityTriggerEvent::Started;
};

USTRUCT(BlueprintType)
struct FMGCAttributeSetDefinition
{
	GENERATED_BODY()

	/** Attribute Set to grant */
	UPROPERTY(EditAnywhere, Category=Attributes)
	TSubclassOf<UAttributeSet> AttributeSet;

	/** Data table reference to initialize the attributes with, if any (can be left unset) */
	UPROPERTY(EditAnywhere, Category=Attributes)
	UDataTable* InitializationData = nullptr;
};

USTRUCT()
struct FMGCMappedAbility
{
	GENERATED_BODY()

	FGameplayAbilitySpecHandle Handle;
	FGameplayAbilitySpec Spec;

	UPROPERTY(Transient)
	UInputAction* InputAction;

	FMGCMappedAbility(): InputAction(nullptr)
	{
	}

	FMGCMappedAbility(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilitySpec& Spec, UInputAction* const InputAction)
		: Handle(Handle),
		Spec(Spec),
		InputAction(InputAction)
	{
	}
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnGiveAbility, FGameplayAbilitySpec&);

/**
 * Revamped Ability System Component for 3.0.0
 *
 * This one is meant to be attached in Blueprint (or via a GameFeature),
 * although 4.27 still requires ASC and IAbilitySystemInterface to be implemented in cpp
 *
 * Does not have support for PlayerState for now.
 */
UCLASS(ClassGroup="ModularGASCompanion", meta=(BlueprintSpawnableComponent))
class MODULARGASCOMPANION_API UMGCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	/** List of Gameplay Abilities to grant when the Ability System Component is initialized */
	UPROPERTY(EditDefaultsOnly, Category = "Modular GAS Companion|Abilities")
	TArray<FMGCAbilityInputMapping> GrantedAbilities;

	/** List of Attribute Sets to grant when the Ability System Component is initialized, with optional initialization data */
	UPROPERTY(EditDefaultsOnly, Category = "Modular GAS Companion|Abilities")
	TArray<FMGCAttributeSetDefinition> GrantedAttributes;

	/** List of GameplayEffects to apply when the Ability System Component is initialized (typically on begin play) */
	UPROPERTY(EditDefaultsOnly, Category = "Modular GAS Companion|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** Delegate invoked OnGiveAbility (when an ability is granted and available) */
	FOnGiveAbility OnGiveAbilityDelegate;

	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	//~ End UActorComponent interface

	//~ Begin UObject interface
	virtual void BeginDestroy() override;
	//~ End UObject interface

	//~ Begin UAbilitySystemComponent interface
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	/**
	 * Overrides InputPressed to conditionally ActivateComboAbility or regular TryActivateAbility based on AbilitySpec Ability CDO
	 * (if child of GSCMeleeAbility, will activate combo via combo component)
	 */
	virtual void AbilityLocalInputPressed(int32 InputID) override;
	//~ End UAbilitySystemComponent interface

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Modular GAS Companion|Abilities")
	FGameplayAbilitySpecHandle GrantAbility(TSubclassOf<UGameplayAbility> Ability, bool bRemoveAfterActivation);

	//~ Those are Delegate Callbacks register with this ASC to trigger corresponding events on the Owning Character (mainly for ability queuing)
	virtual void OnAbilityActivatedCallback(UGameplayAbility* Ability);
	virtual void OnAbilityFailedCallback(const UGameplayAbility* Ability, const FGameplayTagContainer& Tags);
	virtual void OnAbilityEndedCallback(UGameplayAbility* Ability);

	/** Called when Ability System Component is initialized */
	virtual void GrantDefaultAbilitiesAndAttributes(AActor* InOwnerActor, AActor* InAvatarActor);

protected:

	UPROPERTY(transient)
	TArray<FMGCMappedAbility> DefaultAbilityHandles;

	// Cached granted AttributeSets
	UPROPERTY(transient)
	TArray<UAttributeSet*> AddedAttributes;

	// Cached applied Startup Effects
	UPROPERTY(transient)
	TArray<FActiveGameplayEffectHandle> AddedEffects;

	// Keep track of OnGiveAbility handles bound to handle input binding on clients
	TArray<FDelegateHandle> InputBindingDelegateHandles;

	// Cached ComboComponent on Character (if it has any)
	UPROPERTY()
	UGSCComboManagerComponent* ComboComponent;

	//~ Begin UAbilitySystemComponent interface
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	//~ End UAbilitySystemComponent interface

	/** Called when Ability System Component is initialized */
	void GrantStartupEffects();

	/** Reinit the cached ability actor info (specifically the player controller) */
	UFUNCTION()
	void OnPawnControllerChanged(APawn* Pawn, AController* NewController);

	/** Handler for AbilitySystem OnGiveAbility delegate. Sets up input binding for clients (not authority) when ability is granted and available for binding. */
	virtual void HandleOnGiveAbility(FGameplayAbilitySpec& AbilitySpec, UMGCAbilityInputBindingComponent* InputComponent, UInputAction* InputAction, EMGCAbilityTriggerEvent TriggerEvent, FGameplayAbilitySpec NewAbilitySpec);
};
