// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WizardOsHealthBar.generated.h"

/** A simple reusable health bar for the player HUD and enemies. */
UCLASS()
class UWizardOsHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthPercent(float NewHealthPercent);

protected:
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId,
		const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

private:
	float HealthPercent = 1.f;
};
