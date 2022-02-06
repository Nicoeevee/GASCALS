// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "MGCGameFeatureAction_AddAnimLayers.generated.h"

struct FMGCComponentRequestHandle;
struct FComponentRequestHandle;

USTRUCT()
struct FMGCAnimLayerEntry
{
	GENERATED_BODY()

	// The base actor class to add anim layers to
	UPROPERTY(EditAnywhere, Category="Anim Layers")
	TSoftClassPtr<AActor> ActorClass;

	UPROPERTY(EditAnywhere, Category="Anim Layers")
	TArray<TSoftClassPtr<UAnimInstance>> AnimLayers;
};

/**
* GameFeatureAction responsible for "pushing" linked Anim Layers to main Animation Blueprint
*/
UCLASS(MinimalAPI, meta = (DisplayName = "Add Anim Layers (Modular GAS Companion)"))
class UMGCGameFeatureAction_AddAnimLayers : public UGameFeatureAction
{
	GENERATED_BODY()
public:
	/** List of components to add to gameplay actors when this game feature is enabled */
	UPROPERTY(EditAnywhere, Category="Components", meta=(TitleProperty="ActorClass")) // ShowOnlyInnerProperties ?
	TArray<FMGCAnimLayerEntry> AnimLayerEntries;

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

	template<class ComponentType>
	ComponentType* FindOrAddComponentForActor(AActor* Actor, const FMGCAnimLayerEntry& AnimLayerEntry)
	{
		return Cast<ComponentType>(FindOrAddComponentForActor(ComponentType::StaticClass(), Actor, AnimLayerEntry));
	}
	UActorComponent* FindOrAddComponentForActor(UClass* ComponentType, AActor* Actor, const FMGCAnimLayerEntry& AnimLayerEntry);

private:
	struct FActorExtensions
	{
		TArray<TSubclassOf<UAnimInstance>> AnimLayers;
	};

	FDelegateHandle GameInstanceStartHandle;

	TArray<TSharedPtr<FMGCComponentRequestHandle>> ExtensionsRequests;
	TArray<TSharedPtr<FComponentRequestHandle>> ComponentRequestHandles;

	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer
	TMap<AActor*, FActorExtensions> ActiveExtensions;

	void Reset();
	void HandleActorExtension(AActor* Actor, FName EventName, int32 EntryIndex);
	void AddAnimLayers(AActor* Actor, const FMGCAnimLayerEntry& Entry);
	void RemoveAnimLayers(AActor* Actor);

	void AddToWorld(const FWorldContext& WorldContext);
	void HandleGameInstanceStart(UGameInstance* GameInstance);
};
