// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "ModularGASCompanionTypes.h"
#include "Abilities/GameplayAbility.h"
#include "MGCGameFeatureAction_AddAbilities.generated.h"

struct FMGCComponentRequestHandle;
class UMGCAbilityInputBindingComponent;
struct FComponentRequestHandle;
class UInputAction;
class UDataTable;

USTRUCT(BlueprintType)
struct FMGCGameFeatureAbilityMapping
{
	GENERATED_BODY()

	// Type of ability to grant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	TSoftClassPtr<UGameplayAbility> AbilityType;

	// Input action to bind the ability to, if any (can be left unset)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	TSoftObjectPtr<UInputAction> InputAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability, meta=(EditCondition = "InputAction != nullptr", EditConditionHides))
	EMGCAbilityTriggerEvent TriggerEvent = EMGCAbilityTriggerEvent::Started;
};

USTRUCT(BlueprintType)
struct FMGCGameFeatureAttributeSetMapping
{
	GENERATED_BODY()

	// Attribute Set to grant
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	TSoftClassPtr<UAttributeSet> AttributeSet;

	// Data table referent to initialize the attributes with, if any (can be left unset)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Attributes)
	TSoftObjectPtr<UDataTable> InitializationData;
};

USTRUCT()
struct FMGCGameFeatureAbilitiesEntry
{
	GENERATED_BODY()

	// The base actor class to add to
	UPROPERTY(EditAnywhere, Category="Abilities")
	TSoftClassPtr<AActor> ActorClass;

	// List of abilities to grant to actors of the specified class
	UPROPERTY(EditAnywhere, Category="Abilities")
	TArray<FMGCGameFeatureAbilityMapping> GrantedAbilities;

	// List of attribute sets to grant to actors of the specified class
	UPROPERTY(EditAnywhere, Category="Attributes")
	TArray<FMGCGameFeatureAttributeSetMapping> GrantedAttributes;
};

/**
 * GameFeatureAction responsible for granting abilities (and attributes) to actors of a specified type.
 */
UCLASS(MinimalAPI, meta = (DisplayName = "Add Abilities (Modular GAS Companion)"))
class UMGCGameFeatureAction_AddAbilities : public UGameFeatureAction
{
	GENERATED_BODY()

public:
	//~ Begin UGameFeatureAction interface
	virtual void OnGameFeatureActivating() override;
	virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
#if WITH_EDITORONLY_DATA
	virtual void AddAdditionalAssetBundleData(FAssetBundleData& AssetBundleData) override;
#endif
	//~ End UGameFeatureAction interface

	//~ Begin UObject interface
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(TArray<FText>& ValidationErrors) override;
#endif
	//~ End UObject interface

	/** List of Ability to grant to actors of the specified class */
	UPROPERTY(EditAnywhere, Category="Abilities", meta=(TitleProperty="ActorClass", ShowOnlyInnerProperties))
	TArray<FMGCGameFeatureAbilitiesEntry> AbilitiesList;

	void Reset();
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex);
	void AddActorAbilities(AActor* Actor, const FMGCGameFeatureAbilitiesEntry& AbilitiesEntry);
	void RemoveActorAbilities(const AActor* Actor);

	template<class ComponentType>
	ComponentType* FindOrAddComponentForActor(AActor* Actor, const FMGCGameFeatureAbilitiesEntry& AbilitiesEntry)
	{
		return Cast<ComponentType>(FindOrAddComponentForActor(ComponentType::StaticClass(), Actor, AbilitiesEntry));
	}
	UActorComponent* FindOrAddComponentForActor(UClass* ComponentType, const AActor* Actor, const FMGCGameFeatureAbilitiesEntry& AbilitiesEntry);
private:
	struct FActorExtensions
	{
		TArray<FGameplayAbilitySpecHandle> Abilities;
		TArray<UAttributeSet*> Attributes;
		TArray<FDelegateHandle> InputBindingDelegateHandles;
	};

	FDelegateHandle GameInstanceStartHandle;

	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<AActor*, FActorExtensions> ActiveExtensions;

	TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequests;
	TArray<TSharedPtr<FMGCComponentRequestHandle>> ExtensionsRequests;

	virtual void AddToWorld(const FWorldContext& WorldContext);
	void HandleGameInstanceStart(UGameInstance* GameInstance);

	/** Handler for AbilitySystem OnGiveAbility delegate. Sets up input binding for clients (not authority) when GameFeatures are activated during Play. */
	void HandleOnGiveAbility(FGameplayAbilitySpec& AbilitySpec, UMGCAbilityInputBindingComponent* InputComponent, UInputAction* InputAction, EMGCAbilityTriggerEvent TriggerEvent, FGameplayAbilitySpec NewAbilitySpec);

	/** Does the passed in ability system component have this attribute set? */
	static bool HasAttributeSet(UAbilitySystemComponent* AbilitySystemComponent, const TSubclassOf<UAttributeSet> Set);
};
