// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WizardOsMerchantWidget.generated.h"

class AWizardOsCharacter;
class UButton;
class UTextBlock;

/** Presents the traveling merchant's one-time chip purchases between waves. */
UCLASS()
class UWizardOsMerchantWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Configure(AWizardOsCharacter* InOwnerCharacter);

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void BuyChainSpark();

	UFUNCTION()
	void BuyRunebomb();

	UFUNCTION()
	void BuyTwinCast();

	UFUNCTION()
	void LeaveShop();

	void BuyItem(int32 ItemIndex);
	void UpdateAvailability();

	UPROPERTY()
	TObjectPtr<AWizardOsCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UTextBlock> ChipLabel;

	UPROPERTY()
	TArray<TObjectPtr<UButton>> ItemButtons;
};
