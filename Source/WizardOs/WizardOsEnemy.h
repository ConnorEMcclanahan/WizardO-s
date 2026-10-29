// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WizardOsEnemy.generated.h"

class UWidgetComponent;
class UWizardOsHealthBar;
class AWizardOsCharacter;
class AAIController;
class UBlendSpace;

/** A slow melee mannequin that chases and damages the player. */
UCLASS(Blueprintable)
class AWizardOsEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AWizardOsEnemy();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Applies projectile damage to the target. */
	UFUNCTION(BlueprintCallable, Category="Combat")
	void TakeWizardDamage(float Damage);
	void ConfigureAsBoss();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true", ClampMin="1.0"))
	float MaxHealth = 3.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat", meta=(AllowPrivateAccess="true"))
	float Health = 3.f;

	UPROPERTY()
	TObjectPtr<UWizardOsHealthBar> HealthBar;

	UPROPERTY()
	TObjectPtr<UBlendSpace> LocomotionBlendSpace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true", ClampMin="0.0", Units="cm/s"))
	float MoveSpeed = 180.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI", meta=(AllowPrivateAccess="true", ClampMin="0.0", Units="cm"))
	float MeleeRange = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true", ClampMin="0.1", Units="s"))
	float MeleeInterval = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(AllowPrivateAccess="true", ClampMin="0.0"))
	float MeleeDamage = 1.f;

	bool bIsBoss = false;
	float BossAoERange = 340.f;
	float BossAoEDamage = 2.f;
	float BossAoEInterval = 2.5f;

	TObjectPtr<AWizardOsCharacter> TargetPlayer;
	float NextMeleeTime = 0.f;
	float NextBossAoETime = 0.f;
	TObjectPtr<AAIController> EnemyAIController;
	float NextPathUpdateTime = 0.f;

	void UpdateHealthBar();
};
