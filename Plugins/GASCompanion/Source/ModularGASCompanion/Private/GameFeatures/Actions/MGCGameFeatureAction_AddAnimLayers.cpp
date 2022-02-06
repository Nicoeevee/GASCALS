// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "GameFeatures/Actions/MGCGameFeatureAction_AddAnimLayers.h"

#include "GameFeaturesSubsystemSettings.h"
#include "ModularGASCompanionLog.h"
#include "Animation/MGCLinkAnimLayersComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/AssetManager.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"

#define LOCTEXT_NAMESPACE "ModularGASCompanion"

void UMGCGameFeatureAction_AddAnimLayers::OnGameFeatureActivating()
{
#if ENGINE_MAJOR_VERSION == 5
	if (!ensureAlways(ComponentRequestHandles.IsEmpty()))
#else
	if (!ensureAlways(ComponentRequestHandles.Num() == 0))
#endif
	{
		Reset();
	}

	GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &UMGCGameFeatureAction_AddAnimLayers::HandleGameInstanceStart);

	check(ComponentRequestHandles.Num() == 0);

	// Add to any worlds with associated game instances that have already been initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		AddToWorld(WorldContext);
	}
}

void UMGCGameFeatureAction_AddAnimLayers::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartHandle);

	// Releasing the handles will also remove the components from any registered actors too
	ComponentRequestHandles.Empty();

	Reset();
}

#if WITH_EDITORONLY_DATA
void UMGCGameFeatureAction_AddAnimLayers::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (!UAssetManager::IsValid())
	{
		return;
	}

	for (FMGCAnimLayerEntry& Entry : AnimLayerEntries)
	{
		for (TSoftClassPtr<UAnimInstance> AnimLayer : Entry.AnimLayers)
		{
			AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, AnimLayer.ToSoftObjectPath());
			AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, AnimLayer.ToSoftObjectPath());
		}
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UMGCGameFeatureAction_AddAnimLayers::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const FMGCAnimLayerEntry& Entry : AnimLayerEntries)
	{
		if (Entry.ActorClass.IsNull())
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("AnimLayerEntryHasNullActor", "Null ActorClass at index {0} in AnimLayerEntries"), FText::AsNumber(EntryIndex)));
		}

#if ENGINE_MAJOR_VERSION == 5
		if (Entry.AnimLayers.IsEmpty())
#else
		if (Entry.AnimLayers.Num() == 0)
#endif
		{
			Result = EDataValidationResult::Invalid;
			ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNoAnimLayers", "Empty AnimLayers at index {0} in AnimLayerEntries"), FText::AsNumber(EntryIndex)));
		}

		int32 AnimLayerIndex = 0;
		for (const TSoftClassPtr<UAnimInstance> AnimLayer : Entry.AnimLayers)
		{
			if (AnimLayer.IsNull())
			{
				Result = EDataValidationResult::Invalid;
				ValidationErrors.Add(FText::Format(LOCTEXT("EntryHasNullAnimLayer", "Null AnimLayer at index {0} in AnimLayerEntries[{1}].AnimLayers"), FText::AsNumber(AnimLayerIndex), FText::AsNumber(EntryIndex)));
			}

			++AnimLayerIndex;
		}

		++EntryIndex;
	}

	return Result;
}
#endif

UActorComponent* UMGCGameFeatureAction_AddAnimLayers::FindOrAddComponentForActor(UClass* ComponentType, AActor* Actor, const FMGCAnimLayerEntry& AnimLayerEntry)
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
			UObject* ComponentArchetype = Component->GetArchetype();
			bMakeComponentRequest = ComponentArchetype->HasAnyFlags(RF_ClassDefaultObject);
		}
	}

	if (bMakeComponentRequest)
	{
		UWorld* World = Actor->GetWorld();
		UGameInstance* GameInstance = World->GetGameInstance();

		if (UGameFrameworkComponentManager* ComponentMan = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
		{
			const TSharedPtr<FComponentRequestHandle> RequestHandle = ComponentMan->AddComponentRequest(AnimLayerEntry.ActorClass, ComponentType);
			ComponentRequestHandles.Add(RequestHandle);
		}

		if (!Component)
		{
			Component = Actor->FindComponentByClass(ComponentType);
			ensureAlways(Component);
		}
	}

	return Component;
}

void UMGCGameFeatureAction_AddAnimLayers::Reset()
{
	// TODO: Handle reset of anim layers
}

void UMGCGameFeatureAction_AddAnimLayers::HandleActorExtension(AActor* Actor, const FName EventName, const int32 EntryIndex)
{
#if ENGINE_MAJOR_VERSION == 5
	if (AnimLayerEntries.IsValidIndex(EntryIndex))
	{
		const FMGCAnimLayerEntry& Entry = AnimLayerEntries[EntryIndex];
		if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
		{
			RemoveAnimLayers(Actor);
		}
		else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
		{
			AddAnimLayers(Actor, Entry);
		}
	}
#else
	if (AnimLayerEntries.IsValidIndex(EntryIndex))
	{
		const FMGCAnimLayerEntry& Entry = AnimLayerEntries[EntryIndex];
		if (EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionRemoved || EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ReceiverRemoved)
		{
			RemoveAnimLayers(Actor);
		}
		else if (EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionAdded || EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady)
		{
			AddAnimLayers(Actor, Entry);
		}
	}
#endif
}

void UMGCGameFeatureAction_AddAnimLayers::AddAnimLayers(AActor* Actor, const FMGCAnimLayerEntry& Entry)
{
	MGC_LOG(Verbose, TEXT("AddAnimLayers to '%s'."), *Actor->GetPathName());

	UMGCLinkAnimLayersComponent* LinkAnimLayersComponent = FindOrAddComponentForActor<UMGCLinkAnimLayersComponent>(Actor, Entry);
	if (!LinkAnimLayersComponent)
	{
		MGC_LOG(Error, TEXT("Failed to find/add a LinkAnimLayersComponent to '%s'. Anim Layers will not be linked."), *Actor->GetPathName());
		return;
	}

	FActorExtensions AddedExtensions;
	AddedExtensions.AnimLayers.Reserve(Entry.AnimLayers.Num());

	for (const TSoftClassPtr<UAnimInstance> AnimLayerType : Entry.AnimLayers)
	{
		UClass* AnimInstanceType = AnimLayerType.LoadSynchronous();
		LinkAnimLayersComponent->LinkAnimLayer(AnimInstanceType);
		AddedExtensions.AnimLayers.Add(AnimInstanceType);
	}

	ActiveExtensions.Add(Actor, AddedExtensions);
}

void UMGCGameFeatureAction_AddAnimLayers::RemoveAnimLayers(AActor* Actor)
{
	MGC_LOG(Verbose, TEXT("RemoveAnimLayers from '%s'."), *Actor->GetPathName());
	if (FActorExtensions* ActorExtensions = ActiveExtensions.Find(Actor))
	{
		if (UMGCLinkAnimLayersComponent* LinkAnimLayersComponent = Actor->FindComponentByClass<UMGCLinkAnimLayersComponent>())
		{
			for (const TSubclassOf<UAnimInstance> AnimLayer : ActorExtensions->AnimLayers)
			{
				LinkAnimLayersComponent->UnlinkAnimLayer(AnimLayer);
			}
		}

		ActiveExtensions.Remove(Actor);
	}
}

void UMGCGameFeatureAction_AddAnimLayers::AddToWorld(const FWorldContext& WorldContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{

		UGameFrameworkComponentManager* GFCM = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
		UMGCGameFrameworkExtensionManager* ExtensionManager = UGameInstance::GetSubsystem<UMGCGameFrameworkExtensionManager>(GameInstance);

		if (GFCM && ExtensionManager)
		{
			int32 EntryIndex = 0;
			for (const FMGCAnimLayerEntry& Entry : AnimLayerEntries)
			{
#if ENGINE_MAJOR_VERSION == 5
				if (!Entry.ActorClass.IsNull())
				{
					const UGameFrameworkComponentManager::FExtensionHandlerDelegate AddAnimLayersDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
						this,
						&UMGCGameFeatureAction_AddAnimLayers::HandleActorExtension,
						EntryIndex
					);
					TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = GFCM->AddExtensionHandler(Entry.ActorClass, AddAnimLayersDelegate);

					ComponentRequestHandles.Add(ExtensionRequestHandle);
				}
#else
				if (!Entry.ActorClass.IsNull())
				{
					const UMGCGameFrameworkExtensionManager::FExtensionHandlerDelegate AddAnimLayersDelegate = UMGCGameFrameworkExtensionManager::FExtensionHandlerDelegate::CreateUObject(
						this,
						&UMGCGameFeatureAction_AddAnimLayers::HandleActorExtension,
						EntryIndex
					);
					TSharedPtr<FMGCComponentRequestHandle> ExtensionRequestHandle = ExtensionManager->AddExtensionHandler(Entry.ActorClass, AddAnimLayersDelegate);

					// ComponentRequests.Add(ExtensionRequestHandle);
					ExtensionsRequests.Add(ExtensionRequestHandle);
				}
#endif
				EntryIndex++;
			}
		}
	}
}

void UMGCGameFeatureAction_AddAnimLayers::HandleGameInstanceStart(UGameInstance* GameInstance)
{
	if (FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		AddToWorld(*WorldContext);
	}
}

#undef LOCTEXT_NAMESPACE
