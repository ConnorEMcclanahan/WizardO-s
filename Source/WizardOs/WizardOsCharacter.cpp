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
#include "WizardOsHomeWidget.h"
#include "WizardOsMerchantWidget.h"
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
		PlayerHealthBar->SetVisibility(ESlateVisibility::Hidden);

		WaveWidget = CreateWidget<UWizardOsWaveWidget>(PlayerController);
		WaveWidget->SetDesiredSizeInViewport(FVector2D(180.f, 40.f));
		WaveWidget->SetAlignmentInViewport(FVector2D(1.f, 0.f));
		WaveWidget->SetPositionInViewport(FVector2D(-24.f, 24.f), true);
		WaveWidget->AddToViewport();
		UpdateWaveWidget();
		WaveWidget->SetVisibility(ESlateVisibility::Hidden);
	}

	if (HasAuthority())
	{
		ShowHomeScreen();
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
		TSubclassOf<UWizardOsAbilityWidget> AbilityWidgetClass = UWizardOsAbilityWidget::StaticClass();
		if (TSubclassOf<UWizardOsAbilityWidget> DesignerWidgetClass = LoadClass<UWizardOsAbilityWidget>(
			nullptr, TEXT("/Game/UI/WBP_AbilityChoices.WBP_AbilityChoices_C")))
		{
			AbilityWidgetClass = DesignerWidgetClass;
		}

		AbilityWidget = CreateWidget<UWizardOsAbilityWidget>(PlayerController, AbilityWidgetClass);
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

	if (FMath::FRand() < MerchantVisitChance)
	{
		ShowMerchant();
	}
	else
	{
		ContinueToNextWave();
	}
}

void AWizardOsCharacter::NotifyEnemyDefeated(int32 ChipsEarned)
{
	PocketChips += FMath::Max(0, ChipsEarned);
	UpdateWaveWidget();
	RemainingEnemies = FMath::Max(0, RemainingEnemies - 1);
	if (RemainingEnemies == 0 && !bWaitingForUpgrade)
	{
		bWaitingForUpgrade = true;
		ShowAbilityChoices();
	}
}

void AWizardOsCharacter::ShowMerchant()
{
	if (MerchantWidget)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		TSubclassOf<UWizardOsMerchantWidget> MerchantWidgetClass = UWizardOsMerchantWidget::StaticClass();
		if (TSubclassOf<UWizardOsMerchantWidget> DesignerWidgetClass = LoadClass<UWizardOsMerchantWidget>(
			nullptr, TEXT("/Game/UI/WBP_MerchantShop.WBP_MerchantShop_C")))
		{
			MerchantWidgetClass = DesignerWidgetClass;
		}

		MerchantWidget = CreateWidget<UWizardOsMerchantWidget>(PlayerController, MerchantWidgetClass);
		MerchantWidget->Configure(this);

		int32 ViewportWidth;
		int32 ViewportHeight;
		PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);
		MerchantWidget->SetDesiredSizeInViewport(FVector2D(ViewportWidth, ViewportHeight));
		MerchantWidget->SetAlignmentInViewport(FVector2D::ZeroVector);
		MerchantWidget->SetPositionInViewport(FVector2D::ZeroVector);
		MerchantWidget->AddToViewport(100);

		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
	}
}

void AWizardOsCharacter::ShowHomeScreen()
{
	if (HomeWidget)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		TSubclassOf<UWizardOsHomeWidget> HomeWidgetClass = UWizardOsHomeWidget::StaticClass();
		if (TSubclassOf<UWizardOsHomeWidget> DesignerWidgetClass = LoadClass<UWizardOsHomeWidget>(
			nullptr, TEXT("/Game/UI/WBP_HomeScreen.WBP_HomeScreen_C")))
		{
			HomeWidgetClass = DesignerWidgetClass;
		}

		HomeWidget = CreateWidget<UWizardOsHomeWidget>(PlayerController, HomeWidgetClass);
		HomeWidget->Configure(this);
		HomeWidget->AddToViewport(200);

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(HomeWidget->TakeWidget());
		PlayerController->SetInputMode(InputMode);
		PlayerController->bShowMouseCursor = true;
	}
}

void AWizardOsCharacter::StartGameFromHome()
{
	if (bGameStarted)
	{
		return;
	}

	bGameStarted = true;
	if (HomeWidget)
	{
		HomeWidget->RemoveFromParent();
		HomeWidget = nullptr;
	}

	if (PlayerHealthBar)
	{
		PlayerHealthBar->SetVisibility(ESlateVisibility::Visible);
	}
	if (WaveWidget)
	{
		WaveWidget->SetVisibility(ESlateVisibility::Visible);
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}

	StartCurrentWave();
}

void AWizardOsCharacter::BuyMerchantItem(int32 ItemIndex)
{
	static constexpr int32 ItemCosts[] = { 5, 7, 6 };
	if (ItemIndex < 0 || ItemIndex >= UE_ARRAY_COUNT(ItemCosts) || IsMerchantItemOwned(ItemIndex) ||
		PocketChips < ItemCosts[ItemIndex])
	{
		return;
	}

	PocketChips -= ItemCosts[ItemIndex];
	switch (ItemIndex)
	{
	case 0:
		bHasChainSpark = true;
		break;
	case 1:
		bHasRunebomb = true;
		break;
	case 2:
		bHasTwinCast = true;
		break;
	default:
		return;
	}

	UpdateWaveWidget();
	LeaveMerchant();
}

bool AWizardOsCharacter::IsMerchantItemOwned(int32 ItemIndex) const
{
	switch (ItemIndex)
	{
	case 0:
		return bHasChainSpark;
	case 1:
		return bHasRunebomb;
	case 2:
		return bHasTwinCast;
	default:
		return false;
	}
}

void AWizardOsCharacter::LeaveMerchant()
{
	if (MerchantWidget)
	{
		MerchantWidget->RemoveFromParent();
		MerchantWidget = nullptr;
	}

	ContinueToNextWave();
}

void AWizardOsCharacter::ContinueToNextWave()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}

	bWaitingForUpgrade = false;
	++CurrentWave;
	StartCurrentWave();
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
	const int32 ShotCount = bHasTwinCast && FMath::FRand() < 0.3f ? 2 : 1;
	for (int32 ShotIndex = 0; ShotIndex < ShotCount; ++ShotIndex)
	{
		if (AWizardOsProjectile* Projectile = GetWorld()->SpawnActor<AWizardOsProjectile>(
			AWizardOsProjectile::StaticClass(), SpawnLocation, Direction.Rotation(), SpawnParameters))
		{
			Projectile->SetDamage(ProjectileDamage);
			Projectile->SetChainJumps(bHasChainSpark ? 2 : 0);
			Projectile->SetExplosive(bHasRunebomb && FMath::FRand() < 0.25f);
			Projectile->SetTarget(Enemy);
		}
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
		WaveWidget->SetWave(CurrentWave, CurrentWave >= 3, PocketChips);
	}
}
