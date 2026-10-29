// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsHealthBar.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void UWizardOsHealthBar::SetHealthPercent(float NewHealthPercent)
{
	HealthPercent = FMath::Clamp(NewHealthPercent, 0.f, 1.f);
}

int32 UWizardOsHealthBar::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	const FVector2D Size = AllottedGeometry.GetLocalSize();

	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), WhiteBrush,
		ESlateDrawEffect::None, FLinearColor(0.12f, 0.02f, 0.02f, 0.9f));
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
		AllottedGeometry.ToPaintGeometry(FVector2f(Size.X * HealthPercent, Size.Y), FSlateLayoutTransform()), WhiteBrush,
		ESlateDrawEffect::None, FLinearColor(0.05f, 0.8f, 0.12f, 1.f));

	return LayerId + 1;
}
