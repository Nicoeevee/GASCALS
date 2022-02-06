// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GSCHUD.generated.h"

class UUserWidget;

/**
 * Specialized HUD Class to subclass as a Blueprint set in GameMode.
 */
UCLASS()
class GASCOMPANION_API AGSCHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGSCHUD(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	UUserWidget* CreateHUDWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
    void ShowHUDWidget() const;

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	void HideHUDWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
    UUserWidget* GetHUDWidget() const;

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	UUserWidget* CreateAbilityQueueWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
    void ShowAbilityQueueWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	void HideAbilityQueueWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
    UUserWidget* GetAbilityQueueWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	bool IsAbilityQueueWidgetVisible();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	UUserWidget* CreateComboWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	void ShowComboWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	void HideComboWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	void ToggleComboWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	UUserWidget* GetComboWidget();

	UFUNCTION(BlueprintCallable, Category = "GAS Companion|HUD")
	bool IsComboWidgetVisible();

protected:

	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|UI")
	TSubclassOf<UUserWidget> AbilityQueueWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|UI")
	TSubclassOf<UUserWidget> ComboWidgetClass;

	UPROPERTY()
	class UUserWidget* HUDWidget;

	UPROPERTY()
	class UUserWidget* DebugWidget;

	UPROPERTY()
	class UUserWidget* ComboDebugWidget;

	void InitDefaultWidgetClasses();
};
