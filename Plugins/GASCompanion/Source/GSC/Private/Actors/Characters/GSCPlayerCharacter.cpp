// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Actors/Characters/GSCPlayerCharacter.h"

#include "GSCLog.h"
#include "Abilities/GSCGameplayAbility.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCComboManagerComponent.h"
#include "Components/GSCCoreComponent.h"
#include "Player/GSCPlayerController.h"
#include "Player/GSCPlayerState.h"

AGSCPlayerCharacter::AGSCPlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void AGSCPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AGSCPlayerState* PS = GetPlayerState<AGSCPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		SetupAbilitySystemComponent(PS, true);
	}
}

void AGSCPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind player input to the AbilitySystemComponent. Also called in OnRep_PlayerState because of a potential race condition.
	BindASCInput();
}

void AGSCPlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AGSCPlayerState* PS = GetPlayerState<AGSCPlayerState>();
	if (PS)
	{
		SetupAbilitySystemComponent(PS, false);
	}
}

void AGSCPlayerCharacter::BindASCInput()
{
	if (bASCInputBound)
	{
		return;
	}

	if (AbilitySystemComponent.IsValid() && IsValid(InputComponent))
	{
		AbilitySystemComponent->BindAbilityActivationToInputComponent(
			InputComponent,
			FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			FString("CancelTarget"),
			FString("EGSCAbilityInputID"),
			static_cast<int32>(EGSCAbilityInputID::Confirm),
			static_cast<int32>(EGSCAbilityInputID::Cancel))
		);

		bASCInputBound = true;
	}
}

void AGSCPlayerCharacter::SetupAbilitySystemComponent(AGSCPlayerState* PS, const bool bShouldSetupAbilityActor)
{
	if (!PS)
	{
		GSC_LOG(Error, TEXT("AGSCPlayerCharacter::SetupAbilitySystemComponent Passed in PlayerState is null"))
		return;
	}

	AbilitySystemComponent = Cast<UAbilitySystemComponent>(PS->GetAbilitySystemComponent());
	GSCCoreComponent->SetupOwner();
	GSCAbilityQueueComponent->SetupOwner();
	GSCComboComponent->SetupOwner();

	// Update parent's reference to AttributeSets
	AttributeSets = PS->AttributeSets;

	if (bShouldSetupAbilityActor)
	{
		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initiating twice for heroes that have PlayerControllers.
		GSCCoreComponent->SetupAbilityActor(PS->GetAbilitySystemComponent(), PS, this);
	}
	else
	{
		// Partial Setup
		//
		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		// Bind player input to the AbilitySystemComponent. Also called in SetupPlayerInputComponent because of a potential race condition.
		BindASCInput();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that possession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		GSCCoreComponent->InitializeAttributes();
	}

	// UI init
	AGSCPlayerController* PC = Cast<AGSCPlayerController>(GetController());
	if (PC)
	{
		PC->CreateHUD();
	}
}

