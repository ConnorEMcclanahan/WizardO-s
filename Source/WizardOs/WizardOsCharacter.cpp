// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "NavigationSystem.h"
#include "WizardOsEnemy.h"
#include "WizardOsAbilityWidget.h"
#include "WizardOsHealthBar.h"
#include "WizardOsProjectile.h"
#include "WizardOsWaveWidget.h"

AWizardOsCharacter::AWizardOsCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;
	GetCharacterMovement()->MaxWalkSpeed = 650.f;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> WizardMesh(
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> WizardAnimation(
		TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	if (WizardMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(WizardMesh.Object);
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}
	if (WizardAnimation.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(WizardAnimation.Class);
	}

	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create the camera component
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AWizardOsCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerHealthBar = CreateWidget<UWizardOsHealthBar>(PlayerController);
		PlayerHealthBar->SetDesiredSizeInViewport(FVector2D(260.f, 22.f));
		PlayerHealthBar->SetPositionInViewport(FVector2D(24.f, 24.f));
		PlayerHealthBar->AddToViewport();
		UpdateHealthBar();

		WaveWidget = CreateWidget<UWizardOsWaveWidget>(PlayerController);
		WaveWidget->SetDesiredSizeInViewport(FVector2D(180.f, 40.f));
		WaveWidget->SetAlignmentInViewport(FVector2D(1.f, 0.f));
		WaveWidget->SetPositionInViewport(FVector2D(-24.f, 24.f), true);
		WaveWidget->AddToViewport();
		UpdateWaveWidget();
	}

	if (HasAuthority())
	{
		StartCurrentWave();
	}
}

void AWizardOsCharacter::TakeWizardDamage(float Damage)
{
	Health = FMath::Max(0.f, Health - Damage);
	UpdateHealthBar();
}

void AWizardOsCharacter::ShowAbilityChoices()
{
	if (AbilityWidget)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		AbilityWidget = CreateWidget<UWizardOsAbilityWidget>(PlayerController);
		AbilityWidget->Configure(this);

		int32 ViewportWidth;
		int32 ViewportHeight;
		PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
		AbilityWidget->SetDesiredSizeInViewport(FVector2D(ViewportWidth, ViewportHeight));
		AbilityWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
		AbilityWidget->SetPositionInViewport(FVector2D::ZeroVector);
		AbilityWidget->AddToViewport(100);

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
	}
}

void AWizardOsCharacter::ApplyAbility(int32 AbilityIndex)
{
	switch (AbilityIndex)
	{
	case 0:
		FireInterval = FMath::Max(0.25f, FireInterval * 0.75f);
		break;
	case 1:
		ProjectileDamage += 2.f;
		break;
	case 2:
		MaxHealth += 2.f;
		Health += 5.f;
		Health = FMath::Min(Health, MaxHealth);
		UpdateHealthBar();
		break;
	default:
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}

	AbilityWidget->RemoveFromParent();
	AbilityWidget = nullptr;
	bWaitingForUpgrade = false;
	++CurrentWave;
	StartCurrentWave();
}

void AWizardOsCharacter::NotifyEnemyDefeated()
{
	RemainingEnemies = FMath::Max(0, RemainingEnemies - 1);
	if (RemainingEnemies == 0 && !bWaitingForUpgrade)
	{
		bWaitingForUpgrade = true;
		ShowAbilityChoices();
	}
}

void AWizardOsCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float GameTime = GetWorld()->GetTimeSeconds();
	AWizardOsEnemy* NearestEnemy = FindNearestEnemy();
	if (NearestEnemy)
	{
		const FVector DirectionToEnemy = (NearestEnemy->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (!DirectionToEnemy.IsNearlyZero())
		{
			const FRotator TargetRotation = DirectionToEnemy.Rotation();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaSeconds, FacingInterpolationSpeed));
		}
	}

	if (GetVelocity().SizeSquared2D() > 1.f)
	{
		IdleStartTime = GameTime;
		return;
	}

	if (GameTime - IdleStartTime < FireInterval || GameTime - LastFireTime < FireInterval)
	{
		return;
	}

	if (NearestEnemy)
	{
		FireAt(NearestEnemy);
	}
}

void AWizardOsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AWizardOsCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AWizardOsCharacter::MoveRight);
}

void AWizardOsCharacter::MoveForward(float Value)
{
	AddMovementInput(FVector::ForwardVector, Value);
}

void AWizardOsCharacter::MoveRight(float Value)
{
	AddMovementInput(FVector::RightVector, Value);
}

AWizardOsEnemy* AWizardOsCharacter::FindNearestEnemy() const
{
	AWizardOsEnemy* NearestEnemy = nullptr;
	float ClosestDistanceSquared = FMath::Square(AttackRange);

	for (TActorIterator<AWizardOsEnemy> It(GetWorld()); It; ++It)
	{
		if (!IsEnemyVisibleOnScreen(*It))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared2D(GetActorLocation(), It->GetActorLocation());
		if (DistanceSquared < ClosestDistanceSquared)
		{
			ClosestDistanceSquared = DistanceSquared;
			NearestEnemy = *It;
		}
	}

	return NearestEnemy;
}

void AWizardOsCharacter::FireAt(AWizardOsEnemy* Enemy)
{
	const FVector Direction = (Enemy->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const FVector SpawnLocation = GetActorLocation() + Direction * 100.f;
	if (AWizardOsProjectile* Projectile = GetWorld()->SpawnActor<AWizardOsProjectile>(
		AWizardOsProjectile::StaticClass(), SpawnLocation, Direction.Rotation(), SpawnParameters))
	{
		Projectile->SetDamage(ProjectileDamage);
		Projectile->SetTarget(Enemy);
	}
	LastFireTime = GetWorld()->GetTimeSeconds();
}

void AWizardOsCharacter::UpdateHealthBar()
{
	if (PlayerHealthBar)
	{
		PlayerHealthBar->SetHealthPercent(Health / MaxHealth);
	}
}

void AWizardOsCharacter::StartCurrentWave()
{
	const bool bBossWave = CurrentWave >= 3;
	const int32 EnemyCount = CurrentWave == 1 ? 1 : (CurrentWave == 2 ? 2 : 1);
	RemainingEnemies = 0;
	UpdateWaveWidget();

	for (int32 EnemyIndex = 0; EnemyIndex < EnemyCount; ++EnemyIndex)
	{
		FVector EnemyLocation;
		if (!FindSafeEnemySpawnLocation(EnemyLocation))
		{
			continue;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		if (AWizardOsEnemy* Enemy = GetWorld()->SpawnActor<AWizardOsEnemy>(
			AWizardOsEnemy::StaticClass(), EnemyLocation, FRotator::ZeroRotator,
			SpawnParameters))
		{
			++RemainingEnemies;
			if (bBossWave)
			{
				Enemy->ConfigureAsBoss();
			}
		}
	}
}

bool AWizardOsCharacter::IsEnemyVisibleOnScreen(const AWizardOsEnemy* Enemy) const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !Enemy)
	{
		return false;
	}

	FVector2D ScreenPosition;
	int32 ViewportWidth;
	int32 ViewportHeight;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
	return PlayerController->ProjectWorldLocationToScreen(Enemy->GetActorLocation(), ScreenPosition, true) &&
		ScreenPosition.X >= 0.f && ScreenPosition.X <= ViewportWidth &&
		ScreenPosition.Y >= 0.f && ScreenPosition.Y <= ViewportHeight;
}

bool AWizardOsCharacter::FindSafeEnemySpawnLocation(FVector& OutLocation) const
{
	UNavigationSystemV1* NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavigationSystem)
	{
		return false;
	}

	for (int32 Attempt = 0; Attempt < 16; ++Attempt)
	{
		FNavLocation Candidate;
		if (NavigationSystem->GetRandomReachablePointInRadius(GetActorLocation(), 1100.f, Candidate) &&
			FVector::DistSquared2D(Candidate.Location, GetActorLocation()) >= FMath::Square(600.f))
		{
			OutLocation = Candidate.Location;
			return true;
		}
	}

	return false;
}

void AWizardOsCharacter::UpdateWaveWidget()
{
	if (WaveWidget)
	{
		WaveWidget->SetWave(CurrentWave, CurrentWave >= 3);
	}
}
