// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ModularPlayerStateCharacter.generated.h"

class UMGCAbilitySystemComponent;

/**
 * Minimal class that supports extension by game feature plugins.
 *
 * Intended to be used for ACharacters using AbilitySystemComponent living on PlayerState.
 */
UCLASS(Blueprintable)
class MODULARGASCOMPANION_API AModularPlayerStateCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FMGCCharacterRestartedSignature, AModularPlayerStateCharacter, ReceiveCharacterRestartedDelegate, APawn*, Pawn);
	DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams(FMGCCharacterControllerChangedSignature, AModularPlayerStateCharacter, ReceiveCharacterControllerChangedDelegate, APawn*, Pawn, AController*, OldController, AController*, NewController);

public:
	AModularPlayerStateCharacter(const FObjectInitializer& ObjectInitializer);

	// Cached AbilitySystemComponent. Real owner is PlayerState, but pointer gets updated to use PlayerState's here in PossessedBy / OnRep_PlayerState
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Event called after a pawn has been restarted, usually by a possession change. This is called on the server for all pawns and the owning client for player pawns */
	UPROPERTY(BlueprintAssignable, Category = "Modular GAS Companion|Character")
	FMGCCharacterRestartedSignature ReceiveCharacterRestartedDelegate;

	/** Event called after a pawn's controller has changed, on the server and owning client. This will happen at the same time as the delegate on GameInstance */
	UPROPERTY(BlueprintAssignable, Category = "Modular GAS Companion|Character")
	FMGCCharacterControllerChangedSignature ReceiveCharacterControllerChangedDelegate;

	/** Previous controller that was controlling this pawn since the last controller change notification */
	UPROPERTY(transient)
	AController* PreviousCharacterController;

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

	//~ Begin AActor Interface
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~ End AActor Interface

	//~ Begin APawn Interface
	virtual void Restart() override;
	virtual void PawnClientRestart() override;

	virtual void OnRep_Controller() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	//~ End APawn Interface

	/** Event called after a pawn has been restarted, usually by a possession change. This is called on the server for all pawns and the owning client for player pawns */
	UFUNCTION(BlueprintImplementableEvent, Category = "Modular GAS Companion|Character")
	void ReceiveCharacterRestarted();

	/** Event called after a pawn's controller has changed, on the server and owning client. This will happen at the same time as the delegate on GameInstance */
	UFUNCTION(BlueprintImplementableEvent, Category = "Modular GAS Companion|Character")
	void ReceiveCharacterControllerChanged(AController* OldController, AController* NewController);

	/**
	* Notifies other systems that a pawn has been restarted. By default this is called on the server for all pawns and the owning client for player pawns.
	* This can be overridden by subclasses to delay the notification of restart until data has loaded/replicated
	*/
	virtual void NotifyCharacterRestarted();

	/** Call to notify about a change in controller, on both the server and owning client. This calls the above event and delegate */
	virtual void NotifyCharacterControllerChanged();
};
