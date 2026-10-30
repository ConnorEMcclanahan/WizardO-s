// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsProjectile.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardOsEnemy.h"

AWizardOsProjectile::AWizardOsProjectile()
{
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(32.f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	Collision->OnComponentHit.AddDynamic(this, &AWizardOsProjectile::OnProjectileHit);

	ProjectileVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileVisual"));
	ProjectileVisual->SetupAttachment(Collision);
	ProjectileVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CircleMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CircleMesh.Succeeded())
	{
		ProjectileVisual->SetStaticMesh(CircleMesh.Object);
		ProjectileVisual->SetRelativeScale3D(FVector(0.64f, 0.64f, 0.06f));
	}

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->InitialSpeed = TravelSpeed;
	ProjectileMovement->MaxSpeed = TravelSpeed;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	InitialLifeSpan = 2.f;
}

void AWizardOsProjectile::SetTarget(AActor* Target)
{
	if (Target && Target->GetRootComponent())
	{
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingTargetComponent = Target->GetRootComponent();
		ProjectileMovement->HomingAccelerationMagnitude = 15000.f;
	}
}

void AWizardOsProjectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (AWizardOsEnemy* Enemy = Cast<AWizardOsEnemy>(OtherActor))
	{
		if (bExplosive)
		{
			TArray<TObjectPtr<AWizardOsEnemy>> EnemiesInBlast;
			for (TActorIterator<AWizardOsEnemy> It(GetWorld()); It; ++It)
			{
				if (FVector::DistSquared(It->GetActorLocation(), GetActorLocation()) <= FMath::Square(ExplosionRadius))
				{
					EnemiesInBlast.Add(*It);
				}
			}

			for (AWizardOsEnemy* BlastTarget : EnemiesInBlast)
			{
				BlastTarget->TakeWizardDamage(Damage);
			}
		}
		else if (RemainingChainJumps > 0)
		{
			if (AWizardOsEnemy* NextTarget = FindNextChainTarget(Enemy))
			{
				--RemainingChainJumps;
				Enemy->TakeWizardDamage(Damage);
				Collision->IgnoreActorWhenMoving(Enemy, true);
				SetActorLocation(Hit.ImpactPoint);
				SetTarget(NextTarget);
				return;
			}

			Enemy->TakeWizardDamage(Damage);
		}
		else
		{
			Enemy->TakeWizardDamage(Damage);
		}
	}

	Destroy();
}

AWizardOsEnemy* AWizardOsProjectile::FindNextChainTarget(const AWizardOsEnemy* HitEnemy) const
{
	AWizardOsEnemy* NextTarget = nullptr;
	float ClosestDistanceSquared = FMath::Square(800.f);
	for (TActorIterator<AWizardOsEnemy> It(GetWorld()); It; ++It)
	{
		if (*It == HitEnemy)
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(HitEnemy->GetActorLocation(), It->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			NextTarget = *It;
		}
	}

	return NextTarget;
}
