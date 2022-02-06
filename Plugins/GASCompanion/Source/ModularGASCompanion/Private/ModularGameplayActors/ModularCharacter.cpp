// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "ModularGameplayActors/ModularCharacter.h"

#include "Abilities/MGCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"
#include "ModularGASCompanionLog.h"

AModularCharacter::AModularCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UMGCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Replication Mode is set in PostInitProperties to allow users to change the default value from BP
}

UAbilitySystemComponent* AModularCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AModularCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::AddGameFrameworkComponentReceiver(this);
#endif
}

void AModularCharacter::BeginPlay()
{

#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
#else
	// One of the difference with 5.0 is that component requests are added here only if ActorHasBegunPlay, whereas in 5.0 it has been changed slighly
	// to check if ActorHasBeenInitialized. So to workaround that, we call Super::BeginPlay before
	Super::BeginPlay();
	UMGCGameFrameworkExtensionManager::SendGameFrameworkComponentExtensionEvent(this, UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady);
#endif
}

void AModularCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::RemoveGameFrameworkComponentReceiver(this);
#endif

	Super::EndPlay(EndPlayReason);
}

void AModularCharacter::PostInitProperties()
{
	Super::PostInitProperties();
	if (AbilitySystemComponent)
	{
		MGC_LOG(Verbose, TEXT("AModularCharacter::PostInitProperties for %s - Setting up ASC Replication Mode to: %d"), *GetName(), ReplicationMode)
		AbilitySystemComponent->SetReplicationMode(ReplicationMode);
	}
}

void AModularCharacter::Restart()
{
	Super::Restart();
	NotifyCharacterRestarted();
}

void AModularCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	NotifyCharacterRestarted();
}

void AModularCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	bool bNotifyControllerChange = (Controller == nullptr);
	if ((Controller != nullptr) && (Controller->GetPawn() == nullptr))
	{
		bNotifyControllerChange = true;
	}

	if (bNotifyControllerChange)
	{
		NotifyCharacterControllerChanged();
	}
}

void AModularCharacter::PossessedBy(AController* NewController)
{
	AController* OldController = Controller;
	Super::PossessedBy(NewController);

	// dispatch controller changed Blueprint event if necessary, along with ReceiveCharacterControllerChangedDelegate
	if (OldController != NewController)
	{
		NotifyCharacterControllerChanged();
	}
}

void AModularCharacter::UnPossessed()
{
	Super::UnPossessed();

	// TODO: ConsumeMovementInputVector() is called before we notify controller changed, whereas in 5.0 NotifyControllerChanged() happens just before
	NotifyCharacterControllerChanged();
}

void AModularCharacter::NotifyCharacterRestarted()
{
	ReceiveCharacterRestarted();
	ReceiveCharacterRestartedDelegate.Broadcast(this);
}

void AModularCharacter::NotifyCharacterControllerChanged()
{
	ReceiveCharacterControllerChanged(PreviousCharacterController, Controller);
	ReceiveCharacterControllerChangedDelegate.Broadcast(this, PreviousCharacterController, Controller);

	// Update the cached controller
	PreviousCharacterController = Controller;
}
