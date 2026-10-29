// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WizardOsAbilityWidget.generated.h"

class AWizardOsCharacter;
class UButton;

/** A temporary C++ roguelike reward screen with three selectable upgrades. */
UCLASS()
class UWizardOsAbilityWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Configure(AWizardOsCharacter* InOwnerCharacter);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void SelectRapidCasting();

	UFUNCTION()
	void SelectEmpoweredBolt();

	UFUNCTION()
	void SelectVitality();

	void SelectAbility(int32 AbilityIndex);

	UPROPERTY()
	TObjectPtr<AWizardOsCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UButton> RapidCastingButton;

	UPROPERTY()
	TObjectPtr<UButton> EmpoweredBoltButton;

	UPROPERTY()
	TObjectPtr<UButton> VitalityButton;

	float AppearanceTime = 0.f;
};
