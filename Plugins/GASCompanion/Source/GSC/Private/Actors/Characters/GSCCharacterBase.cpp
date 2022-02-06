// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Actors/Characters/GSCCharacterBase.h"
#include "GameplayEffectExtension.h"
#include "Abilities/GSCGameplayAbility.h"
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "UI/GSCUWDebugAbilityQueue.h"
#include "Player/GSCHUD.h"
#include "GSCLog.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCComboManagerComponent.h"
#include "Components/GSCCoreComponent.h"

// Sets default values
AGSCCharacterBase::AGSCCharacterBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
     // Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    // Setup components
	GSCCoreComponent = CreateDefaultSubobject<UGSCCoreComponent>("GSCCoreComponent");
	GSCAbilityQueueComponent = CreateDefaultSubobject<UGSCAbilityQueueComponent>("GSCAbilityQueueComponent");
	GSCComboComponent = CreateDefaultSubobject<UGSCComboManagerComponent>("GSCComboComponent");

    // Setup sensible defaults
	bUseControllerRotationYaw = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	NetPriority = 4.0f;

    USkeletalMeshComponent* MeshComponent = GetMesh();

    MeshComponent->bEnableUpdateRateOptimizations = true;
    MeshComponent->bPropagateCurvesToSlaves = true;

    // Always tick Pose and refresh Bones!
    MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

UGSCAbilitySystemComponent* AGSCCharacterBase::GetASC()
{
    return UGSCAbilitySystemComponent::GetAbilitySystemComponentFromActor(this);
}

void AGSCCharacterBase::Restart()
{
    GSC_VLOG(this, Log, TEXT("Restart Character %s"), *GetName())

    Super::Restart();
    OnRespawn();
}

UAbilitySystemComponent* AGSCCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent.Get();
}

UGSCCoreComponent* AGSCCharacterBase::GetCompanionCoreComponent() const
{
    return GSCCoreComponent;
}

UGSCComboManagerComponent* AGSCCharacterBase::GetComboManagerComponent() const
{
    return GSCComboComponent;
}

UGSCAbilityQueueComponent* AGSCCharacterBase::GetAbilityQueueComponent() const
{
    return GSCAbilityQueueComponent;
}

UGSCUWDebugAbilityQueue* AGSCCharacterBase::GetDebugWidgetFromHUD()
{
    // TODO: Cache HUDWidget ?
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return nullptr;
    }

    AGSCHUD* HUD = PC->GetHUD<AGSCHUD>();
    if (!HUD)
    {
        return nullptr;
    }

    return Cast<UGSCUWDebugAbilityQueue>(HUD->GetAbilityQueueWidget());
}
