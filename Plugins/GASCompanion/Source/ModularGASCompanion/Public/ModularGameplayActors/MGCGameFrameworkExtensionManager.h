// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCGameFrameworkExtensionManager.generated.h"

/**
 * FMGCComponentRequestHandle
 *
 * Specifically done for 4.27 to supplement UGameFrameworkComponentManager with Extension Events available in ue5, but not backported to 4.27 yet.
 *
 * Only difference with FComponentRequestHandle in ModularGameplay module it the addition of ExtensionHandle delegate. (and doesn't handle Components)
 *
 * A handle for a request to put components or call a delegate for an extensible actor class.
 * When this handle is destroyed, it will remove the associated request from the system.
 */
struct FMGCComponentRequestHandle
{
	FMGCComponentRequestHandle(const TWeakObjectPtr<UMGCGameFrameworkExtensionManager>& InOwningManager, const TSoftClassPtr<AActor>& InReceiverClass, FDelegateHandle InExtensionHandle)
		: OwningManager(InOwningManager)
		, ReceiverClass(InReceiverClass)
		, ExtensionHandle(InExtensionHandle)
	{}

	MODULARGASCOMPANION_API ~FMGCComponentRequestHandle();

	/** Returns true if the manager that this request is for still exists */
	MODULARGASCOMPANION_API bool IsValid() const;

private:
	/** The manager that this request was for */
	TWeakObjectPtr<UMGCGameFrameworkExtensionManager> OwningManager;

	/** The class of actor to put components */
	TSoftClassPtr<AActor> ReceiverClass;

	/** A handle to an extension delegate to run */
	FDelegateHandle ExtensionHandle;
};

/**
 * MGCGameFrameworkExtensionManager
 *
 * Specifically done for 4.27 to supplement UGameFrameworkComponentManager with Extension Events available in ue5, but not backported to 4.27 yet.
 *
 * Adds support to submit delegate handlers to listen for actors of a given class. Those handlers will automatically run when actors of a given class
 * or registered as receivers or game events are sent.
 */
UCLASS(MinimalAPI)
class UMGCGameFrameworkExtensionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	/** Using a fake multicast delegate so order can be kept consistent */
	DECLARE_DELEGATE_TwoParams(FExtensionHandlerDelegateInternal, AActor*, FName);
	using FExtensionHandlerEvent = TMap<FDelegateHandle, FExtensionHandlerDelegateInternal>;

public:

	/** Delegate types for extension handlers */
	using FExtensionHandlerDelegate = FExtensionHandlerDelegateInternal;

	//////////////////////////////////////////////////////////////////////////////////////////////
	// The extension system allows registering for arbitrary event callbacks on receiver actors.
	// These are the default events but games can define, send, and listen for their own.

	/** AddReceiver was called for a registered class and components were added, called early in initialization */
	static MODULARGASCOMPANION_API FName MGC_NAME_ReceiverAdded;

	/** RemoveReceiver was called for a registered class and components were removed, normally called from EndPlay */
	static MODULARGASCOMPANION_API FName MGC_NAME_ReceiverRemoved;

	/** A new extension handler was added */
	static MODULARGASCOMPANION_API FName MGC_NAME_ExtensionAdded;

	/** An extension handler was removed by a freed request handle */
	static MODULARGASCOMPANION_API FName MGC_NAME_ExtensionRemoved;

	/**
	 * Game-specific event indicating an actor is mostly initialized and ready for extension.
	 * All extensible games are expected to send this event at the appropriate actor-specific point, as plugins may be listening for it.
	 */
	static MODULARGASCOMPANION_API FName MGC_NAME_GameActorReady;

	/**
	 * Version of GameFrameworkComponentManager RemoveGameFrameworkComponentReceiver that handles execution of FExtensionHandlerEvent with MGC_NAME_ReceiverAdded for 4.27,
	 * since this static method is not exposed in 4.27.
	 *
	 * Adds an actor as a receiver for components (automatically finding the manager for the actor's  game instance). If it passes the actorclass filter on requests it will get the components.
	 */
	static MODULARGASCOMPANION_API void AddGameFrameworkComponentReceiver(AActor* Receiver, bool bAddOnlyInGameWorlds = true);

	/**
	 * Version of GameFrameworkComponentManager RemoveGameFrameworkComponentReceiver that handles execution of FExtensionHandlerEvent with MGC_NAME_ReceiverRemoved for 4.27,
	 * since this static method is not exposed in 4.27.
	 *
	 * Removes an actor as a receiver for components (automatically finding the manager for the actor's game instance).
	 */
	static MODULARGASCOMPANION_API void RemoveGameFrameworkComponentReceiver(AActor* Receiver);

	/** Adds an extension handler to run on actors of the given class. Returns a handle that will keep the handler "alive" until it is destructed, at which point the delegate is removed */
	MODULARGASCOMPANION_API TSharedPtr<FMGCComponentRequestHandle> AddExtensionHandler(const TSoftClassPtr<AActor>& ReceiverClass, FExtensionHandlerDelegate ExtensionHandler);

	/** Sends an arbitrary extension event that can be listened for by other systems */
	static MODULARGASCOMPANION_API void SendGameFrameworkComponentExtensionEvent(AActor* Receiver, const FName& EventName, bool bOnlyInGameWorlds = true);

	/** Sends an arbitrary extension event that can be listened for by other systems */
	UFUNCTION(BlueprintCallable, Category = "Modular GAS Companion|Gameplay", meta = (DefaultToSelf = "Receiver", AdvancedDisplay = 1))
	MODULARGASCOMPANION_API void SendExtensionEvent(AActor* Receiver, FName EventName, bool bOnlyInGameWorlds = true);

private:

	/** A list of FNames to represent an object path. Used for fast hashing and comparison of paths */
	struct FMGCComponentRequestReceiverClassPath
	{
		TArray<FName> Path;

		FMGCComponentRequestReceiverClassPath() {}

		FMGCComponentRequestReceiverClassPath(UClass* InClass)
		{
			check(InClass);
			for (UObject* Obj = InClass; Obj; Obj = Obj->GetOuter())
			{
				Path.Insert(Obj->GetFName(), 0);
			}
		}

		FMGCComponentRequestReceiverClassPath(const TSoftClassPtr<AActor>& InSoftClassPtr)
		{
			TArray<FString> StringPath;
			InSoftClassPtr.ToString().ParseIntoArray(StringPath, TEXT("."));
			Path.Reserve(StringPath.Num());
			for (const FString& StringPathElement : StringPath)
			{
				Path.Add(FName(*StringPathElement));
			}
		}

#if !UE_BUILD_SHIPPING
		FString ToDebugString() const
		{
			FString ReturnString;
			if (Path.Num() > 0)
			{
				ReturnString = Path[0].ToString();
				for (int32 PathIdx = 1; PathIdx < Path.Num(); ++PathIdx)
				{
					ReturnString += TEXT(".") + Path[PathIdx].ToString();
				}
			}

			return ReturnString;
		}
#endif // !UE_BUILD_SHIPPING

		bool operator==(const FMGCComponentRequestReceiverClassPath& Other) const
		{
			return Path == Other.Path;
		}

		friend FORCEINLINE uint32 GetTypeHash(const FMGCComponentRequestReceiverClassPath& Request)
		{
			uint32 ReturnHash = 0;
			for (const FName& PathElement : Request.Path)
			{
				ReturnHash ^= GetTypeHash(PathElement);
			}
			return ReturnHash;
		}
	};

	/** A map of actor classes to delegate handlers that should be executed for actors of that class. */
	TMap<FMGCComponentRequestReceiverClassPath, FExtensionHandlerEvent> ReceiverClassToEventMap;

	/** Called by FMGCComponentRequestHandle's destructor to remove a handler from the system. */
	void RemoveExtensionHandler(const TSoftClassPtr<AActor>& ReceiverClass, FDelegateHandle DelegateHandle);

	void SendExtensionEventInternal(AActor* Receiver, const FName& EventName);

	/** Version of GameFrameworkComponentManager AddReceiverInternal that handles execution of FExtensionHandlerEvent with MGC_NAME_ReceiverAdded for 4.27  */
	void AddReceiverInternal(AActor* Receiver);

	/** Version of GameFrameworkComponentManager RemoveReceiverInternal that handles execution of FExtensionHandlerEvent with MGC_NAME_ReceiverRemoved for 4.27  */
	void RemoveReceiverInternal(AActor* Receiver);

	friend struct FMGCComponentRequestHandle;
};
