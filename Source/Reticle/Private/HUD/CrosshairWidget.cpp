#include "HUD/CrosshairWidget.h"

#include "Sensitivity/AimUserSettingsAsset.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"

void UCrosshairWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BindToProfile();
}

void UCrosshairWidget::NativeDestruct()
{
    UnbindFromProfile();
    Super::NativeDestruct();
}

void UCrosshairWidget::ApplyProfile(UAimUserSettingsAsset* InProfile)
{
    UnbindFromProfile();
    Profile = InProfile;
    BindToProfile();
    Invalidate(EInvalidateWidgetReason::Paint);
}

void UCrosshairWidget::BindToProfile()
{
    if (Profile)
    {
        ProfileChangedHandle = Profile->OnCrosshairChanged.AddUObject(
            this, &UCrosshairWidget::HandleProfileChanged);
        BoundProfile = Profile;
    }
}

void UCrosshairWidget::UnbindFromProfile()
{
    if (BoundProfile.IsValid() && ProfileChangedHandle.IsValid())
    {
        BoundProfile->OnCrosshairChanged.Remove(ProfileChangedHandle);
    }
    ProfileChangedHandle.Reset();
    BoundProfile.Reset();
}

void UCrosshairWidget::HandleProfileChanged()
{
    Invalidate(EInvalidateWidgetReason::Paint);
}

namespace
{
    // Snap so horizontal and vertical arms rasterize on the same pixel grid.
    static FORCEINLINE FVector2D SnapToPixel(const FVector2D& V)
    {
        return FVector2D(FMath::RoundToFloat(V.X), FMath::RoundToFloat(V.Y));
    }

    static void DrawRect(
        FSlateWindowElementList& Out,
        int32 LayerId,
        const FGeometry& Geo,
        const FVector2D& LocalCenter,
        const FVector2D& Size,
        const FLinearColor& Color)
    {
        if (Size.X <= 0.f || Size.Y <= 0.f || Color.A <= 0.f) return;

        const FVector2D SnappedSize(
            FMath::Max(1.f, FMath::RoundToFloat(Size.X)),
            FMath::Max(1.f, FMath::RoundToFloat(Size.Y)));
        const FVector2D SnappedCenter = SnapToPixel(LocalCenter);
        const FVector2D TopLeft = SnappedCenter - SnappedSize * 0.5f;

        const FPaintGeometry PG = Geo.ToPaintGeometry(SnappedSize, FSlateLayoutTransform(TopLeft));
        const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetDefaultBrush();
        FSlateDrawElement::MakeBox(Out, LayerId, PG, WhiteBrush, ESlateDrawEffect::None, Color);
    }

    static void DrawDisc(
        FSlateWindowElementList& Out,
        int32 LayerId,
        const FGeometry& Geo,
        const FVector2D& LocalCenter,
        float Diameter,
        const FLinearColor& Color)
    {
        if (Diameter <= 0.f || Color.A <= 0.f) return;

        const float D = FMath::Max(1.f, FMath::RoundToFloat(Diameter));
        const FVector2D Size(D, D);
        const FVector2D SnappedCenter = SnapToPixel(LocalCenter);
        const FVector2D TopLeft = SnappedCenter - Size * 0.5f;

        FSlateBrush Brush;
        Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
        const float R = D * 0.5f;
        Brush.OutlineSettings.CornerRadii = FVector4(R, R, R, R);
        Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
        Brush.OutlineSettings.Width = 0.f;

        const FPaintGeometry PG = Geo.ToPaintGeometry(Size, FSlateLayoutTransform(TopLeft));
        FSlateDrawElement::MakeBox(Out, LayerId, PG, &Brush, ESlateDrawEffect::None, Color);
    }

    static void DrawCircleRing(
        FSlateWindowElementList& Out,
        int32 LayerId,
        const FGeometry& Geo,
        const FVector2D& LocalCenter,
        float Radius,
        float Thickness,
        const FLinearColor& Color)
    {
        if (Radius <= 0.f || Thickness <= 0.f || Color.A <= 0.f) return;

        constexpr int32 Segments = 48;
        TArray<FVector2D> Points;
        Points.Reserve(Segments + 1);
        for (int32 i = 0; i <= Segments; ++i)
        {
            const float A = (float)i / (float)Segments * 2.f * PI;
            Points.Emplace(LocalCenter.X + FMath::Cos(A) * Radius,
                           LocalCenter.Y + FMath::Sin(A) * Radius);
        }
        const FPaintGeometry PG = Geo.ToPaintGeometry();
        FSlateDrawElement::MakeLines(
            Out, LayerId, PG, Points, ESlateDrawEffect::None, Color, true, Thickness);
    }

    // Draws four arms (right, left, down, up [optional]) around Center.
    // Each arm is a rectangle of (Length × Thickness), starting `Gap` from center.
    // If bOutline, also draws a slightly larger rect underneath in OutlineColor.
    static void DrawArmSet(
        FSlateWindowElementList& Out,
        int32 LayerId,
        const FGeometry& Geo,
        const FVector2D& Center,
        float Length, float Thickness, float Gap,
        const FLinearColor& Color,
        bool bOutline, const FLinearColor& OutlineColor, float OutlineThk,
        bool bIncludeTopArm)
    {
        if (Length <= 0.f || Thickness <= 0.f) return;

        const float OT = bOutline ? OutlineThk : 0.f;

        struct FArm { FVector2D Center; FVector2D Size; };
        TArray<FArm, TInlineAllocator<4>> Arms;

        // Right
        Arms.Add({ FVector2D(Center.X + Gap + Length * 0.5f, Center.Y),
                   FVector2D(Length, Thickness) });
        // Left
        Arms.Add({ FVector2D(Center.X - Gap - Length * 0.5f, Center.Y),
                   FVector2D(Length, Thickness) });
        // Down
        Arms.Add({ FVector2D(Center.X, Center.Y + Gap + Length * 0.5f),
                   FVector2D(Thickness, Length) });
        // Up
        if (bIncludeTopArm)
        {
            Arms.Add({ FVector2D(Center.X, Center.Y - Gap - Length * 0.5f),
                       FVector2D(Thickness, Length) });
        }

        if (bOutline)
        {
            for (const FArm& A : Arms)
            {
                DrawRect(Out, LayerId, Geo, A.Center,
                         A.Size + FVector2D(2.f * OT, 2.f * OT), OutlineColor);
            }
        }
        for (const FArm& A : Arms)
        {
            DrawRect(Out, LayerId + 1, Geo, A.Center, A.Size, Color);
        }
    }
}

int32 UCrosshairWidget::NativePaint(
    const FPaintArgs& Args,
    const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect,
    FSlateWindowElementList& OutDrawElements,
    int32 LayerId,
    const FWidgetStyle& InWidgetStyle,
    bool bParentEnabled) const
{
    Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
                       LayerId, InWidgetStyle, bParentEnabled);

    if (!Profile) return LayerId;

    const FCrosshairSettings& S = Profile->Crosshair;
    const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;

    auto ApplyAlpha = [&S](FLinearColor C, float ElementAlpha) -> FLinearColor
    {
        C.A *= ElementAlpha * S.MasterOpacity;
        return C;
    };

    const FLinearColor InnerCol   = ApplyAlpha(S.Color,        S.InnerLineOpacity);
    const FLinearColor OuterCol   = ApplyAlpha(S.Color,        S.OuterLineOpacity);
    const FLinearColor DotCol     = ApplyAlpha(S.Color,        S.CenterDotOpacity);
    const FLinearColor OutlineCol = ApplyAlpha(S.OutlineColor, S.OutlineOpacity);

    int32 Layer = LayerId;

    switch (S.Shape)
    {
    case ECrosshairShape::Dot:
        // Drawn by the center-dot block below; force visible with at least InnerLineThickness as size.
        break;

    case ECrosshairShape::Cross:
        DrawArmSet(OutDrawElements, Layer, AllottedGeometry, Center,
                   S.InnerLineLength, S.InnerLineThickness, S.CenterGap,
                   InnerCol, S.bOutline, OutlineCol, S.OutlineThickness, true);
        Layer += 2;
        break;

    case ECrosshairShape::Plus:
        DrawArmSet(OutDrawElements, Layer, AllottedGeometry, Center,
                   S.InnerLineLength, S.InnerLineThickness, 0.f,
                   InnerCol, S.bOutline, OutlineCol, S.OutlineThickness, true);
        Layer += 2;
        break;

    case ECrosshairShape::TShape:
        DrawArmSet(OutDrawElements, Layer, AllottedGeometry, Center,
                   S.InnerLineLength, S.InnerLineThickness, S.CenterGap,
                   InnerCol, S.bOutline, OutlineCol, S.OutlineThickness, false);
        Layer += 2;
        break;

    case ECrosshairShape::Circle:
        if (S.bOutline)
        {
            DrawCircleRing(OutDrawElements, Layer, AllottedGeometry, Center,
                           S.CircleRadius,
                           S.InnerLineThickness + 2.f * S.OutlineThickness, OutlineCol);
        }
        DrawCircleRing(OutDrawElements, Layer + 1, AllottedGeometry, Center,
                       S.CircleRadius, S.InnerLineThickness, InnerCol);
        Layer += 2;
        break;
    }

    // Outer lines (optional second tier) — drawn beyond inner lines.
    if (S.bShowOuterLines)
    {
        const float OuterGap = (S.Shape == ECrosshairShape::Plus)
            ? S.InnerLineLength + S.OuterLineOffset
            : S.CenterGap + S.InnerLineLength + S.OuterLineOffset;

        DrawArmSet(OutDrawElements, Layer, AllottedGeometry, Center,
                   S.OuterLineLength, S.OuterLineThickness, OuterGap,
                   OuterCol, S.bOutline, OutlineCol, S.OutlineThickness, true);
        Layer += 2;
    }

    // Center dot (or Dot shape) — drawn as a filled circle.
    if (S.bShowCenterDot || S.Shape == ECrosshairShape::Dot)
    {
        const float DotSize = (S.Shape == ECrosshairShape::Dot)
            ? FMath::Max(S.CenterDotSize, S.InnerLineThickness)
            : S.CenterDotSize;

        if (S.bOutline)
        {
            DrawDisc(OutDrawElements, Layer, AllottedGeometry, Center,
                     DotSize + 2.f * S.OutlineThickness, OutlineCol);
        }
        DrawDisc(OutDrawElements, Layer + 1, AllottedGeometry, Center,
                 DotSize, DotCol);
        Layer += 2;
    }

    return Layer;
}