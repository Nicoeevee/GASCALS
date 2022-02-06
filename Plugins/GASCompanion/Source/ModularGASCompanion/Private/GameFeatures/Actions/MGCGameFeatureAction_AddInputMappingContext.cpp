// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "GameFeatures/Actions/MGCGameFeatureAction_AddInputMappingContext.h"

#include "EnhancedInputSubsystems.h"
#include "GameFeaturesSubsystemSettings.h"
#include "ModularGASCompanionLog.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/AssetManager.h"
#include "InputMappingContext.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"

#define LOCTEXT_NAMESPACE "ModularGASCompanion"

void UMGCGameFeatureAction_AddInputMappingContext::OnGameFeatureActivating()
{
#if ENGINE_MAJOR_VERSION == 5
	if (!ensure(ExtensionRequestHandles.IsEmpty()) || !ensure(ControllersAddedTo.IsEmpty()))
#else
	if (!ensure(ExtensionRequestHandles.Num() == 0) || !ensure(ControllersAddedTo.Num() == 0))
#endif
	{
		Reset();
	}

	GameInstanceStartHandle = FWorldDelegates::OnStartGameInstance.AddUObject(this, &UMGCGameFeatureAction_AddInputMappingContext::HandleGameInstanceStart);

	// Add to any worlds with associated game instances that have already been initialized
	for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
	{
		AddToWorld(WorldContext);
	}

	Super::OnGameFeatureActivating();
}

void UMGCGameFeatureAction_AddInputMappingContext::OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context)
{
	Super::OnGameFeatureDeactivating(Context);
	FWorldDelegates::OnStartGameInstance.Remove(GameInstanceStartHandle);

	Reset();
}

#if WITH_EDITORONLY_DATA
void UMGCGameFeatureAction_AddInputMappingContext::AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData)
{
	if (UAssetManager::IsValid())
	{
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateClient, InputMapping.ToSoftObjectPath());
		AssetBundleData.AddBundleAsset(UGameFeaturesSubsystemSettings::LoadStateServer, InputMapping.ToSoftObjectPath());
	}
}
#endif

#if WITH_EDITOR
EDataValidationResult UMGCGameFeatureAction_AddInputMappingContext::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(ValidationErrors), EDataValidationResult::Valid);

	if (InputMapping.IsNull())
	{
		Result = EDataValidationResult::Invalid;
		ValidationErrors.Add(LOCTEXT("NullInputMapping", "Null InputMapping."));
	}

	return Result;
}
#endif

void UMGCGameFeatureAction_AddInputMappingContext::AddToWorld(const FWorldContext& WorldContext)
{
	UWorld* World = WorldContext.World();
	UGameInstance* GameInstance = WorldContext.OwningGameInstance;

	if ((GameInstance != nullptr) && (World != nullptr) && World->IsGameWorld())
	{
		UGameFrameworkComponentManager* ComponentManager = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance);
		UMGCGameFrameworkExtensionManager* ExtensionManager = UGameInstance::GetSubsystem<UMGCGameFrameworkExtensionManager>(GameInstance);

		if (ComponentManager && ExtensionManager)
		{
			if (!InputMapping.IsNull())
			{
#if ENGINE_MAJOR_VERSION == 5
				const UGameFrameworkComponentManager::FExtensionHandlerDelegate AddMappingContextDelegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
					this,
					&UMGCGameFeatureAction_AddInputMappingContext::HandleControllerExtension
				);
				const TSharedPtr<FComponentRequestHandle> ExtensionRequestHandle = ComponentManager->AddExtensionHandler(APlayerController::StaticClass(), AddMappingContextDelegate);

				ExtensionRequestHandles.Add(ExtensionRequestHandle);
#else
				const UMGCGameFrameworkExtensionManager::FExtensionHandlerDelegate AddMappingContextDelegate = UMGCGameFrameworkExtensionManager::FExtensionHandlerDelegate::CreateUObject(
					this,
					&UMGCGameFeatureAction_AddInputMappingContext::HandleControllerExtension
				);
				const TSharedPtr<FMGCComponentRequestHandle> ExtensionRequestHandle = ExtensionManager->AddExtensionHandler(APlayerController::StaticClass(), AddMappingContextDelegate);

				// ComponentRequests.Add(ExtensionRequestHandle);
				ExtensionsRequests.Add(ExtensionRequestHandle);
#endif
			}
		}
	}
}

void UMGCGameFeatureAction_AddInputMappingContext::Reset()
{
	ExtensionRequestHandles.Empty();

#if ENGINE_MAJOR_VERSION == 5
	while (!ControllersAddedTo.IsEmpty())
#else
	while (ControllersAddedTo.Num() != 0)
#endif
	{
		TWeakObjectPtr<APlayerController> ControllerPtr = ControllersAddedTo.Top();
		if (ControllerPtr.IsValid())
		{
			RemoveInputMapping(ControllerPtr.Get());
		}
		else
		{
			ControllersAddedTo.Pop();
		}
	}
}

void UMGCGameFeatureAction_AddInputMappingContext::HandleControllerExtension(AActor* Actor, const FName EventName)
{
	APlayerController* PC = CastChecked<APlayerController>(Actor);

#if ENGINE_MAJOR_VERSION == 5
	if (EventName == UGameFrameworkComponentManager::NAME_ExtensionRemoved || EventName == UGameFrameworkComponentManager::NAME_ReceiverRemoved)
	{
		RemoveInputMapping(PC);
	}
	else if (EventName == UGameFrameworkComponentManager::NAME_ExtensionAdded || EventName == UGameFrameworkComponentManager::NAME_GameActorReady)
	{
		AddInputMappingForPlayer(PC->GetLocalPlayer());
	}
#else
	if (EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionRemoved || EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ReceiverRemoved)
	{
		MGC_LOG(Log, TEXT("HandleControllerExtension remove '%s'. input mapping will be removed."), *Actor->GetPathName());
		RemoveInputMapping(PC);
	}
	else if (EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionAdded || EventName == UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady)
	{
		MGC_LOG(Log, TEXT("HandleControllerExtension add '%s'. input mapping will be added."), *Actor->GetPathName());
		AddInputMappingForPlayer(PC->GetLocalPlayer());
	}
#endif
}

void UMGCGameFeatureAction_AddInputMappingContext::AddInputMappingForPlayer(UPlayer* Player)
{
	if (ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (!InputMapping.IsNull())
			{
				InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), Priority);
			}
		}
		else
		{
			MGC_LOG(Error, TEXT("Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file."));
		}
	}
}

void UMGCGameFeatureAction_AddInputMappingContext::RemoveInputMapping(APlayerController* PlayerController)
{
	if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			InputSystem->RemoveMappingContext(InputMapping.Get());
		}
	}

	ControllersAddedTo.Remove(PlayerController);
}

void UMGCGameFeatureAction_AddInputMappingContext::HandleGameInstanceStart(UGameInstance* GameInstance)
{
	if (FWorldContext* WorldContext = GameInstance->GetWorldContext())
	{
		AddToWorld(*WorldContext);
	}
}

#undef LOCTEXT_NAMESPACE
