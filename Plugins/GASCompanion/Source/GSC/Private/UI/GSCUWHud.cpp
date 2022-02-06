// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "UI/GSCUWHud.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "GSCLog.h"

void UGSCUWHud::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeHUD();
}

void UGSCUWHud::NativeDestruct()
{
	// Clean up previously registered delegates for OwningPlayer AbilitySystemComponent
	ShutdownAbilitySystemComponentListeners();

	Super::NativeDestruct();
}

void UGSCUWHud::InitializeHUD(const bool bRegisterDelegates)
{
	// Setup refs needed for this UserWidget to react to ASC delegates
	SetupAbilitySystemFromOwningPlayerPawn();

	// Setup delegates for OwningPlayer AbilitySystemComponent
	SetOwnerActor(OwningPawn);

	// Init Stats
	InitFromCharacter();

	// Setup ASC delegates to react to various events
	if (bRegisterDelegates)
	{
		RegisterAbilitySystemDelegates();
	}
}

void UGSCUWHud::SetupAbilitySystemFromOwningPlayerPawn()
{
	OwningPawn = GetOwningPlayerPawn();
	if (!OwningPawn)
	{
		GSC_LOG(Error, TEXT("UGSCUWHud::NativeConstruct - Owning Player Pawn is invalid"))
	}

	OwningAbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningPawn);
}

void UGSCUWHud::RegisterAbilitySystemDelegates()
{
	if (!OwningAbilitySystemComponent)
	{
		// Ability System may not have been available yet for character (PlayerState setup on clients)
		return;
	}

	TArray<FGameplayAttribute> Attributes;
	OwningAbilitySystemComponent->GetAllAttributes(Attributes);

	for (FGameplayAttribute Attribute : Attributes)
	{
		GSC_LOG(Verbose, TEXT("UGSCUWHud::SetupAbilitySystemComponentListeners - Setup callback for %s (%s)"), *Attribute.GetName(), *OwningPawn->GetName());
		OwningAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UGSCUWHud::OnAttributeChanged);
	}

	// Handle GameplayEffects added / remove
	OwningAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UGSCUWHud::OnActiveGameplayEffectAdded);
	OwningAbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UGSCUWHud::OnAnyGameplayEffectRemoved);

	// Handle generic GameplayTags added / removed
	OwningAbilitySystemComponent->RegisterGenericGameplayTagEvent().AddUObject(this, &UGSCUWHud::OnAnyGameplayTagChanged);

	// Handle Ability Commit events
	OwningAbilitySystemComponent->AbilityCommittedCallbacks.AddUObject(this, &UGSCUWHud::OnAbilityCommitted);
}

void UGSCUWHud::ShutdownAbilitySystemComponentListeners() const
{
	if(!OwningAbilitySystemComponent)
	{
		return;
	}

	TArray<FGameplayAttribute> Attributes;
	OwningAbilitySystemComponent->GetAllAttributes(Attributes);

	for (const FGameplayAttribute Attribute : Attributes)
	{
		OwningAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
	}

	OwningAbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
	OwningAbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);
	OwningAbilitySystemComponent->RegisterGenericGameplayTagEvent().RemoveAll(this);
	OwningAbilitySystemComponent->AbilityCommittedCallbacks.RemoveAll(this);

	for (const FActiveGameplayEffectHandle GameplayEffectAddedHandle : GameplayEffectAddedHandles)
	{
		if (GameplayEffectAddedHandle.IsValid())
		{
			FOnActiveGameplayEffectStackChange* EffectStackChangeDelegate = OwningAbilitySystemComponent->OnGameplayEffectStackChangeDelegate(GameplayEffectAddedHandle);
			if (EffectStackChangeDelegate)
			{
				EffectStackChangeDelegate->RemoveAll(this);
			}

			FOnActiveGameplayEffectTimeChange* EffectTimeChangeDelegate = OwningAbilitySystemComponent->OnGameplayEffectTimeChangeDelegate(GameplayEffectAddedHandle);
			if (EffectTimeChangeDelegate)
			{
				EffectTimeChangeDelegate->RemoveAll(this);
			}
		}
	}

	for (const FGameplayTag GameplayTagBoundToDelegate : GameplayTagBoundToDelegates)
	{
		OwningAbilitySystemComponent->RegisterGameplayTagEvent(GameplayTagBoundToDelegate).RemoveAll(this);
	}
}

void UGSCUWHud::OnAttributeChanged(const FOnAttributeChangeData& Data)
{
	HandleAttributeChange(Data.Attribute, Data.NewValue, Data.OldValue);
	// TODO - Maybe find a way to prevent regen from triggering attribute change when it reaches higher / lower bounds of clamping instead ?
	// if (Data.NewValue != Data.OldValue)
	// {
	// 	HandleAttributeChange(Data.Attribute, Data.NewValue, Data.OldValue);
	// }
}

void UGSCUWHud::OnActiveGameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, const FActiveGameplayEffectHandle ActiveHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (OwningAbilitySystemComponent)
	{
		OwningAbilitySystemComponent->OnGameplayEffectStackChangeDelegate(ActiveHandle)->AddUObject(this, &UGSCUWHud::OnActiveGameplayEffectStackChanged);
		OwningAbilitySystemComponent->OnGameplayEffectTimeChangeDelegate(ActiveHandle)->AddUObject(this, &UGSCUWHud::OnActiveGameplayEffectTimeChanged);

		// Store active handles to clear out bound delegates when shutting down listeners
		GameplayEffectAddedHandles.AddUnique(ActiveHandle);
	}

	HandleGameplayEffectAdded(AssetTags, GrantedTags, ActiveHandle);
}

void UGSCUWHud::OnActiveGameplayEffectStackChanged(const FActiveGameplayEffectHandle ActiveHandle, const int32 NewStackCount, const int32 PreviousStackCount)
{
	if (!OwningAbilitySystemComponent)
	{
		return;
	}

	const FActiveGameplayEffect* GameplayEffect = OwningAbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle);
	if (!GameplayEffect)
	{
		return;
	}

	FGameplayTagContainer AssetTags;
	GameplayEffect->Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	GameplayEffect->Spec.GetAllGrantedTags(GrantedTags);

	HandleGameplayEffectStackChange(AssetTags, GrantedTags, ActiveHandle, NewStackCount, PreviousStackCount);
}

void UGSCUWHud::OnActiveGameplayEffectTimeChanged(const FActiveGameplayEffectHandle ActiveHandle, const float NewStartTime, const float NewDuration)
{
	if (!OwningAbilitySystemComponent)
	{
		return;
	}

	const FActiveGameplayEffect* GameplayEffect = OwningAbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle);
	if (!GameplayEffect)
	{
		return;
	}

	FGameplayTagContainer AssetTags;
	GameplayEffect->Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	GameplayEffect->Spec.GetAllGrantedTags(GrantedTags);

	HandleGameplayEffectTimeChange(AssetTags, GrantedTags, ActiveHandle, NewStartTime, NewDuration);
}

void UGSCUWHud::OnAnyGameplayEffectRemoved(const FActiveGameplayEffect& EffectRemoved)
{
	if (!OwningAbilitySystemComponent)
	{
		return;
	}

	FGameplayTagContainer AssetTags;
	EffectRemoved.Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	EffectRemoved.Spec.GetAllGrantedTags(GrantedTags);

	// Broadcast any GameplayEffect change to HUD
	HandleGameplayEffectStackChange(AssetTags, GrantedTags, EffectRemoved.Handle, 0, 1);
	HandleGameplayEffectRemoved(AssetTags, GrantedTags, EffectRemoved.Handle);
}

void UGSCUWHud::OnAnyGameplayTagChanged(const FGameplayTag GameplayTag, const int32 NewCount)
{
	HandleGameplayTagChange(GameplayTag, NewCount);
}

void UGSCUWHud::OnAbilityCommitted(UGameplayAbility* ActivatedAbility)
{
	if (!OwningAbilitySystemComponent)
	{
		return;
	}

	if (!IsValid(ActivatedAbility))
	{
		GSC_LOG(Warning, TEXT("UGSCUWHud::OnAbilityCommitted() Activated ability not valid"))
		return;
	}

	// Figure out cooldown
	UGameplayEffect* CooldownGE = ActivatedAbility->GetCooldownGameplayEffect();
	if (!CooldownGE)
	{
		return;
	}

	if (!ActivatedAbility->IsInstantiated())
	{
		return;
	}

	const FGameplayTagContainer* CooldownTags = ActivatedAbility->GetCooldownTags();
	if (!CooldownTags || CooldownTags->Num() <= 0)
	{
		return;
	}

	const FGameplayAbilityActorInfo ActorInfo = ActivatedAbility->GetActorInfo();
	const FGameplayAbilitySpecHandle AbilitySpecHandle = ActivatedAbility->GetCurrentAbilitySpecHandle();

	float TimeRemaining = 0.f;
	float Duration = 0.f;
	ActivatedAbility->GetCooldownTimeRemainingAndDuration(AbilitySpecHandle, &ActorInfo, TimeRemaining, Duration);

	// Broadcast start of cooldown to HUD
	const FGameplayAbilitySpec* AbilitySpec = OwningAbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (AbilitySpec)
	{
		HandleCooldownStart(AbilitySpec->Ability, *CooldownTags, TimeRemaining, Duration);
	}

	// Register delegate to monitor any change to cooldown gameplay tag to be able to figure out when a cooldown expires
	TArray<FGameplayTag> GameplayTags;
	CooldownTags->GetGameplayTagArray(GameplayTags);
	for (const FGameplayTag GameplayTag : GameplayTags)
	{

		FOnGameplayEffectTagCountChanged CountChangedDelegate = OwningAbilitySystemComponent->RegisterGameplayTagEvent(GameplayTag);

		// Add a change callback if it isn't on it already
		if (CountChangedDelegate.IsBoundToObject(this))
		{
			CountChangedDelegate.AddUObject(this, &UGSCUWHud::OnCooldownGameplayTagChanged, AbilitySpecHandle, Duration);
			GameplayTagBoundToDelegates.AddUnique(GameplayTag);
		}
	}
}

void UGSCUWHud::OnCooldownGameplayTagChanged(const FGameplayTag GameplayTag, const int32 NewCount, FGameplayAbilitySpecHandle AbilitySpecHandle, float Duration)
{
	if (NewCount != 0)
	{
		return;
	}

	if (!OwningAbilitySystemComponent)
	{
		return;
	}

	FGameplayAbilitySpec* AbilitySpec = OwningAbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (!AbilitySpec)
	{
		// Ability might have been cleared when cooldown expires
		return;
	}

	UGameplayAbility* Ability = AbilitySpec->Ability;

	// Broadcast cooldown expiration to HUD
	if (IsValid(Ability))
	{
		HandleCooldownEnd(AbilitySpec->Ability, GameplayTag, Duration);
	}

	OwningAbilitySystemComponent->RegisterGameplayTagEvent(GameplayTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
}

void UGSCUWHud::InitFromCharacter()
{
	if (!OwningAbilitySystemComponent)
	{
		// Prevent calls to GetAttributeValue if no ASC yet, which may happen for clients (and prevent warning logs during initialization)
		return;
	}

	SetHealth(GetAttributeValue(UGSCAttributeSet::GetHealthAttribute()));
	SetStamina(GetAttributeValue(UGSCAttributeSet::GetStaminaAttribute()));
	SetMana(GetAttributeValue(UGSCAttributeSet::GetManaAttribute()));
}

void UGSCUWHud::SetMaxHealth(const float MaxHealth)
{
	const float Health = GetAttributeValue(UGSCAttributeSet::GetHealthAttribute());
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(GetAttributeFormatString(Health, MaxHealth)));
	}

	if (MaxHealth != 0)
	{
		SetHealthPercentage(Health / MaxHealth);
	}
}

void UGSCUWHud::SetHealth(const float Health)
{
	const float MaxHealth = GetAttributeValue(UGSCAttributeSet::GetMaxHealthAttribute());
	if (HealthText)
	{
		HealthText->SetText(FText::FromString(GetAttributeFormatString(Health, MaxHealth)));
	}

	if (MaxHealth != 0)
	{
		SetHealthPercentage(Health / MaxHealth);
	}
}

void UGSCUWHud::SetHealthPercentage(const float HealthPercentage)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(HealthPercentage);
	}
}

void UGSCUWHud::SetMaxStamina(const float MaxStamina)
{
	const float Stamina = GetAttributeValue(UGSCAttributeSet::GetStaminaAttribute());
	if (StaminaText)
	{
		StaminaText->SetText(FText::FromString(GetAttributeFormatString(Stamina, MaxStamina)));
	}

	if (MaxStamina != 0)
	{
		SetStaminaPercentage(Stamina / MaxStamina);
	}
}

void UGSCUWHud::SetStamina(const float Stamina)
{
	const float MaxStamina = GetAttributeValue(UGSCAttributeSet::GetMaxStaminaAttribute());
	if (StaminaText)
	{
		StaminaText->SetText(FText::FromString(GetAttributeFormatString(Stamina, MaxStamina)));
	}

	if (MaxStamina != 0)
	{
		SetStaminaPercentage(Stamina / MaxStamina);
	}
}

void UGSCUWHud::SetStaminaPercentage(const float StaminaPercentage)
{
	if (StaminaProgressBar)
	{
		StaminaProgressBar->SetPercent(StaminaPercentage);
	}
}

void UGSCUWHud::SetMaxMana(const float MaxMana)
{
	const float Mana = GetAttributeValue(UGSCAttributeSet::GetManaAttribute());
	if (ManaText)
	{
		ManaText->SetText(FText::FromString(GetAttributeFormatString(Mana, MaxMana)));
	}

	if (MaxMana != 0)
	{
		SetManaPercentage(Mana / MaxMana);
	}
}

void UGSCUWHud::SetMana(const float Mana)
{
	const float MaxMana = GetAttributeValue(UGSCAttributeSet::GetMaxManaAttribute());
	if (ManaText)
	{
		ManaText->SetText(FText::FromString(GetAttributeFormatString(Mana, MaxMana)));
	}

	if (MaxMana != 0)
	{
		SetManaPercentage(Mana / MaxMana);
	}
}

void UGSCUWHud::SetManaPercentage(const float ManaPercentage)
{
	if (ManaProgressBar)
	{
		ManaProgressBar->SetPercent(ManaPercentage);
	}
}

void UGSCUWHud::HandleAttributeChange(const FGameplayAttribute Attribute, const float NewValue, const float OldValue)
{
	OnAttributeChange(Attribute, NewValue, OldValue);

	if (Attribute == UGSCAttributeSet::GetHealthAttribute())
	{
		SetHealth(NewValue);
	}
	else if (Attribute == UGSCAttributeSet::GetStaminaAttribute())
	{
		SetStamina(NewValue);
	}
	else if (Attribute == UGSCAttributeSet::GetManaAttribute())
	{
		SetMana(NewValue);
	}
	else if (Attribute == UGSCAttributeSet::GetMaxHealthAttribute())
	{
		SetMaxHealth(NewValue);
	}
	else if (Attribute == UGSCAttributeSet::GetMaxStaminaAttribute())
	{
		SetMaxStamina(NewValue);
	}
	else if (Attribute == UGSCAttributeSet::GetMaxManaAttribute())
	{
		SetMaxMana(NewValue);
	}
}

void UGSCUWHud::HandleGameplayEffectStackChange(const FGameplayTagContainer AssetTags, const FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle, const int32 NewStackCount, const int32 OldStackCount)
{
	OnGameplayEffectStackChange(AssetTags, GrantedTags, ActiveHandle, NewStackCount, OldStackCount);
}

void UGSCUWHud::HandleGameplayEffectTimeChange(const FGameplayTagContainer AssetTags, const FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle, const float NewStartTime, const float NewDuration)
{
	OnGameplayEffectTimeChange(AssetTags, GrantedTags, ActiveHandle, NewStartTime, NewDuration);
}

void UGSCUWHud::HandleGameplayEffectAdded(const FGameplayTagContainer AssetTags, const FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle)
{
	OnGameplayEffectAdded(AssetTags, GrantedTags, ActiveHandle, GetGameplayEffectUIData(ActiveHandle));
}

void UGSCUWHud::HandleGameplayEffectRemoved(const FGameplayTagContainer AssetTags, const FGameplayTagContainer GrantedTags, const FActiveGameplayEffectHandle ActiveHandle)
{
	OnGameplayEffectRemoved(AssetTags, GrantedTags, ActiveHandle, GetGameplayEffectUIData(ActiveHandle));
}

void UGSCUWHud::HandleGameplayTagChange(const FGameplayTag GameplayTag, const int32 NewTagCount)
{
	OnGameplayTagChange(GameplayTag, NewTagCount);
}

void UGSCUWHud::HandleCooldownStart(UGameplayAbility* Ability, const FGameplayTagContainer CooldownTags, const float TimeRemaining, const float Duration)
{
	OnCooldownStart(Ability, CooldownTags, TimeRemaining, Duration);
}

void UGSCUWHud::HandleCooldownEnd(UGameplayAbility* Ability, const FGameplayTag CooldownTag, const float Duration)
{
	OnCooldownEnd(Ability, CooldownTag, Duration);
}

float UGSCUWHud::GetAttributeValue(const FGameplayAttribute Attribute) const
{
	if (!OwningAbilitySystemComponent)
	{
		GSC_LOG(Warning, TEXT("UGSCUWHud::GetAttributeValue() The owner AbilitySystemComponent seems to be invalid. GetAttributeValue() will return 0.f."))
		return 0.0f;
	}

	if (!OwningAbilitySystemComponent->HasAttributeSetForAttribute(Attribute))
	{
		GSC_LOG(Warning, TEXT("UGSCUWHud::GetAttributeValue() Attribute %s doesn't seem to be part of the AttributeSet, returning 0.f"))
		return 0.0f;
	}

	return OwningAbilitySystemComponent->GetNumericAttributeBase(Attribute);
}

FString UGSCUWHud::GetAttributeFormatString(const float BaseValue, const float MaxValue)
{
	return FString::Printf(TEXT("%d / %d"), FMath::FloorToInt(BaseValue), FMath::FloorToInt(MaxValue));
}

FGSCGameplayEffectUIData UGSCUWHud::GetGameplayEffectUIData(const FActiveGameplayEffectHandle ActiveHandle)
{
	return FGSCGameplayEffectUIData(
		UAbilitySystemBlueprintLibrary::GetActiveGameplayEffectStartTime(ActiveHandle),
		UAbilitySystemBlueprintLibrary::GetActiveGameplayEffectTotalDuration(ActiveHandle),
		UAbilitySystemBlueprintLibrary::GetActiveGameplayEffectExpectedEndTime(ActiveHandle),
		UAbilitySystemBlueprintLibrary::GetActiveGameplayEffectStackCount(ActiveHandle),
		UAbilitySystemBlueprintLibrary::GetActiveGameplayEffectStackLimitCount(ActiveHandle)
	);
}
