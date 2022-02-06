// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffect.h"
#include "GSCProjectileBase.generated.h"

UCLASS()
class GASCOMPANION_API AGSCProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGSCProjectileBase();

	/** Projectile movement component */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="GAS Companion|Projectile")
	class UProjectileMovementComponent* ProjectileMovement;

	/** Whether to draw debug traces on explosion sphere */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	bool bDebugTrace = false;

	/** Whether to display the static mesh in game */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	bool bStaticMeshHiddenInGame = false;

	/** Radius of the explosion sphere to detect overlapped pawns */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	float ExplosionTraceRadius = 200.f;

	/** Initial speed of projectile */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	float ProjectileInitialSpeed = 3000.f;

	/** Gravity Scale for this projectile. Set to 0 to disable gravity */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	float ProjectileGravityScale = 5.f;

	/** The amount of time (in seconds) before destroying this projectile on explosion, to let the explosion cue vfx play */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	float DestroyDelay = 1.f;

	/** The GameplayTag associated with the GameplayCue to add for projectile (attached to root component on begin play) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	FGameplayTag ProjectileGameplayCueTag;

	/** The GameplayTag associated with the GameplayCue to add for explosion (triggered when projectile overlap pawns or hit WorldStatic/Dynamic) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	FGameplayTag ExplosionGameplayCueTag;

	/** The Event GameplayTag to send back to instigator to end the ability when the projectile is destroyed (after DestroyDelay on explosion) */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	FGameplayTag EndAbilityGameplayEventTag;

	/** The GameplayEffect spec handle to apply to overlapped actors */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category="GAS Companion|Projectile")
	FGameplayEffectSpecHandle DamageEffectSpecHandle;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
