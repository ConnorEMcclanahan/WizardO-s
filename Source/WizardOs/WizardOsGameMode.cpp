// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsGameMode.h"

#include "GameFramework/PlayerController.h"
#include "WizardOsCharacter.h"

AWizardOsGameMode::AWizardOsGameMode()
{
	DefaultPawnClass = AWizardOsCharacter::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
}