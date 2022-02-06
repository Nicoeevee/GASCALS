// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "MGCConsoleManagerSubsystem.generated.h"

/**
 *
 */
UCLASS()
class MODULARGASCOMPANION_API UMGCConsoleManagerSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	//~UEngineSubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of UEngineSubsystem interface

private:

	void ToggleComboWidget(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar);
	void ToggleAbilityQueueWidget(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar);
	void ShowDebugAbilitySystem(const TArray<FString>& Args, UWorld* InWorld, FOutputDevice& Ar, int32 CategoryIndex);

	// Called by gameplay debugger
	static void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
};
