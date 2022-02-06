// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Actors/Projectiles/GSCProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
AGSCProjectileBase::AGSCProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(FName("ProjectileMovement"));

}

// Called when the game starts or when spawned
void AGSCProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}
