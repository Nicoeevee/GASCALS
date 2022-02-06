// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Editors/LaunchPad/GSCSampleManagerDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "IDocumentation.h"
#include "PropertyCustomizationHelpers.h"
#include "Core/Editor/GSCExampleMapManager.h"
#include "Widgets/Input/SEditableTextBox.h"

#define LOCTEXT_NAMESPACE "FGSCMissingInputMappingManagerCustomization"

void FGSCSampleManagerDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	static const FName BindingsCategory = TEXT("Required Bindings");
	static const FName ActionMappings = GET_MEMBER_NAME_CHECKED(AGSCExampleMapManager, ActionMappings);
	static const FName AxisMappings = GET_MEMBER_NAME_CHECKED(AGSCExampleMapManager, AxisMappings);

	IDetailCategoryBuilder& MappingsDetailCategoryBuilder = DetailBuilder.EditCategory(BindingsCategory);

	MappingsDetailCategoryBuilder
	.AddCustomRow(LOCTEXT("MappingsTitle", "Action Axis Mappings"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.AutoWrapText(true)
			.Text(LOCTEXT("MappingsDescription", "Provide action and axis mappings here that are required by the example map."))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			IDocumentation::Get()->CreateAnchor(FString("Gameplay/Input"))
		]
	];

	// Custom Action Mappings
	const TSharedPtr<IPropertyHandle> ActionMappingsPropertyHandle = DetailBuilder.GetProperty(ActionMappings, AGSCExampleMapManager::StaticClass());
	ActionMappingsPropertyHandle->MarkHiddenByCustomization();

	const TSharedRef<FGSCMissingInputMappingsNodeBuilder> ActionMappingsBuilder = MakeShareable(new FGSCMissingInputMappingsNodeBuilder(&DetailBuilder, ActionMappingsPropertyHandle, EGSCNodeBuilderType::ActionMappings));
	MappingsDetailCategoryBuilder.AddCustomBuilder(ActionMappingsBuilder);

	// Custom Axis Mappings
	const TSharedPtr<IPropertyHandle> AxisMappingsPropertyHandle = DetailBuilder.GetProperty(AxisMappings, AGSCExampleMapManager::StaticClass());
	AxisMappingsPropertyHandle->MarkHiddenByCustomization();

	const TSharedRef<FGSCMissingInputMappingsNodeBuilder> AxisMappingsBuilder = MakeShareable(new FGSCMissingInputMappingsNodeBuilder(&DetailBuilder, AxisMappingsPropertyHandle, EGSCNodeBuilderType::AxisMappings));
	MappingsDetailCategoryBuilder.AddCustomBuilder(AxisMappingsBuilder);

	DetailBuilder.HideCategory("Transform");
	DetailBuilder.HideCategory("Rendering");
	DetailBuilder.HideCategory("Input");
	DetailBuilder.HideCategory("Actor");
	DetailBuilder.HideCategory("LOD");
	DetailBuilder.HideCategory("Cooking");
	DetailBuilder.HideCategory("Collision");
	DetailBuilder.HideCategory("Replication");
	DetailBuilder.HideCategory("Bindings");
}

TSharedRef<IDetailCustomization> FGSCSampleManagerDetails::MakeInstance()
{
	return MakeShareable(new FGSCSampleManagerDetails);
}

// FDACustomInputActionMappingsNodeBuilder
FGSCMissingInputMappingsNodeBuilder::FGSCMissingInputMappingsNodeBuilder(IDetailLayoutBuilder* InDetailLayoutBuilder, const TSharedPtr<IPropertyHandle>& InPropertyHandle, const EGSCNodeBuilderType InBuilderType)
	: DetailLayoutBuilder(InDetailLayoutBuilder),
	BuilderType(InBuilderType),
	ActiveMappingPropertyHandle(InPropertyHandle)
{
	// Delegate for when the children in the array change
	FSimpleDelegate RebuildChildrenDelegate = FSimpleDelegate::CreateRaw(this, &FGSCMissingInputMappingsNodeBuilder::RebuildChildren);
	ActiveMappingPropertyHandle->SetOnPropertyValueChanged(RebuildChildrenDelegate);
	ActiveMappingPropertyHandle->AsArray()->SetOnNumElementsChanged(RebuildChildrenDelegate);
}

void FGSCMissingInputMappingsNodeBuilder::Tick(float DeltaTime)
{
	if (GroupsRequireRebuild())
	{
		RebuildChildren();
	}
	HandleDelayedGroupExpansion();
}

void FGSCMissingInputMappingsNodeBuilder::OnActionMappingNameCommitted(const FText& InName, ETextCommit::Type CommitInfo, const FGSCInputMappingSet MappingSet)
{
	const FText TransactionLabel = IsForActionMappings() ?
		LOCTEXT("RenameActionMapping_Transaction", "Rename Action Mapping") :
		LOCTEXT("RenameAxisMapping_Transaction", "Rename Axis Mapping");

	const FScopedTransaction Transaction(TransactionLabel);

	FName NewName = FName(*InName.ToString());
	FName CurrentName = NewName;
	if (MappingSet.Mappings.Num() > 0)
	{
		MappingSet.Mappings[0]->GetChildHandle(GetChildHandleName())->GetValue(CurrentName);
	}

	if (NewName != CurrentName)
	{
		for (int32 Index = 0; Index < MappingSet.Mappings.Num(); ++Index)
		{
			MappingSet.Mappings[Index]->GetChildHandle(GetChildHandleName())->SetValue(NewName);
		}

		if (MappingSet.DetailGroup)
		{
			DelayedGroupExpansionStates.Emplace(NewName, MappingSet.DetailGroup->GetExpansionState());

			// Don't want to save expansion state of old name
			MappingSet.DetailGroup->ToggleExpansion(false);
		}
	}
}

void FGSCMissingInputMappingsNodeBuilder::AddActionMappingToGroupButton_OnClick(const FGSCInputMappingSet MappingSet)
{
	const FText TransactionLabel = IsForActionMappings() ?
		LOCTEXT("AddActionMappingToGroup_Transaction", "Add Action Mapping To Group") :
		LOCTEXT("AddAxisMappingToGroup_Transaction", "Add Axis Mapping To Group");

	const FScopedTransaction Transaction(TransactionLabel);

	TArray<UObject*> OuterObjects;
	ActiveMappingPropertyHandle->GetOuterObjects(OuterObjects);


	if (OuterObjects.Num() == 1)
	{
		AGSCExampleMapManager* InputSettings = CastChecked<AGSCExampleMapManager>(OuterObjects[0]);
		InputSettings->Modify();
		ActiveMappingPropertyHandle->NotifyPreChange();

		DelayedGroupExpansionStates.Emplace(MappingSet.SharedName, true);
		if (IsForActionMappings())
		{
			const FInputActionKeyMapping NewMapping(MappingSet.SharedName);
			InputSettings->ActionMappings.Add(NewMapping);
		} else
		{
			const FInputAxisKeyMapping NewMapping(MappingSet.SharedName);
			InputSettings->AxisMappings.Add(NewMapping);
		}


		#if ENGINE_MAJOR_VERSION == 5
		ActiveMappingPropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
		#else
		ActiveMappingPropertyHandle->NotifyPostChange();
		#endif
	}
}

void FGSCMissingInputMappingsNodeBuilder::RemoveActionMappingGroupButton_OnClick(const FGSCInputMappingSet MappingSet)
{
	const FText TransactionLabel = IsForActionMappings() ?
		LOCTEXT("RemoveActionMappingGroup_Transaction", "Remove Action Mapping Group") :
		LOCTEXT("RemoveAxisMappingGroup_Transaction", "Remove Axis Mapping Group");

	const FScopedTransaction Transaction(TransactionLabel);

	TSharedPtr<IPropertyHandleArray> ActiveMappingsArrayHandle = ActiveMappingPropertyHandle->AsArray();

	TArray<int32> SortedIndices;
	for (int32 Index = 0; Index < MappingSet.Mappings.Num(); ++Index)
	{
		SortedIndices.AddUnique(MappingSet.Mappings[Index]->GetIndexInArray());
	}
	SortedIndices.Sort();

	for (int32 Index = SortedIndices.Num() - 1; Index >= 0; --Index)
	{
		ActiveMappingsArrayHandle->DeleteItem(SortedIndices[Index]);
	}
}
void FGSCMissingInputMappingsNodeBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	const FText TooltipTextAdd = IsForActionMappings() ?
		LOCTEXT("AddActionMappingToolTip", "Adds Action Mapping") :
		LOCTEXT("AddAxisMappingToolTip", "Adds Axis Mapping");

	const FText TooltipTextClear = IsForActionMappings() ?
		LOCTEXT("RemoveActionMappingToolTip", "Removes all Action Mapping") :
		LOCTEXT("RemoveAxisMappingToolTip", "Removes all Axis Mapping");

	const TSharedRef<SWidget> AddButton = PropertyCustomizationHelpers::MakeAddButton(
		FSimpleDelegate::CreateSP(this, &FGSCMissingInputMappingsNodeBuilder::AddActionMappingButton_OnClick),
		TooltipTextAdd
	);

	const TSharedRef<SWidget> ClearButton = PropertyCustomizationHelpers::MakeEmptyButton(
		FSimpleDelegate::CreateSP(this, &FGSCMissingInputMappingsNodeBuilder::ClearActionMappingButton_OnClick),
		TooltipTextClear
	);

	NodeRow
	[
		SNew( SHorizontalBox )
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			ActiveMappingPropertyHandle->CreatePropertyNameWidget()
		]
		+SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			AddButton
		]
		+SHorizontalBox::Slot()
		.Padding(2.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			ClearButton
		]
	];
}

void FGSCMissingInputMappingsNodeBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	RebuildGroupedMappings();

	for (int32 Index = 0; Index < GroupedMappings.Num(); ++Index)
	{
		FGSCInputMappingSet& MappingSet = GroupedMappings[Index];

		FString GroupNameString = IsForActionMappings() ?
			TEXT("ActionMappings.") :
			TEXT("AxisMappings.");

		MappingSet.SharedName.AppendString(GroupNameString);
		const FName GroupName(*GroupNameString);
		IDetailGroup& MappingGroup = ChildrenBuilder.AddGroup(GroupName, FText::FromName(MappingSet.SharedName));
		MappingSet.DetailGroup = &MappingGroup;

		const FText TooltipTextAdd = IsForActionMappings() ?
			LOCTEXT("AddActionMappingToGroupToolTip", "Adds Action Mapping to Group") :
			LOCTEXT("AddAxisMappingToGroupToolTip", "Adds Axis Mapping to Group");

		const FText TooltipTextRemove = IsForActionMappings() ?
			LOCTEXT("RemoveActionMappingToGroupToolTip", "Removes Action Mapping Group") :
			LOCTEXT("RemoveAxisMappingToGroupToolTip", "Removes Axis Mapping Group");;

		const TSharedRef<SWidget> AddButton = PropertyCustomizationHelpers::MakeAddButton(
			FSimpleDelegate::CreateSP(this, &FGSCMissingInputMappingsNodeBuilder::AddActionMappingToGroupButton_OnClick, MappingSet),
			TooltipTextAdd
		);

		const TSharedRef<SWidget> RemoveButton = PropertyCustomizationHelpers::MakeDeleteButton(
			FSimpleDelegate::CreateSP(this, &FGSCMissingInputMappingsNodeBuilder::RemoveActionMappingGroupButton_OnClick, MappingSet),
			TooltipTextRemove
		);

		MappingGroup.HeaderRow()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(200.f)
				[
					SNew(SEditableTextBox)
					.Padding(2.0f)
					.Text(FText::FromName(MappingSet.SharedName))
					.OnTextCommitted(FOnTextCommitted::CreateSP(this, &FGSCMissingInputMappingsNodeBuilder::OnActionMappingNameCommitted, MappingSet))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+SHorizontalBox::Slot()
			  .Padding(2.f, 0.f)
			  .HAlign(HAlign_Center)
			  .VAlign(VAlign_Center)
			  .AutoWidth()
			[
				AddButton
			]
			+SHorizontalBox::Slot()
			  .Padding(2.f, 0.f)
			  .HAlign(HAlign_Center)
			  .VAlign(VAlign_Center)
			  .AutoWidth()
			[
				RemoveButton
			]
		];

		for (int32 MappingIndex = 0; MappingIndex < MappingSet.Mappings.Num(); ++MappingIndex)
		{
			MappingGroup.AddPropertyRow(MappingSet.Mappings[MappingIndex]).ShowPropertyButtons(false);
		}
	}
}

void FGSCMissingInputMappingsNodeBuilder::AddActionMappingButton_OnClick()
{
	const FName InputMappingName = IsForActionMappings() ?
		FName(*LOCTEXT("NewActionMappingName", "NewActionMapping").ToString()) :
		FName(*LOCTEXT("NewAxisMappingName", "NewAxisMapping").ToString());

	static int32 NewMappingCount = 0;

	const FText TransactionLabel = IsForActionMappings() ?
		LOCTEXT("AddActionMapping_Transaction", "Add Action Mapping") :
		LOCTEXT("AddAxisMapping_Transaction", "Add Axis Mapping");

	const FScopedTransaction Transaction(TransactionLabel);

	TArray<UObject*> OuterObjects;
	ActiveMappingPropertyHandle->GetOuterObjects(OuterObjects);

	if (OuterObjects.Num() == 1)
	{
		AGSCExampleMapManager* MapManager = CastChecked<AGSCExampleMapManager>(OuterObjects[0]);
		MapManager->Modify();
		ActiveMappingPropertyHandle->NotifyPreChange();

		FName NewActionMappingName;
		bool bFoundUniqueName;
		do
		{
			// Create a numbered name and check whether it's already been used
			NewActionMappingName = FName(InputMappingName, ++NewMappingCount);
			bFoundUniqueName = true;
			if (IsForActionMappings())
			{
				for (int32 Index = 0; Index < MapManager->ActionMappings.Num(); ++Index)
				{
					if (MapManager->ActionMappings[Index].ActionName == NewActionMappingName)
					{
						bFoundUniqueName = false;
						break;
					}
				}
			}
			else
			{
				for (int32 Index = 0; Index < MapManager->AxisMappings.Num(); ++Index)
				{
					if (MapManager->AxisMappings[Index].AxisName == NewActionMappingName)
					{
						bFoundUniqueName = false;
						break;
					}
				}
			}
		}
		while (!bFoundUniqueName);

		DelayedGroupExpansionStates.Emplace(NewActionMappingName, true);
		if (IsForActionMappings())
		{
			const FInputActionKeyMapping NewMapping(NewActionMappingName);
			MapManager->ActionMappings.Add(NewMapping);
		}
		else
		{
			const FInputAxisKeyMapping NewMapping(NewActionMappingName);
			MapManager->AxisMappings.Add(NewMapping);
		}

		#if ENGINE_MAJOR_VERSION == 5
				ActiveMappingPropertyHandle->NotifyPostChange(EPropertyChangeType::ArrayAdd);
		#else
				ActiveMappingPropertyHandle->NotifyPostChange();
		#endif
	}
}

void FGSCMissingInputMappingsNodeBuilder::ClearActionMappingButton_OnClick()
{
	ActiveMappingPropertyHandle->AsArray()->EmptyArray();
}

bool FGSCMissingInputMappingsNodeBuilder::GroupsRequireRebuild() const
{
	for (int32 GroupIndex = 0; GroupIndex < GroupedMappings.Num(); ++GroupIndex)
	{
		const FGSCInputMappingSet& MappingSet = GroupedMappings[GroupIndex];
		for (int32 MappingIndex = 0; MappingIndex < MappingSet.Mappings.Num(); ++MappingIndex)
		{
			FName ActionName;
			MappingSet.Mappings[MappingIndex]->GetChildHandle(GetChildHandleName())->GetValue(ActionName);
			if (MappingSet.SharedName != ActionName)
			{
				return true;
			}
		}
	}
	return false;
}

void FGSCMissingInputMappingsNodeBuilder::RebuildGroupedMappings()
{
	GroupedMappings.Empty();

	const TSharedPtr<IPropertyHandleArray> ActiveMappingsArrayHandle = ActiveMappingPropertyHandle->AsArray();

	uint32 NumMappings;
	ActiveMappingsArrayHandle->GetNumElements(NumMappings);
	for (uint32 Index = 0; Index < NumMappings; ++Index)
	{
		TSharedRef<IPropertyHandle> ActionMapping = ActiveMappingsArrayHandle->GetElement(Index);
		FName ActionName;
		const FPropertyAccess::Result Result = ActionMapping->GetChildHandle(GetChildHandleName())->GetValue(ActionName);

		if (Result == FPropertyAccess::Success)
		{
			int32 FoundIndex = INDEX_NONE;
			for (int32 GroupIndex = 0; GroupIndex < GroupedMappings.Num(); ++GroupIndex)
			{
				if (GroupedMappings[GroupIndex].SharedName == ActionName)
				{
					FoundIndex = GroupIndex;
					break;
				}
			}
			if (FoundIndex == INDEX_NONE)
			{
				FoundIndex = GroupedMappings.Num();
				GroupedMappings.AddZeroed();
				GroupedMappings[FoundIndex].SharedName = ActionName;
			}
			GroupedMappings[FoundIndex].Mappings.Add(ActionMapping);
		}
	}
}

void FGSCMissingInputMappingsNodeBuilder::HandleDelayedGroupExpansion()
{
	if (DelayedGroupExpansionStates.Num() > 0)
	{
		for (const auto GroupState : DelayedGroupExpansionStates)
		{
			for (auto& MappingSet : GroupedMappings)
			{
				if (MappingSet.SharedName == GroupState.Key)
				{
					MappingSet.DetailGroup->ToggleExpansion(GroupState.Value);
					break;
				}
			}
		}
		DelayedGroupExpansionStates.Empty();
	}
}

bool FGSCMissingInputMappingsNodeBuilder::IsForActionMappings() const
{
	return BuilderType == EGSCNodeBuilderType::ActionMappings;
}

FName FGSCMissingInputMappingsNodeBuilder::GetChildHandleName() const
{
	return IsForActionMappings() ?
		GET_MEMBER_NAME_CHECKED(FInputActionKeyMapping, ActionName) :
		GET_MEMBER_NAME_CHECKED(FInputAxisKeyMapping, AxisName);
}

#undef LOCTEXT_NAMESPACE
