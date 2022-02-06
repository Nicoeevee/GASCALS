// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GSCUserWidget.generated.h"

class UGSCCoreComponent;
struct FGameplayAttribute;

UCLASS()
class GASCOMPANION_API UGSCUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category="GAS Companion")
	AActor* OwnerActor;

	UPROPERTY(BlueprintReadOnly, Category="GAS Companion")
	UGSCCoreComponent* OwnerCoreComponent;

	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual void SetOwnerActor(AActor* Actor);

	/** Helper function to return percentage from Attribute / MaxAttribute */
	UFUNCTION(BlueprintPure, Category="GAS Companion|UI")
	float GetPercentForAttributes(FGameplayAttribute Attribute, FGameplayAttribute MaxAttribute);
};
