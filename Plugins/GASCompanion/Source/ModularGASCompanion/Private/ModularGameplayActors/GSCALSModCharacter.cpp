// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "ModularGameplayActors/GSCALSModCharacter.h"

#include "Abilities/MGCAbilitySystemComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "ModularGameplayActors/MGCGameFrameworkExtensionManager.h"
#include "ModularGASCompanionLog.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/GSCPlayerController.h"
#include "Player/GSCPlayerState.h"


AGSCALSModCharacter::AGSCALSModCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup sensible defaults
	bUseControllerRotationYaw = false;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	NetPriority = 4.0f;

	USkeletalMeshComponent* MeshComponent = GetMesh();

	MeshComponent->bEnableUpdateRateOptimizations = true;
	MeshComponent->bPropagateCurvesToSlaves = true;

	// Always tick Pose and refresh Bones!
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// Setup ALS defaults
	Tags.Add(FName("ALS_Character"));
	bRightShoulder = true;
	bRagdollOnLand = true;
	MovementModel.DataTable = LoadObject<UDataTable>(nullptr, TEXT("/ALSV4_CPP/AdvancedLocomotionV4/Data/DataTables/MovementModelTable"));
	MovementModel.RowName = FName("Normal");
	MeshComponent->bUpdateJointsFromAnimation = true;
	MeshComponent->bEnableUpdateRateOptimizations = true;

	UCharacterMovementComponent* CMC = GetCharacterMovement();
	CMC->MaxAcceleration = 1500.f;
	CMC->BrakingFrictionFactor = 0.f;
	CMC->CrouchedHalfHeight = 60.f;
	CMC->MinAnalogWalkSpeed = 25.f;
	CMC->bCanWalkOffLedgesWhenCrouching = true;
	CMC->AirControl = 0.15f;
	CMC->BrakingDecelerationFlying = 1000.f;
	CMC->NavAgentProps.bCanCrouch = true;
	CMC->NavAgentProps.bCanFly = true;

	AbilitySystemComponent = CreateDefaultSubobject<UMGCAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	// Replication Mode is set in PostInitProperties to allow users to change the default value from BP
}

UAbilitySystemComponent* AGSCALSModCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AGSCALSModCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::AddGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::AddGameFrameworkComponentReceiver(this);
#endif
}

void AGSCALSModCharacter::BeginPlay()
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, UGameFrameworkComponentManager::NAME_GameActorReady);
	Super::BeginPlay();
#else
	// One of the difference with 5.0 is that component requests are added here only if ActorHasBegunPlay, whereas in 5.0 it has been changed slighly
	// to check if ActorHasBeenInitialized. So to workaround that, we call Super::BeginPlay before
	Super::BeginPlay();
	UMGCGameFrameworkExtensionManager::SendGameFrameworkComponentExtensionEvent(this, UMGCGameFrameworkExtensionManager::MGC_NAME_GameActorReady);
#endif
}

void AGSCALSModCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if ENGINE_MAJOR_VERSION == 5
	UGameFrameworkComponentManager::RemoveGameFrameworkComponentReceiver(this);
#else
	UMGCGameFrameworkExtensionManager::RemoveGameFrameworkComponentReceiver(this);
#endif

	Super::EndPlay(EndPlayReason);
}

void AGSCALSModCharacter::PostInitProperties()
{
	Super::PostInitProperties();
	if (AbilitySystemComponent)
	{
		MGC_LOG(Verbose, TEXT("AModularCharacter::PostInitProperties for %s - Setting up ASC Replication Mode to: %d"), *GetName(), ReplicationMode)
		AbilitySystemComponent->SetReplicationMode(ReplicationMode);
	}
}

void AGSCALSModCharacter::Restart()
{
	Super::Restart();
	NotifyCharacterRestarted();
}

void AGSCALSModCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	NotifyCharacterRestarted();
}

void AGSCALSModCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	bool bNotifyControllerChange = (Controller == nullptr);
	if ((Controller != nullptr) && (Controller->GetPawn() == nullptr))
	{
		bNotifyControllerChange = true;
	}

	if (bNotifyControllerChange)
	{
		NotifyCharacterControllerChanged();
	}
}

void AGSCALSModCharacter::PossessedBy(AController* NewController)
{
	AController* OldController = Controller;
	Super::PossessedBy(NewController);

	// dispatch controller changed Blueprint event if necessary, along with ReceiveCharacterControllerChangedDelegate
	if (OldController != NewController)
	{
		NotifyCharacterControllerChanged();
	}
}

void AGSCALSModCharacter::UnPossessed()
{
	Super::UnPossessed();

	// TODO: ConsumeMovementInputVector() is called before we notify controller changed, whereas in 5.0 NotifyControllerChanged() happens just before
	NotifyCharacterControllerChanged();
}

void AGSCALSModCharacter::NotifyCharacterRestarted()
{
	ReceiveCharacterRestarted();
	ReceiveCharacterRestartedDelegate.Broadcast(this);
}

void AGSCALSModCharacter::NotifyCharacterControllerChanged()
{
	ReceiveCharacterControllerChanged(PreviousCharacterController, Controller);
	ReceiveCharacterControllerChangedDelegate.Broadcast(this, PreviousCharacterController, Controller);

	// Update the cached controller
	PreviousCharacterController = Controller;
}

// void AGSCALSModCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
// {
// 	Super::SetupPlayerInputComponent(PlayerInputComponent);
//
// 	// Bind player input to the AbilitySystemComponent. Also called in OnRep_PlayerState because of a potential race condition.
// 	BindASCInput();
// }

// void AGSCALSModCharacter::BindASCInput()
// {
// 	if (bASCInputBound)
// 	{
// 		return;
// 	}
// 	if (IsValid(AbilitySystemComponent) && IsValid(InputComponent))
// 	{
// 		AbilitySystemComponent->BindAbilityActivationToInputComponent(
// 			InputComponent,
// 			FGameplayAbilityInputBinds(FString("ConfirmTarget"),
// 			                           FString("CancelTarget"),
// 			                           FString("EGSCAbilityInputID"),
// 			                           static_cast<int32>(EGSCAbilityInputID::Confirm),
// 			                           static_cast<int32>(EGSCAbilityInputID::Cancel))
// 		);
//
// 		bASCInputBound = true;
// 	}
// }
