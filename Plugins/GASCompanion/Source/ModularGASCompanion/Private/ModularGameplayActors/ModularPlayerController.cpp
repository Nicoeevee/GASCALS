// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "ModularGameplayActors/ModularPlayerController.h"

#include "Components/GameFrameworkComponentManager.h"
#include "Components/ControllerComponent.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"
#include "ModularGameplayActors/ModularCharacter.h"
#include "ModularGameplayActors/ModularPawn.h"
#include "ModularGameplayActors/ModularPlayerStateCharacter.h"

void AModularPlayerController::PreInitializeComponents()
{
	Super::PreInitializeComponents();

#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::AddGameFrameworkComponentReceiver(this);
#endif
}

void AModularPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::RemoveGameFrameworkComponentReceiver(this);
#endif

	Super::EndPlay(EndPlayReason);
}

void AModularPlayerController::ReceivedPlayer()
{
	// Player controllers always get assigned a player and can't do much until then
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
#else
	UMGCGameFrameworkExtensionManager::SendGameFrameworkComponentExtensionEvent(this, UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady);
#endif

	Super::ReceivedPlayer();

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->ReceivedPlayer();
	}
}

void AModularPlayerController::PlayerTick(const float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	TArray<UControllerComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UControllerComponent* Component : ModularComponents)
	{
		Component->PlayerTick(DeltaTime);
	}
}

void AModularPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Local PCs will have the Restart() triggered right away in ClientRestart (via PawnClientRestart()), but the server should call Restart() locally for remote PCs.
	// We're really just trying to avoid calling Restart() multiple times.
	if (!IsLocalPlayerController())
	{
		if (AModularPawn* ModularPawn = Cast<AModularPawn>(InPawn))
		{
			ModularPawn->NotifyPawnRestarted();
		}
		else if (AModularCharacter* ModularCharacter = Cast<AModularCharacter>(InPawn))
		{
			ModularCharacter->NotifyCharacterRestarted();
		}
		else if (AModularPlayerStateCharacter* ModularPlayerStateCharacter = Cast<AModularPlayerStateCharacter>(InPawn))
		{
			ModularPlayerStateCharacter->NotifyCharacterRestarted();
		}
	}
}
