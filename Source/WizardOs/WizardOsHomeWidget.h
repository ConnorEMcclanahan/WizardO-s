// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WizardOsHomeWidget.generated.h"

class AWizardOsCharacter;
class UButton;

/** The editable start menu shown before the first wave. */
UCLASS()
class UWizardOsHomeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Configure(AWizardOsCharacter* InOwnerCharacter);

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void StartGame();

	UFUNCTION()
	void QuitGame();

	UPROPERTY()
	TObjectPtr<AWizardOsCharacter> OwnerCharacter;
};
