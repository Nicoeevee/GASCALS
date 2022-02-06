// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Abilities/GSCAbilitySystemComponent.h"

#include "AbilitySystemGlobals.h"
#include "GSCLog.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Abilities/GSCGameplayAbility.h"
#include "Actors/Characters/GSCCharacterBase.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCCoreComponent.h"

void UGSCAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	AbilityActivatedCallbacks.AddUObject(this, &UGSCAbilitySystemComponent::OnAbilityActivatedCallback);
	AbilityFailedCallbacks.AddUObject(this, &UGSCAbilitySystemComponent::OnAbilityFailedCallback);
	AbilityEndedCallbacks.AddUObject(this, &UGSCAbilitySystemComponent::OnAbilityEndedCallback);
}

UGSCAbilitySystemComponent* UGSCAbilitySystemComponent::GetAbilitySystemComponentFromActor(const AActor* Actor, const bool LookForComponent)
{
	return Cast<UGSCAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, LookForComponent));
}

TArray<UGSCGameplayAbility*> UGSCAbilitySystemComponent::GetActiveAbilitiesByTags(const FGameplayTagContainer& GameplayTagContainer) const
{
	TArray<UGSCGameplayAbility*> ActiveAbilities;
	TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, MatchingGameplayAbilities, false);

	// Iterate the list of all ability specs
	for (FGameplayAbilitySpec* Spec : MatchingGameplayAbilities)
	{
		// Iterate all instances on this ability spec
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			if (ActiveAbility->IsActive())
			{
				ActiveAbilities.Add(Cast<UGSCGameplayAbility>(ActiveAbility));
			}
		}
	}

	return ActiveAbilities;
}

TArray<UGSCGameplayAbility*> UGSCAbilitySystemComponent::GetActiveAbilitiesByClass(const TSubclassOf<UGameplayAbility> AbilityToSearch) const
{
	TArray<FGameplayAbilitySpec> Specs = GetActivatableAbilities();
	TArray<struct FGameplayAbilitySpec*> MatchingGameplayAbilities;
	TArray<UGSCGameplayAbility*> ActiveAbilities;

	// First, search for matching Abilities for this class
	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		if (Spec.Ability && Spec.Ability->GetClass()->IsChildOf(AbilityToSearch))
		{
			MatchingGameplayAbilities.Add(const_cast<FGameplayAbilitySpec*>(&Spec));
		}
	}

	// Iterate the list of all ability specs
	for (FGameplayAbilitySpec* Spec : MatchingGameplayAbilities)
	{
		// Iterate all instances on this ability spec, which can include instance per execution abilities
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			if (ActiveAbility->IsActive())
			{
				ActiveAbilities.Add(Cast<UGSCGameplayAbility>(ActiveAbility));
			}
		}
	}

	return ActiveAbilities;
}

bool UGSCAbilitySystemComponent::ActivateAbilityByTags(const FGameplayTagContainer AbilityTags, UGSCGameplayAbility*& ActivatedAbility, const bool bAllowRemoteActivation)
{
	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTags, AbilitiesToActivate);

	const uint32 Count = AbilitiesToActivate.Num();

	if (Count == 0)
	{
		GSC_LOG(Warning, TEXT("UGSCAbilitySystemComponent::TryActivateAbilityWithTag No matching Ability for %s"), *AbilityTags.ToStringSimple())
        return false;
	}

	FGameplayAbilitySpec* Spec = AbilitiesToActivate[FMath::RandRange(0, Count - 1)];

	// actually trigger the ability
	const bool bSuccess = TryActivateAbility(Spec->Handle, bAllowRemoteActivation);

	TArray<UGSCGameplayAbility*> ActiveAbilities = GetActiveAbilitiesByTags(AbilityTags);
	if (ActiveAbilities.Num() == 0)
	{
		GSC_VLOG(this, Log, TEXT("GSCAbilitySystemComponent::ActivateAbilityByTags Couldn't get back active abilities with tags %s"), *AbilityTags.ToStringSimple());
	}

	if (bSuccess && ActiveAbilities.Num() > 0)
	{
		ActivatedAbility = ActiveAbilities[0];
	}

	return bSuccess;
}

bool UGSCAbilitySystemComponent::ActivateAbilityByClass(const TSubclassOf<UGameplayAbility> AbilityToActivate, UGSCGameplayAbility*& ActivatedAbility, bool bAllowRemoteActivation)
{
	GSC_VLOG(this, Log, TEXT("GSCAbilitySystemComponent::ActivateAbilityByClass Try Activate Ability with Class %s"), *AbilityToActivate->GetName());
	const bool bSuccess = TryActivateAbilityByClass(AbilityToActivate);

	TArray<UGSCGameplayAbility*> ActiveAbilities = GetActiveAbilitiesByClass(AbilityToActivate);
	if (ActiveAbilities.Num() == 0)
	{
		GSC_VLOG(this, Log, TEXT("GSCAbilitySystemComponent::ActivateAbilityByClass Couldn't get back active abilities with Class %s"), *AbilityToActivate->GetName());
	}

	if (bSuccess && ActiveAbilities.Num() > 0)
	{
		ActivatedAbility = ActiveAbilities[0];
	}

	return bSuccess;
}

void UGSCAbilitySystemComponent::OnAbilityActivatedCallback(UGameplayAbility* Ability)
{
	GSC_VLOG(this, Log, TEXT("UGSCAbilitySystemComponent::OnAbilityActivatedCallback %s"), *Ability->GetName());

	AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		GSC_LOG(Warning, TEXT("UGSCAbilitySystemComponent::OnAbilityActivated No OwnerActor for this ability: %s"), *Ability->GetName());
		return;
	}

	UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	UGSCGameplayAbility* GSCAbility = Cast<UGSCGameplayAbility>(Ability);
	if (CoreComponent && GSCAbility)
	{
		CoreComponent->OnAbilityActivated.Broadcast(GSCAbility);
	}
}

void UGSCAbilitySystemComponent::OnAbilityFailedCallback(const UGameplayAbility* Ability, const FGameplayTagContainer& Tags)
{
	GSC_VLOG(this, Log, TEXT("UGSCAbilitySystemComponent::OnAbilityFailedCallback %s"), *Ability->GetName());

	AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		GSC_LOG(Warning, TEXT("UGSCAbilitySystemComponent::OnAbilityFailed No OwnerActor for this ability: %s Tags: %s"), *Ability->GetName(), *Tags.ToString());
		return;
	}

	UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Avatar);
	const UGSCGameplayAbility* GSCAbility = Cast<UGSCGameplayAbility>(Ability);
	if (CoreComponent && GSCAbility)
	{
		CoreComponent->OnAbilityFailed.Broadcast(GSCAbility, Tags);
	}

	if (AbilityQueueComponent && GSCAbility)
	{
		AbilityQueueComponent->OnAbilityFailed(GSCAbility, Tags);
	}
}

void UGSCAbilitySystemComponent::OnAbilityEndedCallback(UGameplayAbility* Ability)
{
	GSC_VLOG(this, Log, TEXT("UGSCAbilitySystemComponent::OnAbilityEndedCallback %s"), *Ability->GetName());
	UGSCGameplayAbility* GSCAbility = Cast<UGSCGameplayAbility>(Ability);
	if (!GSCAbility)
	{
		GSC_LOG(Warning, TEXT("UGSCAbilitySystemComponent::OnAbilityEndedCallback Couldn't cast to UGSGameplayAbility: %s"), *Ability->GetName());
		return;
	}

	AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		GSC_LOG(Warning, TEXT("UGSCAbilitySystemComponent::OnAbilityEndedCallback No OwnerActor for this ability: %s"), *Ability->GetName());
		return;
	}

	UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Avatar);
	if (CoreComponent)
	{
		CoreComponent->OnAbilityEnded.Broadcast(GSCAbility);
	}

	if (AbilityQueueComponent)
	{
		AbilityQueueComponent->OnAbilityEnded(GSCAbility);
	}
}

void UGSCAbilitySystemComponent::K2_ExecuteGameplayCue(const FGameplayTag GameplayCueTag, const FGameplayEffectContextHandle Context)
{
	ExecuteGameplayCue(GameplayCueTag, Context);
}

void UGSCAbilitySystemComponent::K2_ExecuteGameplayCueWithParams(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	ExecuteGameplayCue(GameplayCueTag, GameplayCueParameters);
}

void UGSCAbilitySystemComponent::K2_AddGameplayCue(const FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context)
{
	AddGameplayCue(GameplayCueTag, Context);
}

void UGSCAbilitySystemComponent::K2_AddGameplayCueWithParams(const FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameter)
{
	AddGameplayCue(GameplayCueTag, GameplayCueParameter);
}

void UGSCAbilitySystemComponent::K2_RemoveGameplayCue(const FGameplayTag GameplayCueTag)
{
	RemoveGameplayCue(GameplayCueTag);
}

void UGSCAbilitySystemComponent::K2_RemoveAllGameplayCues()
{
	RemoveAllGameplayCues();
}

