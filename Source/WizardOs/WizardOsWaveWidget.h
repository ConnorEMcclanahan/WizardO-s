// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WizardOsWaveWidget.generated.h"

/** Displays the current encounter wave in the top-right corner. */
UCLASS()
class UWizardOsWaveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetWave(int32 WaveNumber, bool bIsBossWave, int32 ChipCount);

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
		const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	FText Label;
	bool bBoss = false;
};
