// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Abilities/Attributes/GSCAttributeSetBase.h"
#include "GameplayEffectExtension.h"
#include "Actors/Characters/GSCCharacterBase.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Components/GSCCoreComponent.h"
#include "Player/GSCPlayerState.h"

// Sets default values
UGSCAttributeSetBase::UGSCAttributeSetBase()
{
	// Set default values for this Set Attributes here
    const UGSCDeveloperSettings* DeveloperSettings = GetDefault<UGSCDeveloperSettings>();
    for (FGSCAttributeSetMinimumValues ClampMinimumValue : DeveloperSettings->MinimumValues)
    {
        // For each configured value, store and cache them in a map
        MinimumValues.Add(ClampMinimumValue.Attribute, ClampMinimumValue.MinimumValue);
    }
}

void UGSCAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // This is called whenever attributes change, so for max attributes we want to scale the current totals to match
    Super::PreAttributeChange(Attribute, NewValue);

    const FGameplayAbilityActorInfo* ActorInfo = GetActorInfo();
	if (!ActorInfo)
	{
		return;
	}

    const TWeakObjectPtr<AActor> AvatarActorPtr = ActorInfo->AvatarActor;
	if (!AvatarActorPtr.IsValid())
	{
		return;
	}


	// TODO: Figure out PlayerState
    const AActor* AvatarActor = AvatarActorPtr.Get();
    AGSCPlayerState* PS = Cast<AGSCPlayerState>(AvatarActorPtr.Get());
    if (PS)
    {
        AvatarActor = PS->GetPawn();
    }

    UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(AvatarActor);
    if (CoreComponent)
    {
        CoreComponent->PreAttributeChange(this, Attribute, NewValue);
    }
}

void UGSCAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    AActor* SourceActor = nullptr;
    AActor* TargetActor = nullptr;
    GetSourceAndTargetFromContext<AActor>(Data, SourceActor, TargetActor);

    UGSCCoreComponent* TargetCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(TargetActor);
    if (TargetCoreComponent)
    {
        TargetCoreComponent->PostGameplayEffectExecute(this, Data);
    }
}

void UGSCAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

float UGSCAttributeSetBase::GetClampMinimumValueFor(const FGameplayAttribute& Attribute)
{
    float* MinimumValue = MinimumValues.Find(Attribute);
    if (!MinimumValue)
    {
        return 0.f;
    }

    return *MinimumValue;
}

void UGSCAttributeSetBase::GetCharactersFromContext(const FGameplayEffectModCallbackData& Data, AGSCCharacterBase*& SourceCharacter, AGSCCharacterBase*& TargetCharacter)
{
    GetSourceAndTargetFromContext<AGSCCharacterBase>(Data, SourceCharacter, TargetCharacter);
}

const FGameplayTagContainer& UGSCAttributeSetBase::GetSourceTagsFromContext(const FGameplayEffectModCallbackData& Data)
{
    return *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
}

void UGSCAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, const float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
{
    UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponent();
    const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
    if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilitySystemComponent)
    {
        // Change current value to maintain the current Val / Max percent
        const float CurrentValue = AffectedAttribute.GetCurrentValue();
        const float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

        AbilitySystemComponent->ApplyModToAttribute(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
    }
}

void UGSCAttributeSetBase::GetExecutionDataFromMod(const FGameplayEffectModCallbackData& Data, FGSCAttributeSetExecutionData& OutExecutionData)
{
	OutExecutionData.Context = Data.EffectSpec.GetContext();
	OutExecutionData.SourceASC = OutExecutionData.Context.GetOriginalInstigatorAbilitySystemComponent();
	OutExecutionData.SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	Data.EffectSpec.GetAllAssetTags(OutExecutionData.SpecAssetTags);

	OutExecutionData.TargetActor = Data.Target.AbilityActorInfo->AvatarActor.IsValid() ? Data.Target.AbilityActorInfo->AvatarActor.Get() : nullptr;
	OutExecutionData.TargetController = Data.Target.AbilityActorInfo->PlayerController.IsValid() ? Data.Target.AbilityActorInfo->PlayerController.Get() : nullptr;
	OutExecutionData.TargetPawn = Cast<APawn>(OutExecutionData.TargetActor);
	OutExecutionData.TargetCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(OutExecutionData.TargetActor);

	if (OutExecutionData.SourceASC && OutExecutionData.SourceASC->AbilityActorInfo.IsValid())
	{
		// Get the Source actor, which should be the damage causer (instigator)
		if (OutExecutionData.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
		{
			// Set the source actor based on context if it's set
			if (OutExecutionData.Context.GetEffectCauser())
			{
				OutExecutionData.SourceActor = OutExecutionData.Context.GetEffectCauser();
			}
			else
			{
				OutExecutionData.SourceActor = OutExecutionData.SourceASC->AbilityActorInfo->AvatarActor.IsValid()
					? OutExecutionData.SourceASC->AbilityActorInfo->AvatarActor.Get()
					: nullptr;
			}
		}

		OutExecutionData.SourceController = OutExecutionData.SourceASC->AbilityActorInfo->PlayerController.IsValid()
			? OutExecutionData.SourceASC->AbilityActorInfo->PlayerController.Get()
			: nullptr;
		OutExecutionData.SourcePawn = Cast<APawn>(OutExecutionData.SourceActor);
		OutExecutionData.SourceCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(OutExecutionData.SourceActor);
	}

	OutExecutionData.SourceObject = Data.EffectSpec.GetEffectContext().GetSourceObject();

	// Compute the delta between old and new, if it is available
	OutExecutionData.DeltaValue = 0.f;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		// If this was additive, store the raw delta value to be passed along later
		OutExecutionData.DeltaValue = Data.EvaluatedData.Magnitude;
	}
}
