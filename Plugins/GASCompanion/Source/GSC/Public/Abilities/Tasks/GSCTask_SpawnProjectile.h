// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Actors/Projectiles/GSCProjectileBase.h"

#include "GSCTask_SpawnProjectile.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpawnActorDelegate, AActor*, SpawnedActor);

/**
*	Convenience task for spawning actor projectiles on the network authority.
*/
UCLASS()
class GASCOMPANION_API UGSCTask_SpawnProjectile : public UAbilityTask
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FSpawnActorDelegate	Success;

	/** Called when we can't spawn: on clients or potentially on server if they fail to spawn (rare) */
	UPROPERTY(BlueprintAssignable)
	FSpawnActorDelegate	DidNotSpawn;

	/** Spawn new projectile on the network authority (server) */
	UFUNCTION(BlueprintCallable, meta=(HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category="Ability|GAS Companion|Tasks")
	static UGSCTask_SpawnProjectile* SpawnProjectile(UGameplayAbility* OwningAbility, FTransform  SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, TSubclassOf<AGSCProjectileBase> Class);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Abilities")
	bool BeginSpawningActor(UGameplayAbility* OwningAbility, TSubclassOf<AGSCProjectileBase> Class, AGSCProjectileBase*& SpawnedActor);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Abilities")
	void FinishSpawningActor(UGameplayAbility* OwningAbility, AGSCProjectileBase* SpawnedActor);

protected:
	FGameplayAbilityTargetDataHandle CachedTargetDataHandle;
	FTransform CachedSpawnTransform;
	ESpawnActorCollisionHandlingMethod CachedCollisionHandlingOverride;
};
