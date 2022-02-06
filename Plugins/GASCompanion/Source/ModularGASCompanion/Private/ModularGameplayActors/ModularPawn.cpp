// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "ModularGameplayActors/ModularPawn.h"

#include "ModularGASCompanionLog.h"
#include "Abilities/MGCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"

AModularPawn::AModularPawn()
{
	AbilitySystemComponent = CreateDefaultSubobject<UMGCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Replication Mode is set in PostInitProperties to allow users to change the default value from BP
}

UAbilitySystemComponent* AModularPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AModularPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();

#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::AddGameFrameworkComponentReceiver(this);
#endif
}

void AModularPawn::BeginPlay()
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
#else
	Super::BeginPlay();
	UMGCGameFrameworkExtensionManager::SendGameFrameworkComponentExtensionEvent(this, UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady);
#endif
}

void AModularPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::RemoveGameFrameworkComponentReceiver(this);
#endif

	Super::EndPlay(EndPlayReason);
}

void AModularPawn::PostInitProperties()
{
	Super::PostInitProperties();
	if (AbilitySystemComponent)
	{
		MGC_LOG(Verbose, TEXT("AModularPawn::PostInitProperties for %s - Setting up ASC Replication Mode to: %d"), *GetName(), ReplicationMode)
		AbilitySystemComponent->SetReplicationMode(ReplicationMode);
	}
}

void AModularPawn::Restart()
{
	Super::Restart();
	NotifyPawnRestarted();
}

void AModularPawn::PawnClientRestart()
{
	Super::PawnClientRestart();
	NotifyPawnRestarted();
}

void AModularPawn::OnRep_Controller()
{
	Super::OnRep_Controller();

	bool bNotifyControllerChange = (Controller == nullptr);
	if ((Controller != nullptr) && (Controller->GetPawn() == nullptr))
	{
		bNotifyControllerChange = true;
	}

	if (bNotifyControllerChange)
	{
		NotifyPawnControllerChanged();
	}
}

void AModularPawn::PossessedBy(AController* NewController)
{
	AController* OldController = Controller;
	Super::PossessedBy(NewController);

	// dispatch controller changed Blueprint event if necessary, along with ReceiveCharacterControllerChangedDelegate
	if (OldController != NewController)
	{
		NotifyPawnControllerChanged();
	}
}

void AModularPawn::UnPossessed()
{
	Super::UnPossessed();
	NotifyPawnControllerChanged();
}

void AModularPawn::NotifyPawnRestarted()
{
	ReceivePawnRestarted();
	ReceivePawnRestartedDelegate.Broadcast(this);
}

void AModularPawn::NotifyPawnControllerChanged()
{
	ReceivePawnControllerChanged(PreviousCharacterController, Controller);
	ReceivePawnControllerChangedDelegate.Broadcast(this, PreviousCharacterController, Controller);

	// Update the cached controller
	PreviousCharacterController = Controller;
}
