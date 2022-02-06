// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Abilities/MGCAbilityInputBindingComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ModularGASCompanionLog.h"

namespace MGCAbilityInputBindingComponent_Impl
{
	constexpr int32 InvalidInputID = 0;
	int32 IncrementingInputID = InvalidInputID;

	static int32 GetNextInputID()
	{
		return ++IncrementingInputID;
	}
}

void UMGCAbilityInputBindingComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
	ResetBindings();

	for (const auto& Ability : MappedAbilities)
	{
		UInputAction* InputAction = Ability.Key;
		const FMGCAbilityInputBinding AbilityInputBinding = Ability.Value;

		// Convert out internal enum to the real Input Trigger Event for Enhanced Input
		const ETriggerEvent TriggerEvent = AbilityInputBinding.TriggerEvent == EMGCAbilityTriggerEvent::Started ? ETriggerEvent::Started
			: AbilityInputBinding.TriggerEvent == EMGCAbilityTriggerEvent::Triggered ? ETriggerEvent::Triggered
			: ETriggerEvent::Started;

		MGC_LOG(Log, TEXT("UMGCAbilityInputBindingComponent::SetupPlayerControls ... setup pressed handle for %s with %s"), *GetNameSafe(InputAction), *UEnum::GetValueAsName(TriggerEvent).ToString())

		// Pressed event
		InputComponent->BindAction(InputAction, TriggerEvent, this, &UMGCAbilityInputBindingComponent::OnAbilityInputPressed, InputAction);

		// Released event
		InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UMGCAbilityInputBindingComponent::OnAbilityInputReleased, InputAction);
	}

	RunAbilitySystemSetup();
}

void UMGCAbilityInputBindingComponent::ReleaseInputComponent(AController* OldController)
{
	ResetBindings();

	Super::ReleaseInputComponent();
}

void UMGCAbilityInputBindingComponent::SetInputBinding(UInputAction* InputAction, EMGCAbilityTriggerEvent TriggerEvent, FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace MGCAbilityInputBindingComponent_Impl;

	FGameplayAbilitySpec* BindingAbility = FindAbilitySpec(AbilityHandle);

	FMGCAbilityInputBinding* AbilityInputBinding = MappedAbilities.Find(InputAction);
	if (AbilityInputBinding)
	{
		FGameplayAbilitySpec* OldBoundAbility = FindAbilitySpec(AbilityInputBinding->BoundAbilitiesStack.Top());
		if (OldBoundAbility && OldBoundAbility->InputID == AbilityInputBinding->InputID)
		{
			OldBoundAbility->InputID = InvalidInputID;
		}
	}
	else
	{
		AbilityInputBinding = &MappedAbilities.Add(InputAction);
		AbilityInputBinding->InputID = GetNextInputID();
		AbilityInputBinding->TriggerEvent = TriggerEvent;
	}

	if (BindingAbility)
	{
		BindingAbility->InputID = AbilityInputBinding->InputID;
	}

	AbilityInputBinding->BoundAbilitiesStack.Push(AbilityHandle);
	TryBindAbilityInput(InputAction, *AbilityInputBinding);
}

void UMGCAbilityInputBindingComponent::ClearInputBinding(const FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace MGCAbilityInputBindingComponent_Impl;

	FGameplayAbilitySpec* FoundAbility = FindAbilitySpec(AbilityHandle);
	if (!FoundAbility)
	{
		return;
	}

	// Find the mapping for this ability
	auto MappedIterator = MappedAbilities.CreateIterator();
	while (MappedIterator)
	{
		if (MappedIterator.Value().InputID == FoundAbility->InputID)
		{
			break;
		}

		++MappedIterator;
	}

	if (MappedIterator)
	{
		FMGCAbilityInputBinding& AbilityInputBinding = MappedIterator.Value();

		if (AbilityInputBinding.BoundAbilitiesStack.Remove(AbilityHandle) > 0)
		{
			if (AbilityInputBinding.BoundAbilitiesStack.Num() > 0)
			{
				FGameplayAbilitySpec* StackedAbility = FindAbilitySpec(AbilityInputBinding.BoundAbilitiesStack.Top());
				if (StackedAbility && StackedAbility->InputID == 0)
				{
					StackedAbility->InputID = AbilityInputBinding.InputID;
				}
			}
			else
			{
				// NOTE: This will invalidate the `AbilityInputBinding` ref above
				RemoveEntry(MappedIterator.Key());
			}
			// DO NOT act on `AbilityInputBinding` after here (it could have been removed)


			FoundAbility->InputID = InvalidInputID;
		}
	}
}

void UMGCAbilityInputBindingComponent::ClearAbilityBindings(UInputAction* InputAction)
{
	RemoveEntry(InputAction);
}

UInputAction* UMGCAbilityInputBindingComponent::GetBoundInputActionForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
	{
		MGC_LOG(Error, TEXT("GetBoundInputActionForAbility - Passed in ability is null."))
		return nullptr;
	}

	UAbilitySystemComponent* AbilitySystemComponent = Ability->GetAbilitySystemComponentFromActorInfo();
	if (!AbilitySystemComponent)
	{
		MGC_LOG(Error, TEXT("GetBoundInputActionForAbility - Trying to find input action for %s but AbilitySystemComponent from ActorInfo is null. (Ability's)"), *GetNameSafe(Ability))
		return nullptr;
	}

	// Ensure and update inputs ID for specs based on mapped abilities.
	UpdateAbilitySystemBindings(AbilitySystemComponent);

	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Ability->GetClass());
	if (!AbilitySpec)
	{
		MGC_LOG(Error, TEXT("GetBoundInputActionForAbility - AbilitySystemComponent could not return Ability Spec for %s."), *GetNameSafe(Ability->GetClass()))
		return nullptr;
	}

	return GetBoundInputActionForAbilitySpec(AbilitySpec);
}


UInputAction* UMGCAbilityInputBindingComponent::GetBoundInputActionForAbilitySpec(const FGameplayAbilitySpec* AbilitySpec) const
{
	check(AbilitySpec);

	UInputAction* FoundInputAction = nullptr;
	for (const auto MappedAbility : MappedAbilities)
	{
		const FMGCAbilityInputBinding AbilityInputBinding = MappedAbility.Value;
		if (AbilityInputBinding.InputID == AbilitySpec->InputID)
		{
			FoundInputAction = MappedAbility.Key;
			break;
		}
	}

	return FoundInputAction;
}

void UMGCAbilityInputBindingComponent::ResetBindings()
{
	for (auto& InputBinding : MappedAbilities)
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(InputBinding.Value.OnPressedHandle);
			InputComponent->RemoveBindingByHandle(InputBinding.Value.OnReleasedHandle);
		}

		if (AbilityComponent)
		{
			const int32 ExpectedInputID = InputBinding.Value.InputID;

			for (const FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility && FoundAbility->InputID == ExpectedInputID)
				{
					FoundAbility->InputID = MGCAbilityInputBindingComponent_Impl::InvalidInputID;
				}
			}
		}
	}

	AbilityComponent = nullptr;
}

void UMGCAbilityInputBindingComponent::RunAbilitySystemSetup()
{
	AActor* MyOwner = GetOwner();
	check(MyOwner);

	AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyOwner);
	if (AbilityComponent)
	{
		for (auto& InputBinding : MappedAbilities)
		{
			const int32 NewInputID = MGCAbilityInputBindingComponent_Impl::GetNextInputID();
			InputBinding.Value.InputID = NewInputID;

			for (const FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility != nullptr)
				{
					FoundAbility->InputID = NewInputID;
				}
			}
		}
	}
}

void UMGCAbilityInputBindingComponent::UpdateAbilitySystemBindings(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		MGC_LOG(Error, TEXT("UMGCAbilityInputBindingComponent::UpdateAbilitySystemBindings - Passed in ASC is invalid"))
		return;
	}

	for (auto& InputBinding : MappedAbilities)
	{
		const int32 InputID = InputBinding.Value.InputID;
		if (InputID <= 0)
		{
			continue;
		}

		for (const FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
		{
			FGameplayAbilitySpec* FoundAbility = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle);
			if (FoundAbility != nullptr)
			{
				FoundAbility->InputID = InputID;
			}
		}
	}
}

void UMGCAbilityInputBindingComponent::OnAbilityInputPressed(UInputAction* InputAction)
{
	// The AbilitySystemComponent may not have been valid when we first bound input... try again.
	if (AbilityComponent)
	{
		UpdateAbilitySystemBindings(AbilityComponent);
	}
	else
	{
		RunAbilitySystemSetup();
	}

	if (AbilityComponent)
	{
		using namespace MGCAbilityInputBindingComponent_Impl;

		FMGCAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputPressed(FoundBinding->InputID);
		}
	}
}

void UMGCAbilityInputBindingComponent::OnAbilityInputReleased(UInputAction* InputAction)
{
	// The AbilitySystemComponent may need to have specs inputID updated here for clients... try again.
	UpdateAbilitySystemBindings(AbilityComponent);

	if (AbilityComponent)
	{
		using namespace MGCAbilityInputBindingComponent_Impl;

		FMGCAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputReleased(FoundBinding->InputID);
		}
	}
}

void UMGCAbilityInputBindingComponent::RemoveEntry(UInputAction* InputAction)
{
	if (FMGCAbilityInputBinding* Bindings = MappedAbilities.Find(InputAction))
	{
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(Bindings->OnPressedHandle);
			InputComponent->RemoveBindingByHandle(Bindings->OnReleasedHandle);
		}

		for (const FGameplayAbilitySpecHandle AbilityHandle : Bindings->BoundAbilitiesStack)
		{
			using namespace MGCAbilityInputBindingComponent_Impl;

			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilityHandle);
			if (AbilitySpec && AbilitySpec->InputID == Bindings->InputID)
			{
				AbilitySpec->InputID = InvalidInputID;
			}
		}

		MappedAbilities.Remove(InputAction);
	}
}

FGameplayAbilitySpec* UMGCAbilityInputBindingComponent::FindAbilitySpec(const FGameplayAbilitySpecHandle Handle) const
{
	FGameplayAbilitySpec* FoundAbility = nullptr;
	if (AbilityComponent)
	{
		FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(Handle);
	}
	return FoundAbility;
}

void UMGCAbilityInputBindingComponent::TryBindAbilityInput(UInputAction* InputAction, FMGCAbilityInputBinding& AbilityInputBinding)
{
	if (InputComponent)
	{
		// Pressed event
		if (AbilityInputBinding.OnPressedHandle == 0)
		{
			// Convert out internal enum to the real Input Trigger Event for Enhanced Input
			const ETriggerEvent TriggerEvent = AbilityInputBinding.TriggerEvent == EMGCAbilityTriggerEvent::Started ? ETriggerEvent::Started
				: AbilityInputBinding.TriggerEvent == EMGCAbilityTriggerEvent::Triggered ? ETriggerEvent::Triggered
				: ETriggerEvent::Started;

			MGC_LOG(Log, TEXT("TryBindAbilityInput ... setup pressed handle for %s with %s"), *GetNameSafe(InputAction), *UEnum::GetValueAsName(TriggerEvent).ToString())
			AbilityInputBinding.OnPressedHandle = InputComponent->BindAction(InputAction, TriggerEvent, this, &UMGCAbilityInputBindingComponent::OnAbilityInputPressed, InputAction).GetHandle();
		}

		// Released event
		if (AbilityInputBinding.OnReleasedHandle == 0)
		{
			AbilityInputBinding.OnReleasedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UMGCAbilityInputBindingComponent::OnAbilityInputReleased, InputAction).GetHandle();
		}
	}
}
