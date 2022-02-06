// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GSCGameplayEffectCreationMenu.generated.h"

class UGameplayEffect;

USTRUCT()
struct FGSCGameplayEffectCreationData
{
	GENERATED_BODY()

	/** Where to show this in the menu. Use "|" for sub categories. E.g, "Status|Hard|Stun|Root". */
	UPROPERTY(EditAnywhere, Category="Gameplay Effect")
	FString MenuPath;

	/** The default BaseName of the new asset. E.g "Damage" -> GE_Damage */
	UPROPERTY(EditAnywhere, Category="Gameplay Effect")
	FString BaseName;

	UPROPERTY(EditAnywhere, Category = "Gameplay Effect", meta=(DisplayName = "Parent Gameplay Effect"))
	TSubclassOf<UGameplayEffect> ParentClass;

	UPROPERTY()
	FString AssetPrefix = "GE_";

	UPROPERTY()
	FText TooltipText = FText::FromString("Tooltip");
};

UCLASS(config=Game, defaultconfig, notplaceable)
class GASCOMPANIONEDITOR_API UGSCGameplayEffectCreationMenu : public UObject
{
	GENERATED_BODY()

public:
	UGSCGameplayEffectCreationMenu();

	void AddMenuExtensions() const;

	static void TopMenuBuild(FMenuBuilder& TopMenuBuilder, const FText InMenuLabel, const FText InMenuTooltip, const TArray<FString> SelectedPaths, TArray<FGSCGameplayEffectCreationData> Definitions);

	UPROPERTY(config, EditAnywhere, Category="Gameplay Effect")
	TArray<FGSCGameplayEffectCreationData> Definitions;

protected:

	void AddDefinition(const FString MenuPath, const FString BaseName, const TSubclassOf<UGameplayEffect> ParentClass, const FText TooltipText = FText());
	static FGSCGameplayEffectCreationData CreateDefinition(const FString MenuPath, const FString BaseName, const TSubclassOf<UGameplayEffect> ParentClass, const FText TooltipText);
};
