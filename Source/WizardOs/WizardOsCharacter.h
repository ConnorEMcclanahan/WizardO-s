// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WizardOsCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class AWizardOsEnemy;
class UWizardOsHealthBar;
class UWizardOsAbilityWidget;
class UWizardOsWaveWidget;

/**
 * A controllable top-down wizard. It moves with WASD and attacks while idle.
 */
UCLASS()
class AWizardOsCharacter : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Seconds between automatic shots while the wizard is standing still. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true", ClampMin="0.05", Units="s"))
	float FireInterval = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float FacingInterpolationSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true", ClampMin="1.0"))
	float ProjectileDamage = 1.f;

	/** Maximum distance at which the wizard will target an enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true", ClampMin="0.0", Units="cm"))
	float AttackRange = 1800.f;

	float LastFireTime = -1.f;
	float IdleStartTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health", meta=(AllowPrivateAccess="true", ClampMin="1.0"))
	float MaxHealth = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
	float Health = 10.f;

	UPROPERTY()
	TObjectPtr<UWizardOsHealthBar> PlayerHealthBar;

	UPROPERTY()
	TObjectPtr<UWizardOsAbilityWidget> AbilityWidget;

	UPROPERTY()
	TObjectPtr<UWizardOsWaveWidget> WaveWidget;

	int32 CurrentWave = 1;
	int32 RemainingEnemies = 0;
	bool bWaitingForUpgrade = false;

public:

	/** Constructor */
	AWizardOsCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Binds the keyboard movement axes declared in DefaultInput.ini. */
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Allows future enemies to damage the player. */
	UFUNCTION(BlueprintCallable, Category="Health")
	void TakeWizardDamage(float Damage);

	/** Opens the post-kill roguelike upgrade choice screen. */
	void ShowAbilityChoices();

	/** Applies one option from the post-kill upgrade choice screen. */
	void ApplyAbility(int32 AbilityIndex);
	void NotifyEnemyDefeated();

	/** Returns the camera component **/
	UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent.Get(); }

	/** Returns the Camera Boom component **/
	USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }

private:

	void MoveForward(float Value);
	void MoveRight(float Value);
	AWizardOsEnemy* FindNearestEnemy() const;
	void FireAt(AWizardOsEnemy* Enemy);
	void UpdateHealthBar();
	void StartCurrentWave();
	void UpdateWaveWidget();
	bool IsEnemyVisibleOnScreen(const AWizardOsEnemy* Enemy) const;
	bool FindSafeEnemySpawnLocation(FVector& OutLocation) const;
};
