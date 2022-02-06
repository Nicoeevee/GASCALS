// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ModularCharacter.generated.h"

class UMGCAbilitySystemComponent;

/**
 * Minimal class that supports extension by game feature plugins
 *
 * Intended to be used for ACharacters using AbilitySystemComponent living on Pawns
 */
UCLASS(Blueprintable)
class MODULARGASCOMPANION_API AModularCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam(FMGCCharacterRestartedSignature, AModularCharacter, ReceiveCharacterRestartedDelegate, APawn*, Pawn);
	DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_ThreeParams(FMGCCharacterControllerChangedSignature, AModularCharacter, ReceiveCharacterControllerChangedDelegate, APawn*, Pawn, AController*, OldController, AController*, NewController);


public:
	AModularCharacter(const FObjectInitializer& ObjectInitializer);

	/**
	* Ability System Replication Mode: How gameplay effects will be replicated to clients
	*
	* - Full: Replicate full gameplay info to all. Every GameplayEffect is replicated to every client.
	* (Recommended for Single Player games)
	* - Mixed: Only replicate minimal gameplay effect info to simulated proxies but full info to owners and autonomous proxies.
	* GameplayEffects are only replicated to the owning client. Only GameplayTags and GameplayCues are replicated to everyone.
	* (Recommended for Multiplayer on Player controlled Actors)
	* - Minimal: Only replicate minimal gameplay effect info. Note: this does not work for Owned AbilitySystemComponents (Use Mixed instead).
	* GameplayEffects are never replicated to anyone. Only GameplayTags and GameplayCues are replicated to everyone.
	* (Recommended for Multiplayer on AI controlled Actors)
	*
	* @See https://github.com/tranek/GASDocumentation#concepts-asc-rm for more information
	*/
	UPROPERTY(EditDefaultsOnly, Category="Modular GAS Companion|Ability System")
	EGameplayEffectReplicationMode ReplicationMode = EGameplayEffectReplicationMode::Mixed;

	UPROPERTY(Category=Character, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UMGCAbilitySystemComponent* AbilitySystemComponent;

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
	virtual void PostInitProperties() override;
	//~ End AActor Interface


	//~ Begin APawn Interface
	virtual void Restart() override;
	virtual void PawnClientRestart() override;

	virtual void OnRep_Controller() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
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
