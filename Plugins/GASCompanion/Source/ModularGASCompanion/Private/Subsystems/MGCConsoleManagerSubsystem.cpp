// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Subsystems/MGCConsoleManagerSubsystem.h"

#include "AbilitySystemComponent.h"
#include "DisplayDebugHelpers.h"
#include "ModularGASCompanionLog.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCComboManagerComponent.h"
#include "Player/GSCHUD.h"

void UMGCConsoleManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	MGC_LOG(Log, TEXT("Initializing GAS Companion console manager subsystem"));
	Super::Initialize(Collection);

	// Register Console Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("GASCompanion.Debug.Combo"),
		TEXT("Toogles display of Combo Widget"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateUObject(this, &UMGCConsoleManagerSubsystem::ToggleComboWidget)
	);

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("GASCompanion.Debug.AbilityQueue"),
		TEXT("Toogles display of AbilityQueue Widget"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateUObject(this, &UMGCConsoleManagerSubsystem::ToggleAbilityQueueWidget)
	);

	// IConsoleManager::Get().RegisterConsoleCommand(
	// 	TEXT("GASCompanion.ShowDebug.Attributes"),
	// 	TEXT("Executes `showdebug abilitysystem` and navigate to Attributes category"),
	// 	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateUObject(this, &UMGCConsoleManagerSubsystem::ShowDebugAbilitySystem, 0)
	// );
	//
	// IConsoleManager::Get().RegisterConsoleCommand(
	// 	TEXT("GASCompanion.ShowDebug.GameplayEffects"),
	// 	TEXT("Executes `showdebug abilitysystem` and navigate to GameplayEffects category"),
	// 	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateUObject(this, &UMGCConsoleManagerSubsystem::ShowDebugAbilitySystem, 1)
	// );
	//
	// IConsoleManager::Get().RegisterConsoleCommand(
	// 	TEXT("GASCompanion.ShowDebug.Ability"),
	// 	TEXT("Executes `showdebug abilitysystem` and navigate to Ability category"),
	// 	FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateUObject(this, &UMGCConsoleManagerSubsystem::ShowDebugAbilitySystem, 2)
	// );
	//
	// // TODO Custom debugger (main thing I'd like to have is to store the last displayed category)
	// if (!IsRunningDedicatedServer())
	// {
	// 	AHUD::OnShowDebugInfo.AddStatic(&UMGCConsoleManagerSubsystem::OnShowDebugInfo);
	// }
}

void UMGCConsoleManagerSubsystem::Deinitialize()
{
	MGC_LOG(Log, TEXT("Shutting down GAS Companion console manager subsystem"));
	Super::Deinitialize();
}

void UMGCConsoleManagerSubsystem::ShowDebugAbilitySystem(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar, const int32 CategoryIndex)
{
	MGC_LOG(Log, TEXT("UMGCConsoleManagerSubsystem::ShowDebugAbilitySystem Category: %d"), CategoryIndex);

	APlayerController* PC = InWorld->GetFirstPlayerController();
	if (!PC)
	{
		MGC_LOG(Error, TEXT("UMGCConsoleManagerSubsystem:ShowDebugAbilitySystem() Cannot get PlayerController from World"))
		return;
	}

	AHUD* HUD = Cast<AHUD>(PC->GetHUD());
	if (!HUD)
	{
		MGC_LOG(Error, TEXT("UMGCConsoleManagerSubsystem:ShowDebugAbilitySystem() - Cannot get HUD."))
		return;
	}

	HUD->ShowDebug(TEXT("abilitysystem"));
	for (int i = 0; i < CategoryIndex; ++i)
	{
		PC->ConsoleCommand(TEXT("AbilitySystem.Debug.NextCategory"), false);
	}
}

void UMGCConsoleManagerSubsystem::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	// TODO Custom debugger (main thing I'd like to have is to store the last displayed category)
	const bool bAbilitySystemDisplayed = DisplayInfo.IsDisplayOn(TEXT("AbilitySystem"));
	const bool bAttributes = DisplayInfo.IsDisplayOn(TEXT("Attributes"));
	const bool bAbility = DisplayInfo.IsDisplayOn(TEXT("Ability"));
	const bool bGE = DisplayInfo.IsDisplayOn(TEXT("GameplayEffects"));

	// MGC_LOG(Warning, TEXT("UMGCConsoleManagerSubsystem::OnShowDebugInfo - ASC: %s, Attributes: %s, Ability: %s, GameplayEffects: %s"),
	// 	bAbilitySystemDisplayed ? TEXT("true") : TEXT("false"),
	// 	bAttributes ? TEXT("true") : TEXT("false"),
	// 	bAbility ? TEXT("true") : TEXT("false"),
	// 	bGE ? TEXT("true") : TEXT("false")
	// );
	//
	// for (FName Name : HUD->DebugDisplay)
	// {
	// 	MGC_LOG(Warning, TEXT("UMGCConsoleManagerSubsystem::OnShowDebugInfo DebugDisplay: %s"), *Name.ToString());
	// }
	//
	// for (FName Name : HUD->ToggledDebugCategories)
	// {
	// 	MGC_LOG(Warning, TEXT("UMGCConsoleManagerSubsystem::OnShowDebugInfo ToggledDebugCategories: %s"), *Name.ToString());
	// }
}

void UMGCConsoleManagerSubsystem::ToggleComboWidget(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar)
{
	MGC_LOG(Log, TEXT("UMGCConsoleManagerSubsystem::ToggleComboWidget"));

	const APlayerController* PC = InWorld->GetFirstPlayerController();
	if (!PC)
	{
		MGC_LOG(Error, TEXT("UMGCConsoleManagerSubsystem:ToggleComboWidget() Cannot get PlayerController from World"))
		return;
	}

	const APawn* Pawn = PC->GetPawn();
	if (!IsValid(Pawn))
	{
		MGC_LOG(Error, TEXT("UMGCConsoleManagerSubsystem:ToggleComboWidget() Cannot get Pawn from Player Controller"))
		return;
	}

	AGSCHUD* HUD = Cast<AGSCHUD>(PC->GetHUD());
	if (!HUD)
	{
		MGC_SLOG(Error, TEXT("Console Command - Cannot get HUD. Make sure your GameMode is setup to use GSCHUD or a child of it for your HUD."))
		return;
	}

	UGSCComboManagerComponent* Component = Pawn->FindComponentByClass<UGSCComboManagerComponent>();
	if (!Component)
	{
		MGC_SLOG(Error, TEXT("Console Command - Pawn %s doesn't have an ComboManagerComponent. Make sure to add it in Blueprint to your Pawn."), *Pawn->GetName())
		return;
	}

	if (HUD->IsComboWidgetVisible())
	{
		MGC_LOG(Log, TEXT("UMGCConsoleManagerSubsystem:ToggleAbilityQueueWidget() Hide Combo Widget"))
		HUD->HideComboWidget();
	}
	else
	{
		MGC_LOG(Log, TEXT("UMGCConsoleManagerSubsystem:ToggleAbilityQueueWidget() Show Combo Widget"))
		HUD->ShowComboWidget();
	}
}

void UMGCConsoleManagerSubsystem::ToggleAbilityQueueWidget(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar)
{
	const APlayerController* PC = InWorld->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		MGC_LOG(Error, TEXT("UMGCConsoleManagerSubsystem:ToggleAbilityQueueWidget() Cannot get PlayerController from World"))
		return;
	}

	const APawn* Pawn = PC->GetPawn();
	if (!IsValid(Pawn))
	{
		MGC_LOG(Error, TEXT("UMGCConsoleManagerSubsystem:ToggleAbilityQueueWidget() Cannot get Pawn from Player Controller"))
		return;
	}

	AGSCHUD* HUD = Cast<AGSCHUD>(PC->GetHUD());
	if (!IsValid(HUD))
	{
		MGC_SLOG(Error, TEXT("Console Command - Cannot get HUD. Make sure your GameMode is setup to use GSCHUD or a child of it for your HUD."))
		return;
	}

	UGSCAbilityQueueComponent* Component = Pawn->FindComponentByClass<UGSCAbilityQueueComponent>();
	if (!Component)
	{
		MGC_SLOG(Error, TEXT("Console Command - Pawn %s doesn't have an AbilityQueueComponent. Make sure to add it in Blueprint to your Pawn."), *Pawn->GetName())
		return;
	}

	if (HUD->IsAbilityQueueWidgetVisible())
	{
		MGC_LOG(Log, TEXT("UMGCConsoleManagerSubsystem:ToggleAbilityQueueWidget() Hide AbilityQueue Widget"))
		HUD->HideAbilityQueueWidget();
	}
	else
	{
		MGC_LOG(Log, TEXT("UMGCConsoleManagerSubsystem:ToggleAbilityQueueWidget() Show AbilityQueue Widget"))
		HUD->ShowAbilityQueueWidget();
	}
}
