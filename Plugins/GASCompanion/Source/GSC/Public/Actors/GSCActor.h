// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "Core/Interfaces/GSCCompanionInterface.h"

#include "GSCActor.generated.h"

class UGSCAbilitySystemComponent;
class UGSCCoreComponent;

UCLASS()
class GASCOMPANION_API AGSCActor : public AActor, public IAbilitySystemInterface, public IGSCCompanionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGSCActor();

	// Setup AttributeSets here
	virtual void PreInitializeComponents() override;

	/** Companion Core Component */
	UPROPERTY(Category = "GAS Companion|Components", VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UGSCCoreComponent* GSCCoreComponent;

	/** Actor AbilitySystemComponent */
	UPROPERTY(Category=AbilitySystem, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UGSCAbilitySystemComponent* AbilitySystemComponent;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UPROPERTY()
	TArray<const class UAttributeSet*> AttributeSets;

	// Keep track of registered attribute sets to avoid adding it twice
	UPROPERTY()
	TArray<FString> RegisteredAttributeSetNames;

	// Implement IGSCCompanionInterface
	virtual UGSCCoreComponent* GetCompanionCoreComponent() const override;
	virtual UGSCComboManagerComponent* GetComboManagerComponent() const override;
	virtual UGSCAbilityQueueComponent* GetAbilityQueueComponent() const override;
	// ~ IGSCCompanionInterface

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Create and register attribute sets */
	void SetupAttributeSets();
};
