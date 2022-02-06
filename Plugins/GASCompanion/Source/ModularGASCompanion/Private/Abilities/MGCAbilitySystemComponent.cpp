// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Abilities/MGCAbilitySystemComponent.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Abilities/GSCGameplayAbility_MeleeBase.h"
#include "Abilities/MGCAbilityInputBindingComponent.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCComboManagerComponent.h"
#include "Components/GSCCoreComponent.h"
#include "GameFramework/PlayerState.h"
#include "ModularGASCompanionLog.h"

void UMGCAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	AbilityActivatedCallbacks.AddUObject(this, &UMGCAbilitySystemComponent::OnAbilityActivatedCallback);
	AbilityFailedCallbacks.AddUObject(this, &UMGCAbilitySystemComponent::OnAbilityFailedCallback);
	AbilityEndedCallbacks.AddUObject(this, &UMGCAbilitySystemComponent::OnAbilityEndedCallback);

	// Grant startup effects on begin play instead of from within InitAbilityActorInfo to avoid
	// "ticking" periodic effects when BP is first opened
	GrantStartupEffects();
}

void UMGCAbilitySystemComponent::BeginDestroy()
{
	// Reset ...

	// Clear any delegate handled bound previously for this component
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor.IsValid())
	{
		if (UGameInstance* GameInstance = AbilityActorInfo->OwnerActor->GetGameInstance())
		{
			GameInstance->GetOnPawnControllerChanged().RemoveAll(this);
		}
	}

	OnGiveAbilityDelegate.RemoveAll(this);

	// Remove any added attributes
	for (UAttributeSet* AttribSetInstance : AddedAttributes)
	{
		GetSpawnedAttributes_Mutable().Remove(AttribSetInstance);
	}


	// Clear up abilities / bindings
	UMGCAbilityInputBindingComponent* InputComponent = AbilityActorInfo && AbilityActorInfo->AvatarActor.IsValid() ?
		AbilityActorInfo->AvatarActor->FindComponentByClass<UMGCAbilityInputBindingComponent>() :
		nullptr;

	for (const FMGCMappedAbility DefaultAbilityHandle : DefaultAbilityHandles)
	{
		if (InputComponent)
		{
			InputComponent->ClearInputBinding(DefaultAbilityHandle.Handle);
		}

		// Only Clear abilities on authority
		if (IsOwnerActorAuthoritative())
		{
			SetRemoveAbilityOnEnd(DefaultAbilityHandle.Handle);
		}
	}

	// Clear up any bound delegates in Core Component that were registered from InitAbilityActorInfo
	UGSCCoreComponent* CoreComponent = AbilityActorInfo && AbilityActorInfo->AvatarActor.IsValid() ?
		AbilityActorInfo->AvatarActor->FindComponentByClass<UGSCCoreComponent>() :
		nullptr;

	if (CoreComponent)
	{
		CoreComponent->ShutdownAbilitySystemDelegates(this);
	}


	Super::BeginDestroy();
}

void UMGCAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	MGC_LOG(Log, TEXT("UMGCAbilitySystemComponent::InitAbilityActorInfo() - Owner: %s, Avatar: %s"), *InOwnerActor->GetName(), *InAvatarActor->GetName())

	if (AbilityActorInfo)
	{
		if (AbilityActorInfo->AnimInstance == nullptr)
		{
			AbilityActorInfo->AnimInstance = AbilityActorInfo->GetAnimInstance();
		}

		if (UGameInstance* GameInstance = InOwnerActor->GetGameInstance())
		{
			// Sign up for possess / unpossess events so that we can update the cached AbilityActorInfo accordingly
			if (!GameInstance->GetOnPawnControllerChanged().Contains(this, TEXT("OnPawnControllerChanged")))
			{
				GameInstance->GetOnPawnControllerChanged().AddDynamic(this, &UMGCAbilitySystemComponent::OnPawnControllerChanged);
			}
		}
	}

	GrantDefaultAbilitiesAndAttributes(InOwnerActor, InAvatarActor);

	// For PlayerState client pawns, setup and update owner on companion components if pawns have them
	UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(InAvatarActor);
	if (CoreComponent)
	{
		CoreComponent->SetupOwner();
		CoreComponent->RegisterAbilitySystemDelegates(this);
		CoreComponent->SetStartupAbilitiesGranted(true);
	}
}


void UMGCAbilitySystemComponent::AbilityLocalInputPressed(const int32 InputID)
{
	// Consume the input if this InputID is overloaded with GenericConfirm/Cancel and the GenericConfim/Cancel callback is bound
	if (IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}

	if (IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}

	// ---------------------------------------------------------

	ABILITYLIST_SCOPE_LOCK();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputID == InputID && Spec.Ability)
		{
			Spec.InputPressed = true;

			if (Spec.Ability->IsA(UGSCGameplayAbility_MeleeBase::StaticClass()))
			{
				// Ability is a combo ability, try to activate via Combo Component
				if (!IsValid(ComboComponent))
				{
					// Combo Component ref is not set yet, set it once
					ComboComponent = UGSCBlueprintFunctionLibrary::GetComboManagerComponent(GetAvatarActor());
					if (ComboComponent)
					{
						ComboComponent->SetupOwner();
					}
				}

				// Regardless of active or not active, always try to activate the combo. Combo Component will take care of gating activation or queuing next combo
				if (IsValid(ComboComponent))
				{
					// We have a valid combo component, active combo
					ComboComponent->ActivateComboAbility(Spec.Ability->GetClass());
				}
				else
				{
					MGC_LOG(Error, TEXT("UMGCAbilitySystemComponent::AbilityLocalInputPressed - Trying to activate combo without a Combo Manager Component on the Avatar Actor. Make sure to add the component in Blueprint."))
				}


			}
			else
			{
				// Ability is not a combo ability, go through normal workflow
				if (Spec.IsActive())
				{
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					AbilitySpecInputPressed(Spec);

					// Invoke the InputPressed event. This is not replicated here. If someone is listening, they may replicate the InputPressed event to the server.
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, Spec.ActivationInfo.GetActivationPredictionKey());
				}
				else
				{
					TryActivateAbility(Spec.Handle);
				}
			}
		}
	}
}

FGameplayAbilitySpecHandle UMGCAbilitySystemComponent::GrantAbility(const TSubclassOf<UGameplayAbility> Ability, const bool bRemoveAfterActivation)
{
	FGameplayAbilitySpecHandle AbilityHandle;
	if (!IsOwnerActorAuthoritative())
	{
		MGC_LOG(Error, TEXT("UMGCAbilitySystemComponent::GrantAbility Called on non authority"));
		return AbilityHandle;
	}

	if (Ability)
	{
		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.RemoveAfterActivation = bRemoveAfterActivation;

		AbilityHandle = GiveAbility(AbilitySpec);
	}
	return AbilityHandle;
}

void UMGCAbilitySystemComponent::OnAbilityActivatedCallback(UGameplayAbility* Ability)
{
	MGC_LOG(Log, TEXT("UMGCAbilitySystemComponent::OnAbilityActivatedCallback %s"), *Ability->GetName());
	AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		MGC_LOG(Error, TEXT("UMGCAbilitySystemComponent::OnAbilityActivated No OwnerActor for this ability: %s"), *Ability->GetName());
		return;
	}

	const UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	if (CoreComponent)
	{
		CoreComponent->OnAbilityActivated.Broadcast(Ability);
	}
}

void UMGCAbilitySystemComponent::OnAbilityFailedCallback(const UGameplayAbility* Ability, const FGameplayTagContainer& Tags)
{
	MGC_LOG(Log, TEXT("UMGCAbilitySystemComponent::OnAbilityFailedCallback %s"), *Ability->GetName());

	AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		MGC_LOG(Warning, TEXT("UMGCAbilitySystemComponent::OnAbilityFailed No OwnerActor for this ability: %s Tags: %s"), *Ability->GetName(), *Tags.ToString());
		return;
	}

	UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Avatar);
	if (CoreComponent)
	{
		CoreComponent->OnAbilityFailed.Broadcast(Ability, Tags);
	}

	if (AbilityQueueComponent)
	{
		AbilityQueueComponent->OnAbilityFailed(Ability, Tags);
	}
}

void UMGCAbilitySystemComponent::OnAbilityEndedCallback(UGameplayAbility* Ability)
{
	MGC_LOG(Log, TEXT("UMGCAbilitySystemComponent::OnAbilityEndedCallback %s"), *Ability->GetName());
	AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		MGC_LOG(Warning, TEXT("UMGCAbilitySystemComponent::OnAbilityEndedCallback No OwnerActor for this ability: %s"), *Ability->GetName());
		return;
	}

	UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Avatar);
	if (CoreComponent)
	{
		CoreComponent->OnAbilityEnded.Broadcast(Ability);
	}

	if (AbilityQueueComponent)
	{
		AbilityQueueComponent->OnAbilityEnded(Ability);
	}
}

void UMGCAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes(AActor* InOwnerActor, AActor* InAvatarActor)
{
	MGC_LOG(Log, TEXT("UMGCAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes() - Owner: %s, Avatar: %s"), *InOwnerActor->GetName(), *InAvatarActor->GetName())

	// Reset/Remove abilities if we had already added them
	for (UAttributeSet* AttributeSet : AddedAttributes)
	{
		GetSpawnedAttributes_Mutable().Remove(AttributeSet);
	}

	for (const FMGCMappedAbility DefaultAbilityHandle : DefaultAbilityHandles)
	{
		SetRemoveAbilityOnEnd(DefaultAbilityHandle.Handle);
	}

	for (FDelegateHandle InputBindingDelegateHandle : InputBindingDelegateHandles)
	{
		// Clear any delegate handled bound previously for this actor
		OnGiveAbilityDelegate.Remove(InputBindingDelegateHandle);
		InputBindingDelegateHandle.Reset();
	}

	AddedAttributes.Empty(GrantedAttributes.Num());
	DefaultAbilityHandles.Empty(GrantedAbilities.Num());
	InputBindingDelegateHandles.Empty();

	UMGCAbilityInputBindingComponent* InputComponent = IsValid(InAvatarActor) ? InAvatarActor->FindComponentByClass<UMGCAbilityInputBindingComponent>() : nullptr;

	// Startup abilities
	DefaultAbilityHandles.Reserve(GrantedAbilities.Num());
	for (const FMGCAbilityInputMapping GrantedAbility : GrantedAbilities)
	{
		TSubclassOf<UGameplayAbility> Ability = GrantedAbility.Ability;
		UInputAction* InputAction = GrantedAbility.InputAction;
		if (Ability)
		{
			FGameplayAbilitySpecHandle AbilityHandle;
			FGameplayAbilitySpec NewAbilitySpec(Ability);

			// Try to grant the ability first
			if (IsOwnerActorAuthoritative())
			{
				// Only Grant abilities on authority
				MGC_LOG(Log, TEXT("UMGCAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes - Authority, Grant Ability (%s)"), *NewAbilitySpec.Ability->GetClass()->GetName())
				AbilityHandle = GiveAbility(NewAbilitySpec);
				DefaultAbilityHandles.Add(FMGCMappedAbility(AbilityHandle, NewAbilitySpec, InputAction));

				// Handle for server or standalone game, clients need to bind OnGiveAbility
				if (InputComponent && InputAction)
				{
					InputComponent->SetInputBinding(InputAction, GrantedAbility.TriggerEvent, AbilityHandle);
				}
			}

			// Handle clients, we don't grant here but try to get the spec already granted or register delegate to handle input binding
			if (InputComponent && InputAction && !IsOwnerActorAuthoritative())
			{
				FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(Ability);
				if (AbilitySpec)
				{
					InputComponent->SetInputBinding(InputAction, GrantedAbility.TriggerEvent, AbilitySpec->Handle);
				}
				else
				{
					// Register a delegate triggered when ability is granted and available on clients
					FDelegateHandle DelegateHandle = OnGiveAbilityDelegate.AddUObject(this, &UMGCAbilitySystemComponent::HandleOnGiveAbility, InputComponent, InputAction, GrantedAbility.TriggerEvent, NewAbilitySpec);
					InputBindingDelegateHandles.Add(DelegateHandle);
				}
			}
		}
	}

	// Startup attributes
	AddedAttributes.Reserve(GrantedAttributes.Num());
	for (const FMGCAttributeSetDefinition& AttributeSetDefinition : GrantedAttributes)
	{
		if (AttributeSetDefinition.AttributeSet)
		{
			UAttributeSet* AttributeSet = NewObject<UAttributeSet>(InOwnerActor, AttributeSetDefinition.AttributeSet);
			if (AttributeSetDefinition.InitializationData)
			{
				AttributeSet->InitFromMetaDataTable(AttributeSetDefinition.InitializationData);
			}
			AddedAttributes.Add(AttributeSet);
			AddAttributeSetSubobject(AttributeSet);
		}
	}
}

void UMGCAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);
	MGC_LOG(Log, TEXT("UMGCAbilitySystemComponent::OnGiveAbility %s"), *AbilitySpec.Ability->GetName());
	OnGiveAbilityDelegate.Broadcast(AbilitySpec);
}

void UMGCAbilitySystemComponent::GrantStartupEffects()
{
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	// Reset/Remove effects if we had already added them
	for (const FActiveGameplayEffectHandle AddedEffect : AddedEffects)
	{
		RemoveActiveGameplayEffect(AddedEffect);
	}

	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	EffectContext.AddSourceObject(this);

	AddedEffects.Empty(GrantedEffects.Num());

	for (const TSubclassOf<UGameplayEffect> GameplayEffect : GrantedEffects)
	{
		FGameplayEffectSpecHandle NewHandle = MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
		if (NewHandle.IsValid())
		{
			FActiveGameplayEffectHandle EffectHandle = ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), this);
			AddedEffects.Add(EffectHandle);
		}
	}
}

void UMGCAbilitySystemComponent::OnPawnControllerChanged(APawn* Pawn, AController* NewController)
{
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor == Pawn && AbilityActorInfo->PlayerController != NewController)
	{
		AbilityActorInfo->InitFromActor(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get(), this);
	}
}

void UMGCAbilitySystemComponent::HandleOnGiveAbility(FGameplayAbilitySpec& AbilitySpec, UMGCAbilityInputBindingComponent* InputComponent, UInputAction* InputAction, EMGCAbilityTriggerEvent TriggerEvent, FGameplayAbilitySpec NewAbilitySpec)
{
	MGC_LOG(
		Log,
		TEXT("UMGCAbilitySystemComponent::HandleOnGiveAbility: %s, Ability: %s, Input: %s (TriggerEvent: %s) - (InputComponent: %s)"),
		*AbilitySpec.Handle.ToString(),
		*GetNameSafe(AbilitySpec.Ability),
		*GetNameSafe(InputAction),
		*UEnum::GetValueAsName(TriggerEvent).ToString(),
		*GetNameSafe(InputComponent)
	);

	if (InputComponent && InputAction && AbilitySpec.Ability == NewAbilitySpec.Ability)
	{
		InputComponent->SetInputBinding(InputAction, TriggerEvent, AbilitySpec.Handle);
	}
}
