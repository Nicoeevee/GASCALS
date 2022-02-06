// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Abilities/GSCBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCComboManagerComponent.h"
#include "Components/GSCCoreComponent.h"
#include "Core/Interfaces/GSCCompanionInterface.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

UGSCAbilitySystemComponent* UGSCBlueprintFunctionLibrary::GetAbilitySystemComponentFromActor(const AActor* Actor)
{
	return UGSCAbilitySystemComponent::GetAbilitySystemComponentFromActor(Actor);
}

UGSCComboManagerComponent* UGSCBlueprintFunctionLibrary::GetComboManagerComponent(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	const IGSCCompanionInterface* CompanionInterface = Cast<IGSCCompanionInterface>(Actor);
	if (CompanionInterface)
	{
		return CompanionInterface->GetComboManagerComponent();
	}

	// GSC_LOG(Log, TEXT("GetComboManagerComponent called on %s that is not IGSCCompanionInterface. This slow!"), *Actor->GetName())

	// Fall back to a component search to better support BP-only actors
	return Actor->FindComponentByClass<UGSCComboManagerComponent>();
}

UGSCCoreComponent* UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// TODO: Do we keep this idea of BP implementable interface to avoid component search ?
	if (Actor->GetClass()->ImplementsInterface(UGSCCompanionInterface::StaticClass()))
	{
		UGSCCoreComponent* CoreComponent = IGSCCompanionInterface::Execute_K2_GetCompanionCoreComponent(Actor);
		if (CoreComponent)
		{
			return CoreComponent;
		}

		const IGSCCompanionInterface* CompanionInterface = Cast<IGSCCompanionInterface>(Actor);
		if (CompanionInterface)
		{
			return CompanionInterface->GetCompanionCoreComponent();
		}
	}

	// Fall back to a component search to better support BP-only actors
	return Actor->FindComponentByClass<UGSCCoreComponent>();
}

UGSCAbilityQueueComponent* UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(const AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	const IGSCCompanionInterface* CompanionInterface = Cast<IGSCCompanionInterface>(Actor);
	if (CompanionInterface)
	{
		return CompanionInterface->GetAbilityQueueComponent();
	}

	return Actor->FindComponentByClass<UGSCAbilityQueueComponent>();
}

bool UGSCBlueprintFunctionLibrary::AddLooseGameplayTagsToActor(AActor* Actor, const FGameplayTagContainer GameplayTags)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddLooseGameplayTags(GameplayTags);
		return true;
	}

	return false;
}

bool UGSCBlueprintFunctionLibrary::RemoveLooseGameplayTagsFromActor(AActor* Actor, const FGameplayTagContainer GameplayTags)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTags(GameplayTags);
		return true;
	}

	return false;
}

bool UGSCBlueprintFunctionLibrary::HasMatchingGameplayTag(AActor* Actor, const FGameplayTag GameplayTag)
{
	UGSCCoreComponent* CCC = GetCompanionCoreComponent(Actor);
	if (!CCC)
	{
		return false;
	}

	return CCC->HasMatchingGameplayTag(GameplayTag);
}

bool UGSCBlueprintFunctionLibrary::HasAnyMatchingGameplayTag(AActor* Actor, const FGameplayTagContainer GameplayTags)
{
	UGSCCoreComponent* CCC = GetCompanionCoreComponent(Actor);
	if (!CCC)
	{
		return false;
	}

	return CCC->HasAnyMatchingGameplayTags(GameplayTags);
}

FString UGSCBlueprintFunctionLibrary::GetDebugStringFromAttribute(const FGameplayAttribute Attribute)
{
	return Attribute.GetName();
}

void UGSCBlueprintFunctionLibrary::GetAllAttributes(const TSubclassOf<UAttributeSet> AttributeSetClass, TArray<FGameplayAttribute>& OutAttributes)
{
	const UClass* Class = AttributeSetClass.Get();
	for (TFieldIterator<FProperty> It(Class); It; ++It)
	{
		if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(*It))
		{
			OutAttributes.Push(FGameplayAttribute(FloatProperty));
		}
		else if (FGameplayAttribute::IsGameplayAttributeDataProperty(*It))
		{
			OutAttributes.Push(FGameplayAttribute(*It));
		}
	}
}

bool UGSCBlueprintFunctionLibrary::NotEqual_GameplayAttributeGameplayAttribute(FGameplayAttribute A, FString B)
{
	return A.GetName() != B;
}

void UGSCBlueprintFunctionLibrary::ExecuteGameplayCueForActor(AActor* Actor, FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ExecuteGameplayCue(GameplayCueTag, Context);
	}
}

void UGSCBlueprintFunctionLibrary::ExecuteGameplayCueWithParams(AActor* Actor, FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->ExecuteGameplayCue(GameplayCueTag, GameplayCueParameters);
	}
}

void UGSCBlueprintFunctionLibrary::AddGameplayCue(AActor* Actor, FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddGameplayCue(GameplayCueTag, Context);
	}
}

void UGSCBlueprintFunctionLibrary::AddGameplayCueWithParams(AActor* Actor, FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameter)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->AddGameplayCue(GameplayCueTag, GameplayCueParameter);
	}
}

void UGSCBlueprintFunctionLibrary::RemoveGameplayCue(AActor* Actor, FGameplayTag GameplayCueTag)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveGameplayCue(GameplayCueTag);
	}
}

void UGSCBlueprintFunctionLibrary::RemoveAllGameplayCues(AActor* Actor)
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveAllGameplayCues();
	}
}
