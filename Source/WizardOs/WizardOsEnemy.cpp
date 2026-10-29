// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsEnemy.h"

#include "AIController.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/BlendSpace.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "WizardOsCharacter.h"
#include "WizardOsHealthBar.h"

AWizardOsEnemy::AWizardOsEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bCanWalkOffLedges = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> EnemyMesh(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FObjectFinder<UBlendSpace> EnemyLocomotion(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/BS_Idle_Walk_Run.BS_Idle_Walk_Run"));
	if (EnemyMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(EnemyMesh.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}
	if (EnemyLocomotion.Succeeded())
	{
		LocomotionBlendSpace = EnemyLocomotion.Object;
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->PlayAnimation(LocomotionBlendSpace, true);
	}

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(RootComponent);
	HealthBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(120.f, 14.f));
	HealthBarComponent->SetWidgetClass(UWizardOsHealthBar::StaticClass());
}

void AWizardOsEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (LocomotionBlendSpace)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		GetMesh()->PlayAnimation(LocomotionBlendSpace, true);
	}

	HealthBar = Cast<UWizardOsHealthBar>(HealthBarComponent->GetUserWidgetObject());
	UpdateHealthBar();
	TargetPlayer = Cast<AWizardOsCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	EnemyAIController = Cast<AAIController>(GetController());
}

void AWizardOsEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (UAnimSingleNodeInstance* AnimationInstance = GetMesh()->GetSingleNodeInstance())
	{
		const float Speed = GetVelocity().Size2D();
		AnimationInstance->SetBlendSpacePosition(FVector(Speed, Speed, 0.f));
	}

	if (!TargetPlayer || TargetPlayer->IsActorBeingDestroyed())
	{
		return;
	}

	const FVector ToPlayer = TargetPlayer->GetActorLocation() - GetActorLocation();
	const float DistanceToPlayer = ToPlayer.Size2D();
	if (DistanceToPlayer > MeleeRange)
	{
		if (!EnemyAIController)
		{
			EnemyAIController = Cast<AAIController>(GetController());
		}

		if (EnemyAIController && GetWorld()->GetTimeSeconds() >= NextPathUpdateTime)
		{
			EnemyAIController->MoveToActor(TargetPlayer, MeleeRange, true, true, false, nullptr, true);
			NextPathUpdateTime = GetWorld()->GetTimeSeconds() + 0.5f;
		}
		return;
	}

	if (EnemyAIController)
	{
		EnemyAIController->StopMovement();
	}

	if (GetWorld()->GetTimeSeconds() >= NextMeleeTime)
	{
		if (bIsBoss && DistanceToPlayer <= BossAoERange && GetWorld()->GetTimeSeconds() >= NextBossAoETime)
		{
			TargetPlayer->TakeWizardDamage(BossAoEDamage);
			NextBossAoETime = GetWorld()->GetTimeSeconds() + BossAoEInterval;
		}
		else if (!bIsBoss)
		{
			TargetPlayer->TakeWizardDamage(MeleeDamage);
		}
		NextMeleeTime = GetWorld()->GetTimeSeconds() + MeleeInterval;
	}
}

void AWizardOsEnemy::ConfigureAsBoss()
{
	bIsBoss = true;
	MaxHealth = 18.f;
	Health = MaxHealth;
	MoveSpeed = 140.f;
	MeleeRange = BossAoERange;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	SetActorScale3D(FVector(2.f));
	UpdateHealthBar();
}

void AWizardOsEnemy::TakeWizardDamage(float Damage)
{
	Health -= Damage;
	UpdateHealthBar();
	if (Health <= 0.f)
	{
		if (TargetPlayer)
		{
			TargetPlayer->NotifyEnemyDefeated();
		}
		Destroy();
	}
}

void AWizardOsEnemy::UpdateHealthBar()
{
	if (HealthBar)
	{
		HealthBar->SetHealthPercent(Health / MaxHealth);
	}
}
