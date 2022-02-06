// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "UI/GSCUserWidget.h"
#include "GameplayEffectTypes.h"
#include "GSCLog.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Components/GSCCoreComponent.h"


void UGSCUserWidget::SetOwnerActor(AActor* Actor)
{
	OwnerActor = Actor;
	OwnerCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Actor);
}

float UGSCUserWidget::GetPercentForAttributes(const FGameplayAttribute Attribute, const FGameplayAttribute MaxAttribute)
{
	if (!OwnerCoreComponent)
	{
		GSC_LOG(Error, TEXT("UGSCUWStatusBar::GetPercentForAttributes() No OwnerCoreComponent, returns 0"))
		return 0.f;
	}

	const float AttributeValue = OwnerCoreComponent->GetAttributeValue(Attribute);
	const float MaxAttributeValue = OwnerCoreComponent->GetAttributeValue(MaxAttribute);

	if (MaxAttributeValue == 0.f)
	{
		GSC_LOG(Warning, TEXT("UGSCUWStatusBar::GetPercentForAttributes() MaxAttribute %s value is 0. This leads to divide by Zero errors, will return 0"), *MaxAttribute.GetName())
		return 0.f;
	}

	return AttributeValue / MaxAttributeValue;
}
