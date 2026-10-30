// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsWaveWidget.h"

#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

void UWizardOsWaveWidget::SetWave(int32 WaveNumber, bool bIsBossWave, int32 ChipCount)
{
	bBoss = bIsBossWave;
	const FText WaveLabel = bBoss ? FText::FromString(TEXT("BOSS WAVE")) :
		FText::Format(FText::FromString(TEXT("WAVE {0}")), WaveNumber);
	Label = FText::Format(FText::FromString(TEXT("{0}\nCHIPS: {1}")), WaveLabel, ChipCount);
}

int32 UWizardOsWaveWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
	const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush(TEXT("WhiteBrush"));
	FSlateDrawElement::MakeBox(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), WhiteBrush,
		ESlateDrawEffect::None, FLinearColor(0.03f, 0.05f, 0.12f, 0.85f));
	FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1, AllottedGeometry.ToPaintGeometry(),
		Label, FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 20), ESlateDrawEffect::None,
		bBoss ? FLinearColor(1.f, 0.3f, 0.15f) : FLinearColor::White);
	return LayerId + 1;
}
