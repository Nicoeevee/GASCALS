// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "ModularGameplayActors/ModularPlayerStateCharacter.h"

#include "Abilities/MGCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"
#include "ModularGASCompanionLog.h"
#include "ModularGameplayActors/ModularPlayerState.h"

AModularPlayerStateCharacter::AModularPlayerStateCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MGC_LOG(Verbose, TEXT("AModularPlayerStateCharacter ctor for %s"), *GetName())
}

UAbilitySystemComponent* AModularPlayerStateCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent.IsValid() ? AbilitySystemComponent.Get() : nullptr;
}

void AModularPlayerStateCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::AddGameFrameworkComponentReceiver(this);
#endif
}

void AModularPlayerStateCharacter::BeginPlay()
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

void AModularPlayerStateCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::RemoveGameFrameworkComponentReceiver(this);
#endif

	Super::EndPlay(EndPlayReason);
}

void AModularPlayerStateCharacter::Restart()
{
	Super::Restart();
	NotifyCharacterRestarted();
}

void AModularPlayerStateCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	NotifyCharacterRestarted();
}

void AModularPlayerStateCharacter::OnRep_Controller()
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

void AModularPlayerStateCharacter::PossessedBy(AController* NewController)
{
	AController* OldController = Controller;
	Super::PossessedBy(NewController);

	// dispatch controller changed Blueprint event if necessary, along with ReceiveCharacterControllerChangedDelegate
	if (OldController != NewController)
	{
		NotifyCharacterControllerChanged();
	}

	// For Player State ASC Pawns, initialize ASC on server in PossessedBy
	AModularPlayerState* PS = GetPlayerState<AModularPlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}
}

void AModularPlayerStateCharacter::UnPossessed()
{
	Super::UnPossessed();

	// TODO: ConsumeMovementInputVector() is called before we notify controller changed, whereas in 5.0 NotifyControllerChanged() happens just before
	NotifyCharacterControllerChanged();
}

void AModularPlayerStateCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// For Player State ASC Pawns, initialize ASC on clients in OnRep_PlayerState
	AModularPlayerState* PS = GetPlayerState<AModularPlayerState>();
	if (PS)
	{
		AbilitySystemComponent = PS->GetAbilitySystemComponent();
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}
}

void AModularPlayerStateCharacter::NotifyCharacterRestarted()
{
	ReceiveCharacterRestarted();
	ReceiveCharacterRestartedDelegate.Broadcast(this);
}

void AModularPlayerStateCharacter::NotifyCharacterControllerChanged()
{
	ReceiveCharacterControllerChanged(PreviousCharacterController, Controller);
	ReceiveCharacterControllerChangedDelegate.Broadcast(this, PreviousCharacterController, Controller);

	// Update the cached controller
	PreviousCharacterController = Controller;
}
