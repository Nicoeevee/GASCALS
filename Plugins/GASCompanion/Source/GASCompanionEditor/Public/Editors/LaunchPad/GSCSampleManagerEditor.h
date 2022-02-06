// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/Editor/GSCExampleMapManager.h"
#include "GameFramework/PlayerInput.h"
#include "GSCSampleManagerEditor.generated.h"

class AGSCCharacterBase;
class UGSCGameplayAbility;
class UAttributeSet;

UCLASS()
class GASCOMPANIONEDITOR_API AGSCSampleManagerEditor : public AGSCExampleMapManager
{
	GENERATED_BODY()

public:
	AGSCSampleManagerEditor();

	bool HasMissingActionMappings();
	void AddMissingActionMapping();

	bool HasMissingGameplayTags();
	bool AddMissingGameplayTags();

	bool HasMissingAttributeSets();
	bool AddMissingAttributeSets();
	TArray<TSubclassOf<UAttributeSet>> GetMissingAttributeSets();

protected:

	TArray<FInputActionKeyMapping> GetMissingActionMappings();
	TArray<FInputAxisKeyMapping> GetMissingAxisMappings();
	TArray<FString> GetMissingGameplayTags();

	static UTexture2D* GetBillboardSprite();
};
