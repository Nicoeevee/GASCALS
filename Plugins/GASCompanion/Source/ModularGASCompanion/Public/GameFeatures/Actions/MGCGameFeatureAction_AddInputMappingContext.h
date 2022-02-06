// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFeatureAction.h"
#include "MGCGameFeatureAction_AddInputMappingContext.generated.h"

struct FMGCComponentRequestHandle;
struct FComponentRequestHandle;
class UInputMappingContext;

/**
* Adds InputMappingContext to local players' EnhancedInput system.
*
* Expects that local players are set up to use the EnhancedInput system.
*/
UCLASS(MinimalAPI, meta = (DisplayName = "Add Input Mapping (Modular GAS Companion)"))
class UMGCGameFeatureAction_AddInputMappingContext : public UGameFeatureAction
{
	GENERATED_BODY()

public:

	// Input Mapping Context to add to local players EnhancedInput system.
	UPROPERTY(EditAnywhere, Category="Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	// Higher priority input mappings will be prioritized over mappings with a lower priority.
	UPROPERTY(EditAnywhere, Category="Input")
	int32 Priority = 0;

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


private:
	TArray<TSharedPtr<FMGCComponentRequestHandle>> ExtensionsRequests;
	TArray<TSharedPtr<FComponentRequestHandle>> ExtensionRequestHandles;
	TArray<TWeakObjectPtr<APlayerController>> ControllersAddedTo;

	FDelegateHandle GameInstanceStartHandle;

	virtual void AddToWorld(const FWorldContext& WorldContext);

	void Reset();
	void HandleControllerExtension(AActor* Actor, FName EventName);
	void AddInputMappingForPlayer(UPlayer* Player);
	void RemoveInputMapping(APlayerController* PlayerController);
	void HandleGameInstanceStart(UGameInstance* GameInstance);
};
