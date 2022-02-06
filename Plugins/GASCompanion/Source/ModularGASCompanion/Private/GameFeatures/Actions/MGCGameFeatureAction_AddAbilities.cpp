// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "GameFeatures/Actions/MGCGameFeatureAction_AddAbilities.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "EngineUtils.h"
#include "GameFeaturesSubsystemSettings.h"
#include "ModularGASCompanionLog.h"
#include "Abilities/MGCAbilityInputBindingComponent.h"
#include "Abilities/MGCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/GSCCoreComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/World.h" // for FWorldDelegates::OnStartGameInstance
#include "Engine/Engine.h" // for FWorldContext
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"

#define LOCTEXT_NAMESPACE "ModularGASCompanion"

void UMGCGameFeatureAction_AddAbilities::OnGameFeatureActivating()
{
	if (!ensureAlways(ActiveExtensions.Num() == 0) || !ensureAlways(ComponentRequests.Num() == 0) || !ensureAlways(ExtensionsRequests.Num() == 0))
	{
		Reset();
	}

	GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &UMGCGameFeatureAction_AddAbilities::HandleGameInstanceStart);

	check(ComponentRequests.Num() == 0);
	check(ExtensionsRequests.Num() == 0);

	// Add to any worlds with associated game instances that have already been initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		AddToWorld(WorldContext);
	}

	Super::OnGameFeatureActivating();
}

void UMGCGameFeatureAction_AddAbilities::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);

	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartHandle);

	Reset();
}

#if WITH_EDITORONLY_DATA
void UMGCGameFeatureAction_AddAbilities::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (!UAssetManager::IsValid())
	{
		return;
	}

	auto AddBundleAsset = [&AssetBundleData](const FSoftObjectPath& SoftObjectPath)
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, SoftObjectPath);
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, SoftObjectPath);
	};

	for (const FMGCGameFeatureAbilitiesEntry& Entry : AbilitiesList)
	{
		for (const FMGCGameFeatureAbilityMapping& Ability : Entry.GrantedAbilities)
		{
			AddBundleAsset(Ability.AbilityType.ToSoftObjectPath());
			if (!Ability.InputAction.IsNull())
			{
				AddBundleAsset(Ability.InputAction.ToSoftObjectPath());
			}
		}

		for (const FMGCGameFeatureAttributeSetMapping& Attributes : Entry.GrantedAttributes)
		{
			AddBundleAsset(Attributes.AttributeSet.ToSoftObjectPath());
			if (!Attributes.InitializationData.IsNull())
			{
				AddBundleAsset(Attributes.InitializationData.ToSoftObjectPath());
			}
		}
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UMGCGameFeatureAction_AddAbilities::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FMGCGameFeatureAbilitiesEntry& Entry : AbilitiesList)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullActor", "Null ActorClass at index {0} in AbilitiesList"), FText::AsNumber(EntryIndex)));
		}

		if (Entry.GrantedAbilities.Num() == 0 && Entry.GrantedAttributes.Num() == 0)
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNoAddOns", "Empty GrantedAbilities and GrantedAttributes at index {0} in AbilitiesList"), FText::AsNumber(EntryIndex)));
		}

		int32 AbilityIndex = 0;
		for (const FMGCGameFeatureAbilityMapping& Ability : Entry.GrantedAbilities)
		{
			if (Ability.AbilityType.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullAbility", "Null AbilityType at index {0} in AbilitiesList[{1}].GrantedAbilities"), FText::AsNumber(AbilityIndex), FText::AsNumber(EntryIndex)));
			}
			++AbilityIndex;
		}

		int32 AttributesIndex = 0;
		for (const FMGCGameFeatureAttributeSetMapping& Attributes : Entry.GrantedAttributes)
		{
			if (Attributes.AttributeSet.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullAttributeSet", "Null AttributeSetType at index {0} in AbilitiesList[{1}].GrantedAttributes"), FText::AsNumber(AttributesIndex), FText::AsNumber(EntryIndex)));
			}
			++AttributesIndex;
		}

		++EntryIndex;
	}

	return Result;
}
#endif

void UMGCGameFeatureAction_AddAbilities::Reset()
{
	while (ActiveExtensions.Num() != 0)
	{
		const auto ExtensionIt = ActiveExtensions.CreateIterator();
		RemoveActorAbilities(ExtensionIt->Key);
	}

	ExtensionsRequests.Empty();
	ComponentRequests.Empty();
}

void UMGCGameFeatureAction_AddAbilities::HandleActorExtension(AActor* Actor, const FName EventName, const int32 EntryIndex)
{
#if ENGINE_MAJOR_VERSION == 5
	if (AbilitiesList.IsValidIndex(EntryIndex))
	{
		MGC_LOG(Verbose, TEXT("UMGCGameFeatureAction_AddAbilities::HandleActorExtension '%s'. EventName: %s"), *Actor->GetPathName(), *EventName.ToString());
		const FMGCGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
		if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
		{
			MGC_LOG(Verbose, TEXT("UMGCGameFeatureAction_AddAbilities::HandleActorExtension remove '%s'. Abilities will be removed."), *Actor->GetPathName());
			RemoveActorAbilities(Actor);
		}
		else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
		{
			MGC_LOG(Verbose, TEXT("UMGCGameFeatureAction_AddAbilities::HandleActorExtension add '%s'. Abilities will be granted."), *Actor->GetPathName());
			AddActorAbilities(Actor, Entry);
		}
	}
#else
	if (AbilitiesList.IsValidIndex(EntryIndex))
	{
		MGC_LOG(Verbose, TEXT("UMGCGameFeatureAction_AddAbilities::HandleActorExtension '%s'. EventName: %s"), *Actor->GetPathName(), *EventName.ToString());
		const FMGCGameFeatureAbilitiesEntry& Entry = AbilitiesList[EntryIndex];
		if (EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionRemoved || EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ReceiverRemoved)
		{
			MGC_LOG(Verbose, TEXT("UMGCGameFeatureAction_AddAbilities::HandleActorExtension remove '%s'. Abilities will be removed."), *Actor->GetPathName());
			RemoveActorAbilities(Actor);
		}
		else if (EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionAdded || EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady)
		{
			MGC_LOG(Verbose, TEXT("UMGCGameFeatureAction_AddAbilities::HandleActorExtension add '%s'. Abilities will be granted."), *Actor->GetPathName());
			AddActorAbilities(Actor, Entry);
		}
	}
#endif

	// TODO: Handle 4.27
}

void UMGCGameFeatureAction_AddAbilities::AddActorAbilities(AActor* Actor, const FMGCGameFeatureAbilitiesEntry& AbilitiesEntry)
{
	UMGCAbilitySystemComponent* AbilitySystemComponent = FindOrAddComponentForActor<UMGCAbilitySystemComponent>(Actor, AbilitiesEntry);
	if (!AbilitySystemComponent)
	{
		MGC_LOG(Error, TEXT("Failed to find/add an ability component to '%s'. Abilities will not be granted."), *Actor->GetPathName());
		return;
	}

	FActorExtensions AddedExtensions;
	AddedExtensions.Abilities.Reserve(AbilitiesEntry.GrantedAbilities.Num());
	AddedExtensions.Attributes.Reserve(AbilitiesEntry.GrantedAttributes.Num());

	for (const FMGCGameFeatureAbilityMapping& Ability : AbilitiesEntry.GrantedAbilities)
	{
		if (!Ability.AbilityType.IsNull())
		{
			FGameplayAbilitySpecHandle AbilityHandle;
			FGameplayAbilitySpec NewAbilitySpec(Ability.AbilityType.LoadSynchronous());

			// Try to grant the ability first
			if (AbilitySystemComponent->IsOwnerActorAuthoritative())
			{
				// Only Grant abilities on authority
				MGC_LOG(Verbose, TEXT("AddActorAbilities: Authority, Grant Ability (%s) with input ID: %d"), *NewAbilitySpec.Ability->GetClass()->GetName())
				AbilityHandle = AbilitySystemComponent->GiveAbility(NewAbilitySpec);
			}
			else
			{
				// For clients, try to get ability spec and update handle used later on for input binding
				const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Ability.AbilityType.LoadSynchronous());
				if (AbilitySpec)
				{
					AbilityHandle = AbilitySpec->Handle;
				}
				MGC_LOG(Verbose, TEXT("AddActorAbilities: Not Authority, try to find ability handle from spec: %s"), *AbilityHandle.ToString())
			}

			// Handle Input Mapping now
			if (!Ability.InputAction.IsNull())
			{
				UMGCAbilityInputBindingComponent* InputComponent = FindOrAddComponentForActor<UMGCAbilityInputBindingComponent>(Actor, AbilitiesEntry);
				if (InputComponent)
				{
					MGC_LOG(Verbose, TEXT("AddActorAbilities: Try to setup input binding for '%s': '%s' (%s)"), *Ability.InputAction.ToString(), *AbilityHandle.ToString(), *NewAbilitySpec.Handle.ToString())
					if (AbilityHandle.IsValid())
					{
						// Setup input binding if AbilityHandle is valid and already granted (on authority, or when Game Features is active by default)
						InputComponent->SetInputBinding(Ability.InputAction.LoadSynchronous(), Ability.TriggerEvent, AbilityHandle);
					}
					else
					{
						// Register a delegate triggered when ability is granted and available on clients (needed when Game Features are made active during play)
						UInputAction* InputAction = Ability.InputAction.LoadSynchronous();
						FDelegateHandle DelegateHandle = AbilitySystemComponent->OnGiveAbilityDelegate.AddUObject(this, &UMGCGameFeatureAction_AddAbilities::HandleOnGiveAbility, InputComponent, InputAction, Ability.TriggerEvent, NewAbilitySpec);
						AddedExtensions.InputBindingDelegateHandles.Add(DelegateHandle);
					}
				}
				else
				{
					MGC_LOG(Error, TEXT("Failed to find/add an ability input binding component to '%s' -- are you sure it's a pawn class?"), *Actor->GetPathName());
				}
			}

			AddedExtensions.Abilities.Add(AbilityHandle);
		}
	}

	for (const FMGCGameFeatureAttributeSetMapping& Attributes : AbilitiesEntry.GrantedAttributes)
	{
		if (!Attributes.AttributeSet.IsNull() && AbilitySystemComponent->IsOwnerActorAuthoritative())
		{
			TSubclassOf<UAttributeSet> AttributeSetType = Attributes.AttributeSet.LoadSynchronous();
			if (!AttributeSetType)
			{
				continue;
			}

			// Prevent adding the same attribute set multiple times (if already registered by another GF or on Actor ASC directly)
			if (HasAttributeSet(AbilitySystemComponent, AttributeSetType))
			{
				MGC_LOG(Warning, TEXT("AddActorAbilities: %s AttributeSet is already added to %s"), *AttributeSetType->GetName(), *Actor->GetName())
				continue;
			}

			UAttributeSet* AttributeSet = NewObject<UAttributeSet>(Actor, AttributeSetType);
			if (!Attributes.InitializationData.IsNull())
			{
				UDataTable* InitData = Attributes.InitializationData.LoadSynchronous();
				if (InitData)
				{
					AttributeSet->InitFromMetaDataTable(InitData);
				}
			}

			AddedExtensions.Attributes.Add(AttributeSet);
			AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
			AbilitySystemComponent->bIsNetDirty = true;
		}
	}

	UGSCCoreComponent* CoreComponent = Actor->FindComponentByClass<UGSCCoreComponent>();
	if (CoreComponent)
	{
		// If we have companion core component, make sure to notify we may have added attributes
		CoreComponent->ShutdownAbilitySystemDelegates(AbilitySystemComponent);
		CoreComponent->RegisterAbilitySystemDelegates(AbilitySystemComponent);
	}

	ActiveExtensions.Add(Actor, AddedExtensions);
}

void UMGCGameFeatureAction_AddAbilities::RemoveActorAbilities(const AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return;
	}

	if (FActorExtensions* ActorExtensions = ActiveExtensions.Find(Actor))
	{
		// TODO: In 4.27 and as client, getting random invalid interface address with GetAbilitySystemComponentFromActor
		// UMGCAbilitySystemComponent* AbilitySystemComponent = Cast<UMGCAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor));
		UMGCAbilitySystemComponent* AbilitySystemComponent = Actor->FindComponentByClass<UMGCAbilitySystemComponent>();
		if (AbilitySystemComponent)
		{
			for (UAttributeSet* AttribSetInstance : ActorExtensions->Attributes)
			{
				AbilitySystemComponent->GetSpawnedAttributes_Mutable().Remove(AttribSetInstance);
			}

			UMGCAbilityInputBindingComponent* InputComponent = Actor->FindComponentByClass<UMGCAbilityInputBindingComponent>();
			for (const FGameplayAbilitySpecHandle AbilityHandle : ActorExtensions->Abilities)
			{
				if (InputComponent)
				{
					InputComponent->ClearInputBinding(AbilityHandle);
				}

				// Only Clear abilities on authority
				if (AbilitySystemComponent->IsOwnerActorAuthoritative())
				{
					AbilitySystemComponent->SetRemoveAbilityOnEnd(AbilityHandle);
				}
			}
		}
		else
		{
			MGC_LOG(Error, TEXT("RemoveActorAbilities: Not able to find MGCAbilitySystemComponent"))
		}

		// Clear any delegate handled bound previously for this actor
		for (FDelegateHandle InputBindingDelegateHandle : ActorExtensions->InputBindingDelegateHandles)
		{
			if (AbilitySystemComponent)
			{
				AbilitySystemComponent->OnGiveAbilityDelegate.Remove(InputBindingDelegateHandle);
			}
			InputBindingDelegateHandle.Reset();
		}

		ActiveExtensions.Remove(Actor);
	}
}

UActorComponent* UMGCGameFeatureAction_AddAbilities::FindOrAddComponentForActor(UClass* ComponentType, const AActor* Actor, const FMGCGameFeatureAbilitiesEntry& AbilitiesEntry)
{
	UActorComponent* Component = Actor->FindComponentByClass(ComponentType);

	bool bMakeComponentRequest = (Component == nullptr);
	if (Component)
	{
		// Check to see if this component was created from a different `UGameFrameworkComponentManager` request.
		// `Native` is what `CreationMethod` defaults to for dynamically added components.
		if (Component->CreationMethod == EComponentCreationMethod::Native)
		{
			// Attempt to tell the difference between a true native component and one created by the GameFrameworkComponent system.
			// If it is from the UGameFrameworkComponentManager, then we need to make another request (requests are ref counted).
			const UObject* ComponentArchetype = Component->GetArchetype();
			bMakeComponentRequest = ComponentArchetype->HasAnyFlags(RF_ClassDefaultObject);
		}
	}

	if (bMakeComponentRequest)
	{
		const UWorld* World = Actor->GetWorld();
		const UGameInstance* GameInstance = World->GetGameInstance();

		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			const TSharedPtr<FComponentRequestHandle> RequestHandle = ComponentMan->AddComponentRequest(AbilitiesEntry.ActorClass, ComponentType);
			ComponentRequests.Add(RequestHandle);
		}

		if (!Component)
		{
			Component = Actor->FindComponentByClass(ComponentType);
			ensureAlways(Component);
		}
	}

	return Component;
}

void UMGCGameFeatureAction_AddAbilities::AddToWorld(const FWorldContext& WorldContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
		UMGCGameFrameworkExtensionManager* ExtensionManager = UGameInstance::GetSubsystem<UMGCGameFrameworkExtensionManager>(GameInstance);

		if (ComponentMan && ExtensionManager)
		{
			int32 EntryIndex = 0;

			MGC_LOG(Verbose, TEXT("Adding abilities for %s to world %s"), *GetPathNameSafe(this), *World->GetDebugDisplayName());

#if ENGINE_MAJOR_VERSION == 5
			for (const FMGCGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					const UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
						this,
						&UMGCGameFeatureAction_AddAbilities::HandleActorExtension,
						EntryIndex
					);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentMan->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);

					ComponentRequests.Add(ExtensionRequestHandle);
					EntryIndex++;
				}
			}
#else
			for (const FMGCGameFeatureAbilitiesEntry& Entry : AbilitiesList)
			{
				if (!Entry.ActorClass.IsNull())
				{
					const UMGCGameFrameworkExtensionManager::FExtensionHandlerDelegate AddAbilitiesDelegate = UMGCGameFrameworkExtensionManager::FExtensionHandlerDelegate::CreateUObject(
						this,
						&UMGCGameFeatureAction_AddAbilities::HandleActorExtension,
						EntryIndex
					);
					TSharedPtr<FMGCComponentRequestHandle> ExtensionRequestHandle = ExtensionManager->AddExtensionHandler(Entry.ActorClass, AddAbilitiesDelegate);

					// ComponentRequests.Add(ExtensionRequestHandle);
					ExtensionsRequests.Add(ExtensionRequestHandle);
					EntryIndex++;
				}
			}
#endif
		}
	}
}

void UMGCGameFeatureAction_AddAbilities::HandleGameInstanceStart(UGameInstance* GameInstance)
{
	if (const FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		AddToWorld(*WorldContext);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UMGCGameFeatureAction_AddAbilities::HandleOnGiveAbility(FGameplayAbilitySpec& AbilitySpec, UMGCAbilityInputBindingComponent* InputComponent, UInputAction* InputAction, const EMGCAbilityTriggerEvent TriggerEvent, const FGameplayAbilitySpec NewAbilitySpec)
{
	MGC_LOG(
		Verbose,
		TEXT("UMGCGameFeatureAction_AddAbilities::HandleOnGiveAbility: %s, Ability: %s, Input: %s (TriggerEvent: %s) - (InputComponent: %s)"),
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

bool UMGCGameFeatureAction_AddAbilities::HasAttributeSet(UAbilitySystemComponent* AbilitySystemComponent, const TSubclassOf<UAttributeSet> Set)
{
	check(AbilitySystemComponent != nullptr);

	for (const UAttributeSet* SpawnedAttribute : AbilitySystemComponent->GetSpawnedAttributes())
	{
		if (SpawnedAttribute && SpawnedAttribute->IsA(Set))
		{
			return true;
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
