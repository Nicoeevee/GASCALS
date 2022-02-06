// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "MGCSegmentedControlWidgetStyle.generated.h"

/**
 * Represents the appearance of an SMGCSegmentedControl
 */
USTRUCT(BlueprintType)
struct MODULARGASCOMPANIONEDITOR_API FMGCSegmentedControlStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FMGCSegmentedControlStyle();

	/**
	 * The style to use for our Center Control
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FCheckBoxStyle ControlStyle;
	FMGCSegmentedControlStyle& SetControlStyle( const FCheckBoxStyle& InStyle ){ ControlStyle = InStyle; return *this; }

	/**
	 * The style to use for our Left Control
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FCheckBoxStyle FirstControlStyle;
	FMGCSegmentedControlStyle& SetFirstControlStyle( const FCheckBoxStyle& InStyle ){ FirstControlStyle = InStyle; return *this; }

	/**
	 * The style to use for our Left Control
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Appearance)
	FCheckBoxStyle LastControlStyle;
	FMGCSegmentedControlStyle& SetLastControlStyle( const FCheckBoxStyle& InStyle ){ LastControlStyle = InStyle; return *this; }


	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FMGCSegmentedControlStyle& GetDefault();

	/**
	 * Unlinks all colors in this style.
	 * @see FSlateColor::Unlink
	 */
	void UnlinkColors()
	{
		ControlStyle.UnlinkColors();
		FirstControlStyle.UnlinkColors();
		LastControlStyle.UnlinkColors();
	}

};

/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UMGCSegmentedControlWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FMGCSegmentedControlStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &WidgetStyle );
	}
};
