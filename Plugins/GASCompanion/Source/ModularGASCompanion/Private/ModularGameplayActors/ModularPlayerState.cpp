// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "ModularGameplayActors/ModularPlayerState.h"

#include "ModularGASCompanionLog.h"
#include "Abilities/MGCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Components/PlayerStateComponent.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"

AModularPlayerState::AModularPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UMGCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Replication Mode is set in PostInitProperties to allow users to change the default value from BP

	// Set PlayerState's NetUpdateFrequency to sensible defaults.
	//
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 10.0f;
}

void AModularPlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();

#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::AddGameFrameworkComponentReceiver(this);
#endif
}

void AModularPlayerState::BeginPlay()
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
#else
	Super::BeginPlay();
	UMGCGameFrameworkExtensionManager::SendGameFrameworkComponentExtensionEvent(this, UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady);
#endif
}

void AModularPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::RemoveGameFrameworkComponentReceiver(this);
#endif

	Super::EndPlay(EndPlayReason);
}

void AModularPlayerState::PostInitProperties()
{
	Super::PostInitProperties();
	if (AbilitySystemComponent)
	{
		MGC_LOG(Verbose, TEXT("AModularPlayerState::PostInitProperties for %s - Setting up ASC Replication Mode to: %d"), *GetName(), ReplicationMode)
		AbilitySystemComponent->SetReplicationMode(ReplicationMode);
	}
}

void AModularPlayerState::Reset()
{
	Super::Reset();

	TArray<UPlayerStateComponent*> ModularComponents;
	GetComponents(ModularComponents);
	for (UPlayerStateComponent* Component : ModularComponents)
	{
		Component->Reset();
	}
}

UAbilitySystemComponent* AModularPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AModularPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::Reset();

	TArray<UPlayerStateComponent*> ModularComponents;
	GetComponents(ModularComponents);

	TArray<UPlayerStateComponent*> OtherModularComponents;
	PlayerState->GetComponents(OtherModularComponents);

	for (UPlayerStateComponent* Component : ModularComponents)
	{
		for (UPlayerStateComponent* OtherComponent : OtherModularComponents)
		{
			Component->CopyProperties(OtherComponent);
		}
	}
}
