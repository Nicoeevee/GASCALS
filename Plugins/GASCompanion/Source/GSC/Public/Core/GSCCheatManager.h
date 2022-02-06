// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"

#include "GSCCheatManager.generated.h"


class UGSCUWDebugAbilityQueue;

/**
 *
 */
UCLASS()
class GASCOMPANION_API UGSCCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:

	/**
	 * Toggles the display of Ability Queue System Debug Widget
	 */
	UFUNCTION(exec)
	void GSC_AbilityQueueDebug();

	/**
	 * Runs command "showdebug AbilitySystem"
	 */
	UFUNCTION(exec)
	void GSC_ShowDebugAbilitySystem() const;

	/**
	 * Runs command "AbilitySystem.Debug.NextCategory"
	 */
	UFUNCTION(exec)
	void GSC_AbilitySystemNextCategory() const;

	/**
	* Toggles the display of Combo Debug Widget
	*/
	UFUNCTION(exec)
	void GSC_OpenComboDebug();

protected:
	// Little helper to execute command via PlayerController
	void ExecuteConsoleCommand(FString Command) const;
};
