// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Player/GSCPlayerState.h"

#include "GSCLog.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Components/GSCCoreComponent.h"
#include "Player/GSCPlayerController.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "UI/GSCUWHud.h"

AGSCPlayerState::AGSCPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UGSCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies.
	// If another PlayerState (Hero) receives a GE, we won't be told about it by the Server.
	// Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 10.0f;

	// Create the attribute sets from DeveloperSettings Configuration
    SetupAttributeSets();
}

UAbilitySystemComponent* AGSCPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGSCPlayerState::SetupAttributeSets()
{
	const UGSCDeveloperSettings* Settings = GetDefault<UGSCDeveloperSettings>();
	TArray<TSubclassOf<UGSCAttributeSetBase>> PlayerStateAttributeSets = Settings->PlayerStateAttributeSets;

	// Add GSCPlayerState if not prevented by config
	if (Settings->bWantsDefaultAttributePlayerState)
	{
		PlayerStateAttributeSets.Add(UGSCAttributeSet::StaticClass());
	}

	for (const TSubclassOf<UAttributeSet> AttributeSetClass : PlayerStateAttributeSets)
	{
		if (!AttributeSetClass)
		{
			continue;
		}

		// Only create AttributeSet if not already done for this AttributeSet class
		if (!RegisteredAttributeSetNames.Contains(AttributeSetClass->GetName()))
		{
			GSC_LOG(Verbose, TEXT("GSCPlayerState: Adding AttributeSet %s to PlayerState."), *AttributeSetClass->GetName())

			// Adding it as a subobject of the owning actor of an AbilitySystemComponent automatically registers the AttributeSet with the AbilitySystemComponent
			// Here we using the non templated version of CreateDefaultSubobject to be able to dynamically create those from configured PlayerState AttributeSets
			UAttributeSet* AttributeSetTemp = Cast<UAttributeSet>(CreateDefaultSubobject(AttributeSetClass->GetFName(), *AttributeSetClass, *AttributeSetClass, true, false));
			AttributeSets.Add(AttributeSetTemp);
			RegisteredAttributeSetNames.Add(AttributeSetClass->GetName());
		}
		else
		{
			GSC_LOG(Warning, TEXT("GSCPlayerState: AttributeSet %s seem to have duplicates. Please check Project's Settings for any duplicated AttributeSet."), *AttributeSetClass->GetName())
		}
	}
}
