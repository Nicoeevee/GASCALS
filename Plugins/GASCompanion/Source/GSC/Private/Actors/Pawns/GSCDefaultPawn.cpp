// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Actors/Pawns/GSCDefaultPawn.h"
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "Components/GSCCoreComponent.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "GSCLog.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GSCAbilitySystemComponent.h"

AGSCDefaultPawn::AGSCDefaultPawn()
{
	// Setup sensible defaults
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	NetPriority = 4.0f;

	// Create Components
	AbilitySystemComponent = CreateDefaultSubobject<UGSCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	GSCCoreComponent = CreateDefaultSubobject<UGSCCoreComponent>("GSCCoreComponent");
}

void AGSCDefaultPawn::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	SetupAttributeSets();
}

UAbilitySystemComponent* AGSCDefaultPawn::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGSCCoreComponent* AGSCDefaultPawn::GetCompanionCoreComponent() const
{
	return GSCCoreComponent;
}

UGSCComboManagerComponent* AGSCDefaultPawn::GetComboManagerComponent() const
{
	return nullptr;
}

UGSCAbilityQueueComponent* AGSCDefaultPawn::GetAbilityQueueComponent() const
{
	return nullptr;
}

void AGSCDefaultPawn::BeginPlay()
{
    Super::BeginPlay();

	GSCCoreComponent->SetupAbilityActor(this, this);
}

void AGSCDefaultPawn::SetupAttributeSets()
{
	if (!AbilitySystemComponent)
	{
		GSC_LOG(Error, TEXT("AGSCDefaultPawn: ASC null, can't setup attribute sets for %s"), *GetName())
		return;
	}

	// Create the attribute sets from DeveloperSettings Configuration
	const UGSCDeveloperSettings* Settings = GetDefault<UGSCDeveloperSettings>();
	TArray<TSubclassOf<UGSCAttributeSetBase>> PawnsAttributeSets = Settings->PawnsAttributeSets;

	// Add AttributeSet if not prevented by config
	if (Settings->bWantsDefaultAttributePawns)
	{
		PawnsAttributeSets.Add(UGSCAttributeSet::StaticClass());
	}

	for (const TSubclassOf<UAttributeSet> AttributeSetClass : PawnsAttributeSets)
	{
		if (!AttributeSetClass)
		{
			GSC_LOG(Warning, TEXT("AGSCDefaultPawn: Some configured AttributeSets for Pawn Characters seem to be missing. Please check Project's Settings for any AttributeSet with a NONE value."))
			continue;
		}

		// Only create AttributeSet if not already done for this AttributeSet class
		if (!RegisteredAttributeSetNames.Contains(AttributeSetClass->GetName()))
		{
			GSC_VLOG(this, Log, TEXT("AGSCDefaultPawn: Adding AttributeSet %s to Pawn."), *AttributeSetClass->GetName())
			const UAttributeSet* AttributeSet = AbilitySystemComponent->InitStats(AttributeSetClass, nullptr);
			AttributeSets.Add(AttributeSet);
			RegisteredAttributeSetNames.Add(AttributeSetClass->GetName());
		}
		else
		{
			GSC_LOG(Warning, TEXT("AGSCDefaultPawn: AttributeSet %s seem to have duplicates. Please check Project's Settings for any duplicated AttributeSet."), *AttributeSetClass->GetName())
		}
	}
}
