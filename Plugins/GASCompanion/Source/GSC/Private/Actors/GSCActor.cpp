// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Actors/GSCActor.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "Components/GSCCoreComponent.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "GSCLog.h"

// Sets default values
AGSCActor::AGSCActor()
{
	// Setup sensible defaults
	PrimaryActorTick.bCanEverTick = true;

	// Explicitly set it to replicated (not done by default for actors, it's enabled on pawn)
	bReplicates = true;
	NetPriority = 4.0f;

	// Create components
	AbilitySystemComponent = CreateDefaultSubobject<UGSCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	GSCCoreComponent = CreateDefaultSubobject<UGSCCoreComponent>("GSCCoreComponent");

}

void AGSCActor::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	SetupAttributeSets();
}

UAbilitySystemComponent* AGSCActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UGSCCoreComponent* AGSCActor::GetCompanionCoreComponent() const
{
	return GSCCoreComponent;
}

UGSCComboManagerComponent* AGSCActor::GetComboManagerComponent() const
{
	return nullptr;
}

UGSCAbilityQueueComponent* AGSCActor::GetAbilityQueueComponent() const
{
	return nullptr;
}

// Called when the game starts or when spawned
void AGSCActor::BeginPlay()
{
    Super::BeginPlay();

	GSCCoreComponent->SetupAbilityActor(this, this);
}

void AGSCActor::SetupAttributeSets()
{
	if (!AbilitySystemComponent)
	{
		GSC_LOG(Error, TEXT("AGSCActor: ASC null, can't setup attribute sets for %s"), *GetName())
		return;
	}

	// Create the attribute sets from DeveloperSettings Configuration
	const UGSCDeveloperSettings* Settings = GetDefault<UGSCDeveloperSettings>();
	TArray<TSubclassOf<UGSCAttributeSetBase>> ActorsAttributeSets = Settings->ActorsAttributeSets;

	// Add AttributeSet if not prevented by config
	if (Settings->bWantsDefaultAttributeActors)
	{
		ActorsAttributeSets.Add(UGSCAttributeSet::StaticClass());
	}

	for (const TSubclassOf<UAttributeSet> AttributeSetClass : ActorsAttributeSets)
	{
		if (!AttributeSetClass)
		{
			GSC_LOG(Warning, TEXT("AGSCActor: Some configured AttributeSets for Pawn Characters seem to be missing. Please check Project's Settings for any AttributeSet with a NONE value."))
			continue;
		}

		// Only create AttributeSet if not already done for this AttributeSet class
		if (!RegisteredAttributeSetNames.Contains(AttributeSetClass->GetName()))
		{
			GSC_VLOG(this, Log, TEXT("AGSCActor: Adding AttributeSet %s to Pawn."), *AttributeSetClass->GetName())

			const UAttributeSet* AttributeSet = AbilitySystemComponent->InitStats(AttributeSetClass, nullptr);
			AttributeSets.Add(AttributeSet);
			RegisteredAttributeSetNames.Add(AttributeSetClass->GetName());
		}
		else
		{
			GSC_LOG(Warning, TEXT("AGSCActor: AttributeSet %s seem to have duplicates. Please check Project's Settings for any duplicated AttributeSet."), *AttributeSetClass->GetName())
		}
	}
}

