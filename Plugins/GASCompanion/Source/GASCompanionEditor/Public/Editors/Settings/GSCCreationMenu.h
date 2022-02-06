// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Factories/BlueprintFactory.h"
#include "GSCCreationMenu.generated.h"

struct FGSCMenuItem : TSharedFromThis<FGSCMenuItem>
{
	TMap<FString, TSharedPtr<FGSCMenuItem>> SubItems;

	FString AssetPrefix;
	FString DefaultAssetName;
	FText TooltipText;
	TSubclassOf<class UObject> CDO;

	static void BuildMenus_r(TSharedPtr<FGSCMenuItem> Item, FMenuBuilder& MenuBuilder, TArray<FString> SelectedPaths)
	{
		for (auto It : Item->SubItems)
		{
			TSharedPtr<FGSCMenuItem> SubItem  = It.Value;
			FString CatName = It.Key;

			// Add a submenu if this guy has sub items
			if (SubItem->SubItems.Num() > 0)
			{
				MenuBuilder.AddSubMenu(
					FText::FromString(CatName),
					FText::FromString(CatName),
					FNewMenuDelegate::CreateLambda([SubItem, SelectedPaths](FMenuBuilder& SubMenuBuilder)
					{
						BuildMenus_r(SubItem, SubMenuBuilder, SelectedPaths);
					})
				);
			}

			// Add the actual menu item to create the new GE
			if (SubItem->CDO.Get() != nullptr)
			{
				MenuBuilder.AddMenuEntry(
					// note: the last category string is what we use for this. Using the "Default Asset Name" is not desirable here. (e.g., Damage|Ability|Instant but "Damage" is default asset name)
					FText::FromString(CatName),
					SubItem->TooltipText,
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([=]()
					{
						if (SelectedPaths.Num() > 0)
						{
							FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
							FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

							TArray<FAssetData> SelectedAssets;
							ContentBrowserModule.Get().GetSelectedAssets(SelectedAssets);

							UBlueprintFactory* BlueprintFactory = NewObject<UBlueprintFactory>();
							BlueprintFactory->ParentClass = SubItem->CDO;

							FString PackageName = SelectedPaths[0];
							FString AssetName = SubItem->AssetPrefix + SubItem->DefaultAssetName;

							const FString DefaultFullPath = PackageName / AssetName;

							AssetToolsModule.Get().CreateUniqueAssetName(DefaultFullPath, TEXT(""), /*out*/ PackageName, /*out*/ AssetName);
							ContentBrowserModule.Get().CreateNewAsset(*AssetName, SelectedPaths[0], UBlueprint::StaticClass(), BlueprintFactory);
						}
					}))
				);
			}
		}
	}
};

UCLASS()
class GASCOMPANIONEDITOR_API UGSCCreationMenu : public UObject
{
	GENERATED_BODY()

public:

	template <typename TCreationData>
	static void TopMenuBuild(FMenuBuilder& TopMenuBuilder, const FText InMenuLabel, const FText InMenuTooltip, const TArray<FString> SelectedPaths, TArray<TCreationData> Definitions)
	{
		if (Definitions.Num() == 0)
		{
			return;
		}

		TopMenuBuilder.AddSubMenu(
			InMenuLabel,
			InMenuTooltip,
			FNewMenuDelegate::CreateLambda([SelectedPaths, Definitions](FMenuBuilder& GETopMenuBuilder)
			{
				// Loop through our Definitions and build FGSCMenuItem
				const TSharedPtr<FGSCMenuItem> RootItem = MakeShareable(new FGSCMenuItem);
				for (const TCreationData& Def : Definitions)
				{
					if (Def.ParentClass.Get() == nullptr)
					{
						continue;
					}

					TArray<FString> CategoryStrings;
					Def.MenuPath.ParseIntoArray(CategoryStrings, TEXT("|"), true);

					FGSCMenuItem* CurrentItem = RootItem.Get();
					for (int32 idx=0; idx < CategoryStrings.Num(); ++idx)
					{
						FString& Str = CategoryStrings[idx];
						TSharedPtr<FGSCMenuItem>& DestItem = CurrentItem->SubItems.FindOrAdd(Str);
						if (!DestItem.IsValid())
						{
							DestItem = MakeShareable(new FGSCMenuItem);
						}
						CurrentItem = DestItem.Get();
					}

					CurrentItem->AssetPrefix = Def.AssetPrefix;
					CurrentItem->DefaultAssetName = Def.BaseName;
					CurrentItem->TooltipText = Def.TooltipText;
					CurrentItem->CDO = Def.ParentClass.Get();
				}

				// Build menu delegates based on our data
				FGSCMenuItem::BuildMenus_r(RootItem, GETopMenuBuilder, SelectedPaths);
			})
		);
	}
};
