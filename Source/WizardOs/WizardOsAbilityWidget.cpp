// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsAbilityWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "WizardOsCharacter.h"

namespace
{
UButton* CreateUpgradeCard(UWidgetTree* WidgetTree, const FName Name, UTexture2D* CardArt)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%sSize"), *Name.ToString()));
	SizeBox->SetWidthOverride(220.f);
	SizeBox->SetHeightOverride(342.f);
	UImage* Artwork = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), *FString::Printf(TEXT("%sArtwork"), *Name.ToString()));
	Artwork->SetBrushFromTexture(CardArt, true);
	SizeBox->SetContent(Artwork);
	Button->SetContent(SizeBox);
	return Button;
}
}

void UWizardOsAbilityWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	AppearanceTime = FMath::Min(AppearanceTime + InDeltaTime, 0.3f);
	const float Progress = FMath::InterpEaseOut(0.f, 1.f, AppearanceTime / 0.3f, 3.f);
	SetRenderOpacity(Progress);
	SetRenderScale(FVector2D(FMath::Lerp(0.82f, 1.f, Progress)));
}

void UWizardOsAbilityWidget::Configure(AWizardOsCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
}

void UWizardOsAbilityWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree->RootWidget)
	{
		RapidCastingButton = Cast<UButton>(GetWidgetFromName(TEXT("RapidCasting")));
		EmpoweredBoltButton = Cast<UButton>(GetWidgetFromName(TEXT("EmpoweredBolt")));
		VitalityButton = Cast<UButton>(GetWidgetFromName(TEXT("Vitality")));

		if (RapidCastingButton && EmpoweredBoltButton && VitalityButton)
		{
			RapidCastingButton->OnClicked.AddUniqueDynamic(this, &UWizardOsAbilityWidget::SelectRapidCasting);
			EmpoweredBoltButton->OnClicked.AddUniqueDynamic(this, &UWizardOsAbilityWidget::SelectEmpoweredBolt);
			VitalityButton->OnClicked.AddUniqueDynamic(this, &UWizardOsAbilityWidget::SelectVitality);
			return;
		}

		WidgetTree->RootWidget = nullptr;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RewardScreen"));
	WidgetTree->RootWidget = Root;

	UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ReadabilityScrim"));
	Scrim->SetBrushColor(FLinearColor(0.012f, 0.027f, 0.067f, 0.72f));
	UCanvasPanelSlot* ScrimSlot = Root->AddChildToCanvas(Scrim);
	ScrimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	ScrimSlot->SetOffsets(FMargin(0.f));

	USizeBox* RewardPanelSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RewardPanelSize"));
	RewardPanelSize->SetWidthOverride(900.f);
	RewardPanelSize->SetHeightOverride(500.f);

	UVerticalBox* Choices = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Choices"));
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CardPanel"));
	Panel->SetBrushColor(FLinearColor(0.01f, 0.025f, 0.075f, 0.98f));
	Panel->SetPadding(FMargin(30.f, 18.f));
	Panel->SetContent(Choices);
	RewardPanelSize->SetContent(Panel);
	UCanvasPanelSlot* RewardPanelSlot = Root->AddChildToCanvas(RewardPanelSize);
	RewardPanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	RewardPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	RewardPanelSlot->SetPosition(FVector2D::ZeroVector);

	UHorizontalBox* Cards = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Cards"));
	Choices->AddChildToVerticalBox(Cards);

	UTexture2D* FastHandsArt = LoadObject<UTexture2D>(nullptr,
		TEXT("/Game/UI/AbilityCards/clovercard.clovercard"));
	UTexture2D* SharpCardsArt = LoadObject<UTexture2D>(nullptr,
		TEXT("/Game/UI/AbilityCards/QVKrxeqj.QVKrxeqj"));
	UTexture2D* SecondWindArt = LoadObject<UTexture2D>(nullptr,
		TEXT("/Game/UI/AbilityCards/heartcard.heartcard"));

	RapidCastingButton = CreateUpgradeCard(WidgetTree, TEXT("RapidCasting"), FastHandsArt);
	RapidCastingButton->OnClicked.AddDynamic(this, &UWizardOsAbilityWidget::SelectRapidCasting);
	UHorizontalBoxSlot* RapidSlot = Cards->AddChildToHorizontalBox(RapidCastingButton);
	RapidSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	RapidSlot->SetPadding(FMargin(8.f));

	EmpoweredBoltButton = CreateUpgradeCard(WidgetTree, TEXT("EmpoweredBolt"), SharpCardsArt);
	EmpoweredBoltButton->OnClicked.AddDynamic(this, &UWizardOsAbilityWidget::SelectEmpoweredBolt);
	UHorizontalBoxSlot* BoltSlot = Cards->AddChildToHorizontalBox(EmpoweredBoltButton);
	BoltSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	BoltSlot->SetPadding(FMargin(8.f));

	VitalityButton = CreateUpgradeCard(WidgetTree, TEXT("Vitality"), SecondWindArt);
	VitalityButton->OnClicked.AddDynamic(this, &UWizardOsAbilityWidget::SelectVitality);
	UHorizontalBoxSlot* VitalSlot = Cards->AddChildToHorizontalBox(VitalityButton);
	VitalSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	VitalSlot->SetPadding(FMargin(8.f));
}

void UWizardOsAbilityWidget::SelectRapidCasting()
{
	SelectAbility(0);
}

void UWizardOsAbilityWidget::SelectEmpoweredBolt()
{
	SelectAbility(1);
}

void UWizardOsAbilityWidget::SelectVitality()
{
	SelectAbility(2);
}

void UWizardOsAbilityWidget::SelectAbility(int32 AbilityIndex)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->ApplyAbility(AbilityIndex);
	}
}
