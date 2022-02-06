// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "IDetailCustomNodeBuilder.h"

class IDetailGroup;

struct FGSCInputMappingSet
{
	FName SharedName;
	IDetailGroup* DetailGroup;
	TArray<TSharedRef<IPropertyHandle>> Mappings;
};

enum EGSCNodeBuilderType
{
	ActionMappings,
	AxisMappings
};

class GASCOMPANIONEDITOR_API FGSCSampleManagerDetails : public IDetailCustomization
{
public:

	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//

	static TSharedRef<IDetailCustomization> MakeInstance();
};

class FGSCMissingInputMappingsNodeBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FGSCMissingInputMappingsNodeBuilder>
{
public:
	FGSCMissingInputMappingsNodeBuilder(IDetailLayoutBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle, EGSCNodeBuilderType InBuilderType);

	/** IDetailCustomNodeBuilder interface */
	virtual void SetOnRebuildChildren(const FSimpleDelegate InOnRebuildChildren) override
	{
		OnRebuildChildren = InOnRebuildChildren;
	}

	virtual bool RequiresTick() const override { return true; }
	virtual void Tick(float DeltaTime) override;
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override { return true; };
	virtual FName GetName() const override { return FName(TEXT("ActionMappings")); }

private:
	void AddActionMappingButton_OnClick();
	void ClearActionMappingButton_OnClick();
	void OnActionMappingNameCommitted(const FText& InName, ETextCommit::Type CommitInfo, const FGSCInputMappingSet MappingSet);
	void AddActionMappingToGroupButton_OnClick(const FGSCInputMappingSet MappingSet);
	void RemoveActionMappingGroupButton_OnClick(const FGSCInputMappingSet MappingSet);

	bool GroupsRequireRebuild() const;
	void RebuildGroupedMappings();
	void RebuildChildren()
	{
		OnRebuildChildren.ExecuteIfBound();
	}
	/** Makes sure that groups have their expansion set after any rebuilding */
	void HandleDelayedGroupExpansion();

	/** Little helper to determine if we're dealing with ActionMappings (true) or AxisMappings (false) */
	bool IsForActionMappings() const;

	FName GetChildHandleName() const;

	/** Called to rebuild the children of the detail tree */
	FSimpleDelegate OnRebuildChildren;

	/** Associated detail layout builder */
	IDetailLayoutBuilder* DetailLayoutBuilder;

	/** Type of builder (action or axis) */
	EGSCNodeBuilderType BuilderType;

	/** ActiveProperty handle */
	TSharedPtr<IPropertyHandle> ActiveMappingPropertyHandle;

	TArray<FGSCInputMappingSet> GroupedMappings;

	TArray<TPair<FName, bool>> DelayedGroupExpansionStates;
};

class FGSCMissingAxisMappingsNodeBuilder : public IDetailCustomNodeBuilder, public TSharedFromThis<FGSCMissingAxisMappingsNodeBuilder>
{
public:
	FGSCMissingAxisMappingsNodeBuilder( IDetailLayoutBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle );

	/** IDetailCustomNodeBuilder interface */
	virtual void SetOnRebuildChildren( FSimpleDelegate InOnRebuildChildren  ) override { OnRebuildChildren = InOnRebuildChildren; }
	virtual bool RequiresTick() const override { return true; }
	virtual void Tick( float DeltaTime ) override;
	virtual void GenerateHeaderRowContent( FDetailWidgetRow& NodeRow ) override;
	virtual void GenerateChildContent( IDetailChildrenBuilder& ChildrenBuilder ) override;
	virtual bool InitiallyCollapsed() const override { return true; };
	virtual FName GetName() const override { return FName(TEXT("AxisMappings")); }

private:
	void AddAxisMappingButton_OnClick();
	void ClearAxisMappingButton_OnClick();
	void OnAxisMappingNameCommitted(const FText& InName, ETextCommit::Type CommitInfo, const FGSCInputMappingSet MappingSet);
	void AddAxisMappingToGroupButton_OnClick(const FGSCInputMappingSet MappingSet);
	void RemoveAxisMappingGroupButton_OnClick(const FGSCInputMappingSet MappingSet);

	bool GroupsRequireRebuild() const;
	void RebuildGroupedMappings();
	void RebuildChildren()
	{
		OnRebuildChildren.ExecuteIfBound();
	}
	/** Makes sure that groups have their expansion set after any rebuilding */
	void HandleDelayedGroupExpansion();

private:
	/** Called to rebuild the children of the detail tree */
	FSimpleDelegate OnRebuildChildren;

	/** Associated detail layout builder */
	IDetailLayoutBuilder* DetailLayoutBuilder;

	/** Property handle to associated axis mappings */
	TSharedPtr<IPropertyHandle> AxisMappingsPropertyHandle;

	TArray<FGSCInputMappingSet> GroupedMappings;

	TArray<TPair<FName, bool>> DelayedGroupExpansionStates;
};
