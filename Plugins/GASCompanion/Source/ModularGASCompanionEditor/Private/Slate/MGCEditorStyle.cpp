// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Slate/MGCEditorStyle.h"
#include "Slate/MGCSegmentedControlWidgetStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "EditorStyle.h"

#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( Style, RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )
#define JPG_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush(Style->RootToContentDir( RelativePath, TEXT(".jpg")), __VA_ARGS__ )
#define COLOR( HexValue ) FLinearColor::FromSRGBColor(FColor::FromHex(HexValue))
#define BOX2_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )


TSharedPtr<FSlateStyleSet> FMGCEditorStyle::StyleInstance = NULL;

void FMGCEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FMGCEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FMGCEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ModularGASCompanionEditorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon180x100(180.0f, 100.0f);
const FVector2D Icon256x256(256.0f, 256.0f);

TSharedRef<FSlateStyleSet> FMGCEditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = CreateStyle("ModularGASCompanionEditorStyle");
	Style->Set("Icons.Lock", new IMAGE_BRUSH("icons/lock", Icon16x16));

	const FTextBlockStyle NormalText = FTextBlockStyle()
	   .SetFont(DEFAULT_FONT("Regular", FCoreStyle::RegularTextSize))
	   .SetColorAndOpacity(FSlateColor::UseForeground())
	   .SetShadowOffset(FVector2D::ZeroVector)
	   .SetShadowColorAndOpacity(FLinearColor::Black)
	   .SetHighlightColor( FLinearColor( 0.02f, 0.3f, 0.0f ) )
	   .SetHighlightShape(BOX_BRUSH(Style, "Common/TextBlockHighlightShape", FMargin(3.f/8.f)));

	Style->Set("ButtonText", FTextBlockStyle(NormalText).SetFont(DEFAULT_FONT("Bold", 10)));

	const FMargin DefaultMargins(8.f, 4.f);

	const FSlateColor Foreground = COLOR("#C0C0C0FF");
	const FSlateColor ForegroundHover = COLOR("#FFFFFFFF");
	const FSlateColor Primary = COLOR("#26BBFFFF");
	const FSlateColor Secondary = COLOR("#383838FF");
	const FSlateColor Hover = COLOR("#575757FF");
	const FSlateColor Input = COLOR("#0F0F0FFF");

	Style->Set("Color.Foreground", Foreground);
	Style->Set("Color.ForegroundHover", ForegroundHover);
	Style->Set("Color.Primary", Primary);
	Style->Set("Color.Secondary", Secondary);
	Style->Set("Color.Hover", Hover);
	Style->Set("Color.Input", Input);

	/* Style for a segmented box */
	const FCheckBoxStyle SegmentedBoxLeft = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/left", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Secondary))
		.SetUncheckedHoveredImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/left", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Hover))
		.SetUncheckedPressedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/left", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Secondary))
		.SetCheckedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/left", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Input))
		.SetCheckedHoveredImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/left", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Input))
		.SetCheckedPressedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/left", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Input))
		.SetForegroundColor(Foreground)
		.SetPadding(DefaultMargins);

	const FCheckBoxStyle SegmentedBoxCenter = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(ToFSlateBrush(Secondary))
		.SetUncheckedHoveredImage(ToFSlateBrush(Hover))
		.SetUncheckedPressedImage(ToFSlateBrush(Secondary))
		.SetCheckedImage(ToFSlateBrush(Input))
		.SetCheckedHoveredImage(ToFSlateBrush(Input))
		.SetCheckedPressedImage(ToFSlateBrush(Input))
		.SetForegroundColor(Foreground)
		.SetPadding(DefaultMargins);


	const FCheckBoxStyle SegmentedBoxRight = FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/right", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Secondary))
		.SetUncheckedHoveredImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/right", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Hover))
		.SetUncheckedPressedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/right", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Secondary))
		.SetCheckedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/right", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Input))
		.SetCheckedHoveredImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/right", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Input))
		.SetCheckedPressedImage(BOX_BRUSH(Style, "CoreWidgets/SegmentedBox/right", FVector2D(16.f, 16.f), FMargin(4.0f / 16.0f), Input))
		.SetForegroundColor(Foreground)
		.SetPadding(DefaultMargins);

	const FButtonStyle ButtonStyle = FButtonStyle()
		.SetNormalPadding(DefaultMargins)
		.SetPressedPadding(DefaultMargins)
		.SetNormal(ToFSlateBrush(Secondary))
		.SetPressed(ToFSlateBrush(Input))
		.SetHovered(ToFSlateBrush(Hover));

	Style->Set("EditButtonStyle", ButtonStyle);

	Style->Set("SegmentedControl", FMGCSegmentedControlStyle()
		.SetControlStyle(SegmentedBoxCenter)
		.SetFirstControlStyle(SegmentedBoxLeft)
		.SetLastControlStyle(SegmentedBoxRight)
	);

	const float PaddingAmount = 2.0f;
	Style->Set("PluginTile.Padding", PaddingAmount);

	const float ThumbnailImageSize = 128.0f;
	Style->Set("PluginTile.ThumbnailImageSize", ThumbnailImageSize);

	FTextBlockStyle NameText = FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(0.9f, 0.9f, 0.9f));
	NameText.Font.Size = 14;
	Style->Set("PluginTile.NameText", NameText);

	FTextBlockStyle DescriptionText = FTextBlockStyle(NormalText).SetColorAndOpacity(FLinearColor(0.8f, 0.8f, 0.8f));
	DescriptionText.Font.Size = 10;
	Style->Set("PluginTile.DescriptionText", DescriptionText);

	// Plugin Creator
	{
		Style->Set("PluginPath.BrowseButton", FButtonStyle(ButtonStyle)
			// .SetNormal(FSlateRoundedBoxBrush(FStyleColors::Secondary, 4.0f, FStyleColors::Secondary, 2.0f))
			// .SetHovered(FSlateRoundedBoxBrush(FStyleColors::Hover, 4.0f, FStyleColors::Hover, 2.0f))
			// .SetPressed(FSlateRoundedBoxBrush(FStyleColors::Header, 4.0f, FStyleColors::Header, 2.0f))
			.SetNormalPadding(FMargin(2, 2, 2, 2))
			.SetPressedPadding(FMargin(2, 3, 2, 1))
		);
	}

	return Style;
}

TSharedRef<FSlateStyleSet> FMGCEditorStyle::CreateStyle(const FName StyleSetName)
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(StyleSetName));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("GASCompanion")->GetBaseDir() / TEXT("Resources"));
	return Style;
}

void FMGCEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FMGCEditorStyle::Get()
{
	return *StyleInstance;
}

FSlateBrush FMGCEditorStyle::ToFSlateBrush(const FSlateColor& InColor)
{
	FSlateBrush Brush;
	Brush.ImageSize = FVector2D::ZeroVector;
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.Margin = FMargin(0.0f);
	Brush.Tiling = ESlateBrushTileType::NoTile;
	Brush.ImageType = ESlateBrushImageType::NoImage;
	Brush.TintColor = InColor;
	return Brush;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
#undef JPG_IMAGE_BRUSH
#undef COLOR
#undef BOX2_BRUSH
