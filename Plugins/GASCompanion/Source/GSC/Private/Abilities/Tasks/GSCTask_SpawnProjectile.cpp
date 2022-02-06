// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Abilities/Tasks/GSCTask_SpawnProjectile.h"

#include "AbilitySystemComponent.h"

UGSCTask_SpawnProjectile* UGSCTask_SpawnProjectile::SpawnProjectile(UGameplayAbility* OwningAbility, const FTransform SpawnTransform, ESpawnActorCollisionHandlingMethod CollisionHandlingOverride, TSubclassOf<AGSCProjectileBase> InClass)
{
	UGSCTask_SpawnProjectile* MyObj = NewAbilityTask<UGSCTask_SpawnProjectile>(OwningAbility);
	MyObj->CachedSpawnTransform = SpawnTransform;
	MyObj->CachedCollisionHandlingOverride = CollisionHandlingOverride;
	return MyObj;
}

// ---------------------------------------------------------------------------------------

bool UGSCTask_SpawnProjectile::BeginSpawningActor(UGameplayAbility* OwningAbility, const TSubclassOf<AGSCProjectileBase> InClass, AGSCProjectileBase*& SpawnedActor)
{
	if (Ability && Ability->GetCurrentActorInfo()->IsNetAuthority() && ShouldBroadcastAbilityTaskDelegates())
	{
		UWorld* const World = GEngine->GetWorldFromContextObject(OwningAbility, EGetWorldErrorMode::LogAndReturnNull);
		if (World)
		{
			AActor* OwningActor = Ability->GetOwningActorFromActorInfo();
			APawn* AvatarPawn = Cast<APawn>(Ability->GetAvatarActorFromActorInfo());
			SpawnedActor = World->SpawnActorDeferred<AGSCProjectileBase>(InClass, CachedSpawnTransform, OwningActor, AvatarPawn, CachedCollisionHandlingOverride);
		}
	}

	if (SpawnedActor == nullptr)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(nullptr);
		}
		return false;
	}

	return true;
}

void UGSCTask_SpawnProjectile::FinishSpawningActor(UGameplayAbility* OwningAbility, AGSCProjectileBase* SpawnedActor)
{
	if (SpawnedActor)
	{
		SpawnedActor->FinishSpawning(CachedSpawnTransform);

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			Success.Broadcast(SpawnedActor);
		}
	}

	EndTask();
}
