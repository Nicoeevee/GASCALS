// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Actors/Characters/GSCAICharacter.h"
#include "GameplayEffectExtension.h"
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCCoreComponent.h"
#include "GSCLog.h"

AGSCAICharacter::AGSCAICharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Create ability system component
	AbilitySystemComponent = CreateDefaultSubobject<UGSCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// Disable Ability Queue System for AI Characters by default (can still be enabled back in Blueprints)
	GSCAbilityQueueComponent->bAbilityQueueEnabled = false;
}

void AGSCAICharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	SetupAttributeSets();
}

void AGSCAICharacter::BeginPlay()
{
    Super::BeginPlay();

	GSCCoreComponent->SetupAbilityActor(this,this);
}

void AGSCAICharacter::SetupAttributeSets()
{
	if (!AbilitySystemComponent.IsValid())
	{
		return;
	}

	// Create the attribute sets from DeveloperSettings Configuration
	const UGSCDeveloperSettings* Settings = GetDefault<UGSCDeveloperSettings>();
	TArray<TSubclassOf<UGSCAttributeSetBase>> AICharacterAttributeSets = Settings->AICharactersAttributeSets;

	// Add AttributeSet if not prevented by config
	if (Settings->bWantsDefaultAttributeAICharacters)
	{
		AICharacterAttributeSets.Add(UGSCAttributeSet::StaticClass());
	}

	for (const TSubclassOf<UAttributeSet> AttributeSetClass : AICharacterAttributeSets)
	{
		if (!AttributeSetClass)
		{
			GSC_LOG(Warning, TEXT("AGSCAICharacter: Some configured AttributeSets for Pawn Characters seem to be missing. Please check Project's Settings for any AttributeSet with a NONE value."))
			continue;
		}

		// Only create AttributeSet if not already done for this AttributeSet class
		if (!RegisteredAttributeSetNames.Contains(AttributeSetClass->GetName()))
		{
			GSC_VLOG(this, Log, TEXT("AGSCAICharacter: Adding AttributeSet %s to AI Character."), *AttributeSetClass->GetName())

			const UAttributeSet* AttributeSet = AbilitySystemComponent->InitStats(AttributeSetClass, nullptr);
			AttributeSets.Add(AttributeSet);
			RegisteredAttributeSetNames.Add(AttributeSetClass->GetName());
		}
		else
		{
			GSC_LOG(Warning, TEXT("AGSCAICharacter: AttributeSet %s seem to have duplicates. Please check Project's Settings for any duplicated AttributeSet."), *AttributeSetClass->GetName())
		}
	}
}
