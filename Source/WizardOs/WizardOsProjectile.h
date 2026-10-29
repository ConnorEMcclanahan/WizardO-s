// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WizardOsProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

/** A small visible magic bolt used by the prototype auto-attack. */
UCLASS()
class AWizardOsProjectile : public AActor
{
	GENERATED_BODY()

public:
	AWizardOsProjectile();

	/** Makes this spell curve toward the specified enemy until it collides. */
	void SetTarget(AActor* Target);
	void SetDamage(float NewDamage) { Damage = NewDamage; }

private:
	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> ProjectileVisual;

	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, Category="Combat", meta=(ClampMin="1.0"))
	float Damage = 1.f;

	UPROPERTY(EditAnywhere, Category="Combat", meta=(ClampMin="1.0", Units="cm/s"))
	float TravelSpeed = 500.f;
};
