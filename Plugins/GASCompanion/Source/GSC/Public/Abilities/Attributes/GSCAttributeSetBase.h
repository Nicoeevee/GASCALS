// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

#include "GSCAttributeSetBase.generated.h"

class UGSCCoreComponent;
class AGSCCharacterBase;
struct FGameplayTagContainer;

/** Structure holding various information to deal with AttributeSet PostGameplayEffectExecute, extracting info from FGameplayEffectModCallbackData */
USTRUCT()
struct FGSCAttributeSetExecutionData
{
	GENERATED_BODY()

	/** The physical representation of the Source ASC (The ability system component of the instigator that started the whole chain) */
	UPROPERTY()
	AActor* SourceActor = nullptr;

	/** The physical representation of the owner (Avatar) for the target we intend to apply to  */
	UPROPERTY()
	AActor* TargetActor = nullptr;

	/** The ability system component of the instigator that started the whole chain */
	UPROPERTY()
	UAbilitySystemComponent* SourceASC = nullptr;

	/** GAS Companion Core actor component attached to Source Actor (if any) */
	UPROPERTY()
	UGSCCoreComponent* SourceCoreComponent = nullptr;

	/** GAS Companion Core actor component attached to Target Actor (if any) */
	UPROPERTY()
	UGSCCoreComponent* TargetCoreComponent = nullptr;

	/** PlayerController associated with the owning actor for the Source ASC (The ability system component of the instigator that started the whole chain) */
	UPROPERTY()
	APlayerController* SourceController = nullptr;

	/** PlayerController associated with the owning actor for the target we intend to apply to */
	UPROPERTY()
	APlayerController* TargetController = nullptr;

	/** The physical representation of the Source ASC (The ability system component of the instigator that started the whole chain), as a APawn */
	UPROPERTY()
	APawn* SourcePawn = nullptr;

	/** The physical representation of the owner (Avatar) for the target we intend to apply to, as a APawn */
	UPROPERTY()
	APawn* TargetPawn = nullptr;

	/** The object this effect was created from. */
	UPROPERTY()
	UObject* SourceObject = nullptr;

	/** This tells us how we got here (who / what applied us) */
	FGameplayEffectContextHandle Context;

	/** Combination of spec and actor tags for the captured Source Tags on GameplayEffectSpec creation */
	FGameplayTagContainer SourceTags;

	/** All tags that apply to the gameplay effect spec */
	FGameplayTagContainer SpecAssetTags;

	/** Holds the delta value between old and new, if it is available (for Additive Operations) */
	float DeltaValue;
};

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * Base AttributeSet Class to extend from.
 *
 * Doesn't hold any Gameplay Attribute but defines any shared logic for AttributeSet, such as:
 *
 * - AdjustAttributeForMaxChange()
 * - Clamp Values definition from Project Config
 * -
 */
UCLASS()
class GASCOMPANION_API UGSCAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:

	// Sets default values for this AttributeSet attributes
	UGSCAttributeSetBase();

    // AttributeSet Overrides
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    /** Helper function to get the minimum clamp value for a given attribute, from developer settings if available */
    virtual float GetClampMinimumValueFor(const FGameplayAttribute& Attribute);

	/**
	 * Extract Source and Target Characters from the EffectSpec, Context and Target AbilityActorInfo.
	 *
	 * @param Data The PostGameplayEffectExecute callback data (@see FGameplayEffectModCallbackData)
	 * @param SourceCharacter returned Source Character (AGSCCharacterBase), the instigator that started the whole chain
	 * @param TargetCharacter returned Target Character (AGSCCharacterBase), the owning actor to which the EffectSpec Context is applied to
	 *
	 * @deprecated Use GetExecutionDataFromMod() instead and read SourceActor / TargetActor from returned structure (FGSCAttributeSetExecutionData)
	 *
	 */
	UE_DEPRECATED(4.27, "Use GetExecutionDataFromMod() instead and read SourceActor / TargetActor from returned structure (FGSCAttributeSetExecutionData).")
	virtual void GetCharactersFromContext(const FGameplayEffectModCallbackData& Data, AGSCCharacterBase*& SourceCharacter, AGSCCharacterBase*& TargetCharacter);

	/**
	 * Return the current active state Guid. If a state is not active an invalid Guid will be returned.
	 * This only returns top level states! Use GetNestedActiveStateGuid for the exact state.
	 *
	 * @deprecated Use GetSingleActiveStateGuid() with bCheckNested = false.
	 */
	// UE_DEPRECATED(4.24, "Use `GetSingleActiveStateInstance` with bCheckNested = false instead and read `GetNodeName` from there")

	/**
	 * Returns the aggregated SourceTags for this EffectSpec
	 */
	virtual const FGameplayTagContainer& GetSourceTagsFromContext(const FGameplayEffectModCallbackData& Data);

	/**
	 * Templated version of GetCharactersFromContext
	 */
	template<class TReturnType>
	void GetSourceAndTargetFromContext(const FGameplayEffectModCallbackData& Data, TReturnType*& SourceActor, TReturnType*& TargetActor)
	{
		const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
		UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();

		// Get the Target actor, which should be our owner
		if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
		{
			TargetActor = Cast<TReturnType>(Data.Target.AbilityActorInfo->AvatarActor.Get());
		}

		// Get the Source actor, which should be the damage causer (instigator)
		if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
		{
			// Set the source actor based on context if it's set
			if (Context.GetEffectCauser())
			{
				SourceActor = Cast<TReturnType>(Context.GetEffectCauser());
			}
			else
			{
				SourceActor = Cast<TReturnType>(Source->AbilityActorInfo->AvatarActor.Get());
			}
		}
	}

    /**
     * Helper function to proportionally adjust the value of an attribute when it's associated max attribute changes.
     *
     * (e.g. When MaxHealth increases, Health increases by an amount that maintains the same percentage as before)
     */
    virtual void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const;

protected:

    /** Cached map of minimum clamp values for each configured attributes*/
    UPROPERTY(meta=(HideInDetailsView))
    TMap<FGameplayAttribute, float> MinimumValues;

	/**
	 * Fills out FGSCAttributeSetExecutionData structure based on provided data.
	 *
	 * @param Data The gameplay effect mod callback data available in attribute sets' PostGameplayEffectExecute
	 * @param OutExecutionData Returned structure with various information extracted from Data (Source / Target Actor, Controllers, etc.)
	 */
	virtual void GetExecutionDataFromMod(const FGameplayEffectModCallbackData& Data, OUT FGSCAttributeSetExecutionData& OutExecutionData);
};
