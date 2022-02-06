// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Animation/MGCLinkAnimLayersComponent.h"
#include "ModularGASCompanionLog.h"
#include "GameFramework/Character.h"

void UMGCLinkAnimLayersComponent::BeginPlay()
{
	MGC_LOG(Log, TEXT("Link Anim Layers Component Begin Play"))

	const ACharacter* Owner = GetPawn<ACharacter>();
	if (IsValid(Owner))
	{
		OwnerSkeletalMeshComponent = Owner->GetMesh();
		LinkAnimLayers();
	}

	Super::BeginPlay();
}

void UMGCLinkAnimLayersComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	MGC_LOG(Log, TEXT("Link Anim Layers Component End Play"))
	UnlinkAnimLayers();

	Super::EndPlay(EndPlayReason);
}

void UMGCLinkAnimLayersComponent::LinkAnimLayer(const TSubclassOf<UAnimInstance> AnimInstance)
{
	if (IsValid(OwnerSkeletalMeshComponent))
	{
		MGC_LOG(Log, TEXT("Linking Anim Layer %s"), *AnimInstance->GetName())
		OwnerSkeletalMeshComponent->LinkAnimClassLayers(AnimInstance);
	}
}

void UMGCLinkAnimLayersComponent::UnlinkAnimLayer(const TSubclassOf<UAnimInstance> AnimInstance)
{
	if (IsValid(OwnerSkeletalMeshComponent))
	{
		OwnerSkeletalMeshComponent->UnlinkAnimClassLayers(AnimInstance);
	}
}

void UMGCLinkAnimLayersComponent::LinkAnimLayers()
{
	for (const TSubclassOf<UAnimInstance> LayerType : LayerTypes)
	{
		LinkAnimLayer(LayerType);
	}
}

void UMGCLinkAnimLayersComponent::UnlinkAnimLayers()
{
	for (const TSubclassOf<UAnimInstance> LayerType : LayerTypes)
	{
		UnlinkAnimLayer(LayerType);
	}
}
