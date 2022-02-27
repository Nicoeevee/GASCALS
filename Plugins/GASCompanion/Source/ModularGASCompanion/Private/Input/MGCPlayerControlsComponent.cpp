// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Input/MGCPlayerControlsComponent.h"

#include "EnhancedInputSubsystems.h"
#include "ModularGASCompanionLog.h"
#include "ModularGameplayActors/GSCALSModCharacter.h"
#include "ModularGameplayActors/ModularCharacter.h"
#include "ModularGameplayActors/ModularPawn.h"
#include "ModularGameplayActors/ModularPlayerStateCharacter.h"

void UMGCPlayerControlsComponent::OnRegister()
{
	Super::OnRegister();

	UWorld* World = GetWorld();
	APawn* MyOwner = GetPawn<APawn>();

	if (ensure(MyOwner) && World->IsGameWorld())
	{
#if ENGINE_MAJOR_VERSION == 5
		MyOwner->ReceiveRestartedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnPawnRestarted);
		MyOwner->ReceiveControllerChangedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnControllerChanged);
#else
		// Note: ue5 adds these two delegates
		//
		// ReceiveRestartedDelegate triggered from Controller OnPossess
		// ReceiveControllerChangedDelegate triggered from Pawn OnRep_Controller / PossessedBy / UnPossessed
		//
		// So we need to account for those delegates and manually handle them (ue5 handles this all in a single place in APawn),
		// and need to check for three different Pawns: ModularPawn, ModularCharacter and ModularPlayerState character
		//
		// One possible improvements would be to rely on interface for those three, but I believe interfaces doesn't play very nicely with
		// UPROPERTY that those those delegates are

		if (AModularPawn* ModularPawn = GetPawn<AModularPawn>())
		{
			ModularPawn->ReceivePawnRestartedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnPawnRestarted);
			ModularPawn->ReceivePawnControllerChangedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnControllerChanged);
		}
		else if (AModularCharacter* ModularCharacter = GetPawn<AModularCharacter>())
		{
			ModularCharacter->ReceiveCharacterRestartedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnPawnRestarted);
			ModularCharacter->ReceiveCharacterControllerChangedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnControllerChanged);
		}
		else if (AGSCALSModCharacter* ALSModularCharacter = GetPawn<AGSCALSModCharacter>())
		{
			ALSModularCharacter->ReceiveCharacterRestartedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnPawnRestarted);
			ALSModularCharacter->ReceiveCharacterControllerChangedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnControllerChanged);
		}
		else if (AModularPlayerStateCharacter* ModularPlayerStateCharacter = GetPawn<AModularPlayerStateCharacter>())
		{
			ModularPlayerStateCharacter->ReceiveCharacterRestartedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnPawnRestarted);
			ModularPlayerStateCharacter->ReceiveCharacterControllerChangedDelegate.AddDynamic(this, &UMGCPlayerControlsComponent::OnControllerChanged);
		}
#endif

		// If our pawn has an input component we were added after restart
		if (MyOwner->InputComponent)
		{
			OnPawnRestarted(MyOwner);
		}
	}
}

void UMGCPlayerControlsComponent::OnUnregister()
{
	UWorld* World = GetWorld();
	if (World && World->IsGameWorld())
	{
		ReleaseInputComponent();

#if ENGINE_MAJOR_VERSION == 5
		APawn* MyOwner = GetPawn<APawn>();
		if (MyOwner)
		{
			MyOwner->ReceiveRestartedDelegate.RemoveAll(this);
			MyOwner->ReceiveControllerChangedDelegate.RemoveAll(this);
		}
#else
		// Note: ue5 adds these two delegates
		//
		// ReceiveRestartedDelegate triggered from Controller OnPossess
		// ReceiveControllerChangedDelegate triggered from Pawn OnRep_Controller / PossessedBy / UnPossessed
		//
		// So we need to account for those delegates and manually handle them (ue5 handles this all in a single place in APawn),
		// and need to check for three different Pawns: ModularPawn, ModularCharacter and ModularPlayerState character
		//
		// One possible improvements would be to rely on interface for those three, but I believe interfaces doesn't play very nicely with
		// UPROPERTY that those those delegates are

		if (AModularPawn* ModularPawn = GetPawn<AModularPawn>())
		{
			ModularPawn->ReceivePawnRestartedDelegate.RemoveAll(this);
			ModularPawn->ReceivePawnControllerChangedDelegate.RemoveAll(this);
		}
		else if (AModularCharacter* ModularCharacter = GetPawn<AModularCharacter>())
		{
			ModularCharacter->ReceiveCharacterRestartedDelegate.RemoveAll(this);
			ModularCharacter->ReceiveCharacterControllerChangedDelegate.RemoveAll(this);
		}
		else if (AGSCALSModCharacter* ALSModularCharacter = GetPawn<AGSCALSModCharacter>())
		{
			ALSModularCharacter->ReceiveCharacterRestartedDelegate.RemoveAll(this);
			ALSModularCharacter->ReceiveCharacterControllerChangedDelegate.RemoveAll(this);
		}
		else if (AModularPlayerStateCharacter* ModularPlayerStateCharacter = GetPawn<AModularPlayerStateCharacter>())
		{
			ModularPlayerStateCharacter->ReceiveCharacterRestartedDelegate.RemoveAll(this);
			ModularPlayerStateCharacter->ReceiveCharacterControllerChangedDelegate.RemoveAll(this);
		}
#endif
	}

	Super::OnUnregister();
}

void UMGCPlayerControlsComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
}

void UMGCPlayerControlsComponent::TeardownPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
}

void UMGCPlayerControlsComponent::OnPawnRestarted(APawn* Pawn)
{
	MGC_LOG(Verbose, TEXT("UMGCPlayerControlsComponent::OnPawnRestarted Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"))
	if (ensure(Pawn && Pawn == GetOwner()) && Pawn->InputComponent)
	{
		ReleaseInputComponent();

		if (Pawn->InputComponent)
		{
			SetupInputComponent(Pawn);
		}
	}
}

void UMGCPlayerControlsComponent::OnControllerChanged(APawn* Pawn, AController* OldController, AController* NewController)
{
	MGC_LOG(Verbose, TEXT("UMGCPlayerControlsComponent::OnControllerChanged Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"))
	// Only handle releasing, restart is a better time to handle binding
	if (ensure(Pawn && Pawn == GetOwner()) && OldController)
	{
		ReleaseInputComponent(OldController);
	}
}

void UMGCPlayerControlsComponent::SetupInputComponent(APawn* Pawn)
{
	InputComponent = CastChecked<UEnhancedInputComponent>(Pawn->InputComponent);

	if (ensureMsgf(InputComponent, TEXT("Project must use EnhancedInputComponent to support PlayerControlsComponent")))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem();
		check(Subsystem);

		if (InputMappingContext)
		{
			Subsystem->AddMappingContext(InputMappingContext, InputPriority);
		}

		MGC_LOG(Verbose, TEXT("UMGCPlayerControlsComponent::SetupInputComponent Pawn: %s"), Pawn ? *Pawn->GetName() : TEXT("NONE"))
		SetupPlayerControls(InputComponent);
	}
}

void UMGCPlayerControlsComponent::ReleaseInputComponent(AController* OldController)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = GetEnhancedInputSubsystem(OldController);
	if (Subsystem && InputComponent)
	{
		TeardownPlayerControls(InputComponent);

		if (InputMappingContext)
		{
			Subsystem->RemoveMappingContext(InputMappingContext);
		}
	}
	InputComponent = nullptr;
}

UEnhancedInputLocalPlayerSubsystem* UMGCPlayerControlsComponent::GetEnhancedInputSubsystem(AController* OldController) const
{
	const APlayerController* PC = GetController<APlayerController>();
	if (!PC)
	{
		PC = Cast<APlayerController>(OldController);
		if (!PC)
		{
			return nullptr;
		}
	}

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return nullptr;
	}

	return LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
}
