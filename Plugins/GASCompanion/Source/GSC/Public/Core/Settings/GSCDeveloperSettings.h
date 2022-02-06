// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Attributes/GSCAttributeSetBase.h"
#include "UI/GSCUWHud.h"

#include "GSCDeveloperSettings.generated.h"

/**
 * Attribute Set Settings
 */
USTRUCT(BlueprintType)
struct GASCOMPANION_API FGSCAttributeSetMinimumValues
{
	GENERATED_BODY()

	/** The Attribute we want to configure clamp values for. */
	UPROPERTY(EditDefaultsOnly, Category=GameplayModifier, meta=(FilterMetaTag="HideFromModifiers"))
	FGameplayAttribute Attribute;

	/** Minimum value for this attribute when a Clamp is done in PostGameplayEffectExecute of Attribute Sets */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Attribute Set")
	float MinimumValue = 0.f;
};

/**
 * General Settings for GAS Companion Plugin.
 *
 * ---
 *
 * **Note** Old AttributeSets settings that were used in 2.0 version of GAS Companion for 4.26 have been deprecated
 * and moved to AdvancedDisplay (the little arrow at the bottom of Attribute Sets category)
 *
 * These should no longer be used in favor of Modular Actors which allow configuration of Attribute Sets per actor.
 *
 * Configuration for clamping of minimal values still has some use, but will be revamped in future versions.
 *
 * ---
 */
UCLASS(Config="Game", defaultconfig, meta=(DisplayName="GAS Companion"))
class GASCOMPANION_API UGSCDeveloperSettings : public UObject
{
	GENERATED_BODY()

public:

	UGSCDeveloperSettings(const FObjectInitializer& ObjectInitializer);

	/**
	 * Turn this on to prevent GAS Companion module to initialize UAbilitySystemGlobals (InitGlobalData) in the plugin StartupModule method.
	 *
	 * InitGlobalData() might be invoked a bit too early otherwise (with GAS Companion's StartupModule). It is expected that if you set this option to true to use
	 * an AssetManager subclass where `UAbilitySystemGlobals::Get().InitGlobalData()` is called in `StartInitialLoading``
	 *
	 * You'll need to update `Project Settings -> Engine > General Settings > Asset Manager Class` to use your AssetManager subclass.
	 *
	 * GAS Companion provides one `GSCAssetManager` and the editor should ask you if you want to update the `Asset Manager Class` to use it if the current Manager class
	 * is using engine's default one.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Ability System", meta=(DisplayName = "Prevent Ability System Global Data Initialization in Startup Module (Recommended)"))
	bool bPreventGlobalDataInitialization = false;

	/**
	 * Configuration of minimum values for Attributes for FMath::Clamp(), if not specified minimum value defaults to 0.
	 *
	 * This is useful to specify when using abilities with bIgnoreAbilityCost set to true, which allow activation of abilities
	 * even if the applied cost would go into negative values, and only prevented if the attribute is 0 or below.
	 *
	 * (ex. If you have a regeneration effect on an attribute, it would then take longer for the attribute to go positive again)
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets")
	TArray<FGSCAttributeSetMinimumValues> MinimumValues;

	/**
	 * List of AttributeSet to create and attach to PlayerStates (for PlayerCharacter or Pawns with a PlayerState
	 * where the Ability System Component is registered - GSCPlayerCharacter)
	 *
	 * This is useful to configure here any custom AttributeSets you created and want to have available for your Player Characters.
	 *
	 * Default AttributeSet (UGSCAttribute Set with Health, Stamina, etc.) is added by default unless prevented by advanced config below.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "Player State Attribute Sets (Deprecated)"))
	TArray<TSubclassOf<UGSCAttributeSetBase>> PlayerStateAttributeSets;

	/**
	* List of AttributeSet to create and attach to AI Characters (GSCAICharacter)
	*
	* This is useful to configure here any custom AttributeSets you created and want to have available for your AI/NPC Characters
	* (or Pawns where the Ability System Component is registred directly on Pawn)
	*
	* Default AttributeSet (UGSCAttribute Set with Health, Stamina, etc.) is added by default unless prevented by advanced config below.
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "AI Characters Attribute Sets (Deprecated)"))
	TArray<TSubclassOf<UGSCAttributeSetBase>> AICharactersAttributeSets;

	/**
	* List of AttributeSet to create and attach to Pawns (GSCDefaultPawn)
	*
	* This is useful to configure here any custom AttributeSets you created and want to have available for your non character-based avatars
	* (creatures, props, etc.)
	*
	* Default AttributeSet (UGSCAttribute Set with Health, Stamina, etc.) is added by default unless prevented by advanced config below.
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "Pawns Attribute Sets (Deprecated)"))
	TArray<TSubclassOf<UGSCAttributeSetBase>> PawnsAttributeSets;

	/**
	* List of AttributeSet to create and attach to Actors (GSCActor)
	*
	* This is useful to configure here any custom AttributeSets you created and want to have available for your non character-based avatars
	* (creatures, props, etc.)
	*
	* Default AttributeSet (UGSCAttribute Set with Health, Stamina, etc.) is added by default unless prevented by advanced config below.
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "Actors Attribute Sets (Deprecated)"))
	TArray<TSubclassOf<UGSCAttributeSetBase>> ActorsAttributeSets;

	/**
	 * Control whether or not to create and attach default GAS Companion AttributeSet (UGSCAttributeSet) in PlayerStates (GSCPlayerCharacter).
	 *
	 * You probably want to keep it true, unless you handle those attributes yourself (in HUD, Abilities, Effects, ...)
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "Create Default AttributeSet for Player State (Deprecated)"))
	bool bWantsDefaultAttributePlayerState = true;

	/**
	* Control whether or not to create and attach default GAS Companion AttributeSet (UGSCAttributeSet) for AI Characters (GSCAICharacter).
	*
	* You probably want to keep it true, unless you handle those attributes yourself (UI, Abilities, Effects, ...)
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "Create Default AttributeSet for AI Characters (Deprecated)"))
	bool bWantsDefaultAttributeAICharacters = true;

	/**
	* Control whether or not to create and attach default GAS Companion AttributeSet (UGSCAttributeSet) for Pawns (GSCDefaultPawn).
	*
	* You probably want to keep it true, unless you handle those attributes yourself (UI, Abilities, Effects, ...)
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "Create Default AttributeSet for Pawns (Deprecated)"))
	bool bWantsDefaultAttributePawns = true;

	/**
	* Control whether or not to create and attach default GAS Companion AttributeSet (UGSCAttributeSet) for Actors (GSCActor).
	*
	* You probably want to keep it true, unless you handle those attributes yourself (UI, Abilities, Effects, ...)
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Attribute Sets", AdvancedDisplay, meta=(DisplayName = "Create Default AttributeSet for Pawns (Deprecated)"))
	bool bWantsDefaultAttributeActors = true;
};
