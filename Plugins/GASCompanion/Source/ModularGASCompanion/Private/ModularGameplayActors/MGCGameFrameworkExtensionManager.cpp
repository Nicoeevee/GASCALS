// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"

#include "EngineUtils.h"

FName UMGCGameFrameworkExtensionManager::MGC_NAME_ReceiverAdded = FName("ReceiverAdded");
FName UMGCGameFrameworkExtensionManager::MGC_NAME_ReceiverRemoved = FName("ReceiverRemoved");
FName UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionAdded = FName("ExtensionAdded");
FName UMGCGameFrameworkExtensionManager::MGC_NAME_ExtensionRemoved = FName("ExtensionRemoved");
FName UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady = FName("GameActorReady");

FMGCComponentRequestHandle::~FMGCComponentRequestHandle()
{
	UMGCGameFrameworkExtensionManager* LocalManager = OwningManager.Get();
	if (LocalManager)
	{
		if (ExtensionHandle.IsValid())
		{
			LocalManager->RemoveExtensionHandler(ReceiverClass, ExtensionHandle);
		}
	}
}

bool FMGCComponentRequestHandle::IsValid() const
{
	return OwningManager.IsValid();
}

void UMGCGameFrameworkExtensionManager::AddGameFrameworkComponentReceiver(AActor* Receiver, const bool bAddOnlyInGameWorlds)
{
	if (Receiver != nullptr)
	{
		UGameInstance* GameInstance = nullptr;
		if (bAddOnlyInGameWorlds)
		{
			UWorld* ReceiverWorld = Receiver->GetWorld();
			if ((ReceiverWorld != nullptr) && ReceiverWorld->IsGameWorld() && !ReceiverWorld->IsPreviewWorld())
			{
				GameInstance = ReceiverWorld->GetGameInstance();
			}
		}
		else
		{
			GameInstance = Receiver->GetGameInstance();
		}

		if (GameInstance)
		{
			if (UGameFrameworkComponentManager* GFCM = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(GameInstance))
			{
				GFCM->AddReceiver(Receiver, bAddOnlyInGameWorlds);
			}

			if (UMGCGameFrameworkExtensionManager* ExtensionManager = UGameInstance::GetSubsystem<UMGCGameFrameworkExtensionManager>(GameInstance))
			{
				ExtensionManager->AddReceiverInternal(Receiver);
			}
		}
	}
}

void UMGCGameFrameworkExtensionManager::RemoveGameFrameworkComponentReceiver(AActor* Receiver)
{
	if (Receiver != nullptr)
	{
		if (UGameFrameworkComponentManager* GFCM = UGameInstance::GetSubsystem<UGameFrameworkComponentManager>(Receiver->GetGameInstance()))
		{
			GFCM->RemoveReceiver(Receiver);
		}

		if (UMGCGameFrameworkExtensionManager* ExtensionManager = UGameInstance::GetSubsystem<UMGCGameFrameworkExtensionManager>(Receiver->GetGameInstance()))
		{
			ExtensionManager->RemoveReceiverInternal(Receiver);
		}
	}
}

TSharedPtr<FMGCComponentRequestHandle> UMGCGameFrameworkExtensionManager::AddExtensionHandler(const TSoftClassPtr<AActor>& ReceiverClass, const FExtensionHandlerDelegate ExtensionHandler)
{
	// You must have a target and bound handler. The target cannot be AActor, that is too broad and would be bad for performance.
	if (!ensure(!ReceiverClass.IsNull()) || !ensure(ExtensionHandler.IsBound()) || !ensure(ReceiverClass.ToString() != TEXT("/Script/Engine.Actor")))
	{
		return nullptr;
	}

	const FMGCComponentRequestReceiverClassPath ReceiverClassPath(ReceiverClass);
	FExtensionHandlerEvent& HandlerEvent = ReceiverClassToEventMap.FindOrAdd(ReceiverClassPath);

	// This is a fake multicast delegate using a map
	FDelegateHandle DelegateHandle(FDelegateHandle::EGenerateNewHandleType::GenerateNewHandle);
	HandlerEvent.Add(DelegateHandle, ExtensionHandler);

	if (UClass* ReceiverClassPtr = ReceiverClass.Get())
	{
		UGameInstance* LocalGameInstance = GetGameInstance();
		if (ensure(LocalGameInstance))
		{
			UWorld* LocalWorld = LocalGameInstance->GetWorld();
			if (ensure(LocalWorld))
			{
				for (TActorIterator<AActor> ActorIt(LocalWorld, ReceiverClassPtr); ActorIt; ++ActorIt)
				{
					if (ActorIt->IsActorInitialized())
					{
						ExtensionHandler.Execute(*ActorIt, MGC_NAME_ExtensionAdded);
					}
				}
			}
		}
	}
	else
	{
		// Actor class is not in memory, there will be no actor instances
	}

	return MakeShared<FMGCComponentRequestHandle>(this, ReceiverClass, DelegateHandle);
}

void UMGCGameFrameworkExtensionManager::SendGameFrameworkComponentExtensionEvent(AActor* Receiver, const FName& EventName, const bool bOnlyInGameWorlds)
{
	if (Receiver != nullptr)
	{
		if (bOnlyInGameWorlds)
		{
			UWorld* ReceiverWorld = Receiver->GetWorld();
			if ((ReceiverWorld != nullptr) && ReceiverWorld->IsGameWorld() && !ReceiverWorld->IsPreviewWorld())
			{
				if (UMGCGameFrameworkExtensionManager* GFCM = UGameInstance::GetSubsystem<UMGCGameFrameworkExtensionManager>(ReceiverWorld->GetGameInstance()))
				{
					GFCM->SendExtensionEvent(Receiver, EventName);
				}
			}
		}
		else
		{
			if (UMGCGameFrameworkExtensionManager* GFCM = UGameInstance::GetSubsystem<UMGCGameFrameworkExtensionManager>(Receiver->GetGameInstance()))
			{
				GFCM->SendExtensionEvent(Receiver, EventName);
			}
		}
	}
}

void UMGCGameFrameworkExtensionManager::SendExtensionEvent(AActor* Receiver, const FName EventName, const bool bOnlyInGameWorlds)
{
	if (Receiver != nullptr)
	{
		if (bOnlyInGameWorlds)
		{
			UWorld* ReceiverWorld = Receiver->GetWorld();
			if ((ReceiverWorld == nullptr) || !ReceiverWorld->IsGameWorld() || ReceiverWorld->IsPreviewWorld())
			{
				return;
			}
		}

		SendExtensionEventInternal(Receiver, EventName);
	}
}

void UMGCGameFrameworkExtensionManager::SendExtensionEventInternal(AActor* Receiver, const FName& EventName)
{
	for (UClass* Class = Receiver->GetClass(); Class && Class != AActor::StaticClass(); Class = Class->GetSuperClass())
	{
		FMGCComponentRequestReceiverClassPath ReceiverClassPath(Class);
		if (FExtensionHandlerEvent* HandlerEvent = ReceiverClassToEventMap.Find(ReceiverClassPath))
		{
			for (const TPair<FDelegateHandle, FExtensionHandlerDelegate>& Pair : *HandlerEvent)
			{
				Pair.Value.Execute(Receiver, EventName);
			}
		}
	}
}

void UMGCGameFrameworkExtensionManager::AddReceiverInternal(AActor* Receiver)
{
	for (UClass* Class = Receiver->GetClass(); Class && Class != AActor::StaticClass(); Class = Class->GetSuperClass())
	{
		FMGCComponentRequestReceiverClassPath ReceiverClassPath(Class);

		if (FExtensionHandlerEvent* HandlerEvent = ReceiverClassToEventMap.Find(ReceiverClassPath))
		{
			for (const TPair<FDelegateHandle, FExtensionHandlerDelegate>& Pair : *HandlerEvent)
			{
				Pair.Value.Execute(Receiver, MGC_NAME_ReceiverAdded);
			}
		}
	}
}

void UMGCGameFrameworkExtensionManager::RemoveReceiverInternal(AActor* Receiver)
{
	SendExtensionEventInternal(Receiver, MGC_NAME_ReceiverRemoved);
}

void UMGCGameFrameworkExtensionManager::RemoveExtensionHandler(const TSoftClassPtr<AActor>& ReceiverClass, FDelegateHandle DelegateHandle)
{
	const FMGCComponentRequestReceiverClassPath ReceiverClassPath(ReceiverClass);

	if (FExtensionHandlerEvent* HandlerEvent = ReceiverClassToEventMap.Find(ReceiverClassPath))
	{
		FExtensionHandlerDelegate* HandlerDelegate = HandlerEvent->Find(DelegateHandle);
		if (ensure(HandlerDelegate))
		{
			// Call it once on unregister
			if (UClass* ReceiverClassPtr = ReceiverClass.Get())
			{
				UGameInstance* LocalGameInstance = GetGameInstance();
				if (ensure(LocalGameInstance))
				{
					UWorld* LocalWorld = LocalGameInstance->GetWorld();
					if (ensure(LocalWorld))
					{
						for (TActorIterator<AActor> ActorIt(LocalWorld, ReceiverClassPtr); ActorIt; ++ActorIt)
						{
							if (ActorIt->IsActorInitialized())
							{
								HandlerDelegate->Execute(*ActorIt, MGC_NAME_ExtensionRemoved);
							}
						}
					}
				}
			}
			else
			{
				// Actor class is not in memory, there will be no actor instances
			}

			HandlerEvent->Remove(DelegateHandle);

			if (HandlerEvent->Num() == 0)
			{
				ReceiverClassToEventMap.Remove(ReceiverClassPath);
			}
		}
	}
}
