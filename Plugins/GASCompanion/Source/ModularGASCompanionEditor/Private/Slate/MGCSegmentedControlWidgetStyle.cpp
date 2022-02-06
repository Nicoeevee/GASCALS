// Copyright 2021 Mickael Daniel. All Rights Reserved.


#include "Slate/MGCSegmentedControlWidgetStyle.h"

FMGCSegmentedControlStyle::FMGCSegmentedControlStyle()
{
}

const FName FMGCSegmentedControlStyle::TypeName(TEXT("FMGCSegmentedControlStyle"));

const FMGCSegmentedControlStyle& FMGCSegmentedControlStyle::GetDefault()
{
	static FMGCSegmentedControlStyle Default;
	return Default;
}

void FMGCSegmentedControlStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
	ControlStyle.GetResources(OutBrushes);
	FirstControlStyle.GetResources(OutBrushes);
	LastControlStyle.GetResources(OutBrushes);
}

