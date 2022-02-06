// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayTagContainer.h"
#include "GSCTypes.generated.h"

class UGSCTargetType;
class UGameplayEffect;

/** List of Ability Inputs ID */
UENUM(BlueprintType)
enum class EGSCAbilityInputID : uint8
{
	None            UMETA(DisplayName = "None"),
    Confirm         UMETA(DisplayName = "Confirm"),
    Cancel          UMETA(DisplayName = "Cancel"),
    Ability1        UMETA(DisplayName = "Ability1")
};

/**
* Struct defining a list of gameplay effects, a tag, and targeting info
*
* These containers are defined statically in blueprints or assets and then turn into Specs at runtime
*/
USTRUCT(BlueprintType)
struct FGSCGameplayEffectContainer
{
	GENERATED_BODY()

public:
	FGSCGameplayEffectContainer() {}

	/** Sets the way that targeting happens */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TSubclassOf<UGSCTargetType> TargetType;

	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	bool bUseSetByCallerMagnitude = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	FGameplayTag SetByCallerDataTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	float SetByCallerMagnitude = 1.0f;
};

/** A "processed" version of GSCGameplayEffectContainer that can be passed around and eventually applied */
USTRUCT(BlueprintType)
struct GASCOMPANION_API FGSCGameplayEffectContainerSpec
{
	GENERATED_BODY()

public:
	FGSCGameplayEffectContainerSpec() {}

	/** Computed target data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	FGameplayAbilityTargetDataHandle TargetData;

	/** List of gameplay effects to apply to the targets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

	/** Returns true if this has any valid effect specs */
	bool HasValidEffects() const;

	/** Returns true if this has any valid targets */
	bool HasValidTargets() const;

	/** Adds new targets to target data */
	void AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);
};
