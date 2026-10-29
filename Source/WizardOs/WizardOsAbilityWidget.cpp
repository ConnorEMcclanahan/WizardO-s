// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsAbilityWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "WizardOsCharacter.h"

namespace
{
UTextBlock* CreateCardText(UWidgetTree* WidgetTree, const FName Name, const FString& Text,
	const FLinearColor& Color, int32 FontSize)
{
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Label->SetText(FText::FromString(Text));
	Label->SetColorAndOpacity(FSlateColor(Color));
	Label->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), FontSize)));
	Label->SetJustification(ETextJustify::Center);
	return Label;
}

UButton* CreateUpgradeCard(UWidgetTree* WidgetTree, const FName Name, const FString& Suit,
	const FString& Title, const FString& Effect, const FString& Description, const FLinearColor& FaceColor,
	const FLinearColor& AccentColor)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%sSize"), *Name.ToString()));
	SizeBox->SetWidthOverride(220.f);
	SizeBox->SetHeightOverride(360.f);

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%sFrame"), *Name.ToString()));
	Frame->SetBrushColor(FLinearColor(0.86f, 0.77f, 0.61f));
	Frame->SetPadding(FMargin(5.f));
	UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%sFace"), *Name.ToString()));
	Card->SetBrushColor(FaceColor);
	Card->SetPadding(FMargin(12.f));
	UVerticalBox* Contents = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("%sContents"), *Name.ToString()));
	Card->SetContent(Contents);

	UTextBlock* SuitLabel = CreateCardText(WidgetTree, *FString::Printf(TEXT("%sSuit"), *Name.ToString()), Suit, AccentColor, 22);
	SuitLabel->SetJustification(ETextJustify::Left);
	Contents->AddChildToVerticalBox(SuitLabel);
	Contents->AddChildToVerticalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), *FString::Printf(TEXT("%sArtSpace"), *Name.ToString())))
		->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UBorder* NamePlate = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%sNamePlate"), *Name.ToString()));
	NamePlate->SetBrushColor(FLinearColor(0.005f, 0.008f, 0.02f, 0.94f));
	NamePlate->SetPadding(FMargin(6.f));
	UVerticalBox* NamePlateContents = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("%sNamePlateContents"), *Name.ToString()));
	NamePlate->SetContent(NamePlateContents);
	NamePlateContents->AddChildToVerticalBox(CreateCardText(WidgetTree, *FString::Printf(TEXT("%sTitle"), *Name.ToString()), Title, FLinearColor(0.95f, 0.84f, 0.72f), 18));
	NamePlateContents->AddChildToVerticalBox(CreateCardText(WidgetTree, *FString::Printf(TEXT("%sEffect"), *Name.ToString()), Effect, AccentColor, 14));
	NamePlateContents->AddChildToVerticalBox(CreateCardText(WidgetTree, *FString::Printf(TEXT("%sDescription"), *Name.ToString()), Description, FLinearColor(0.9f, 0.9f, 0.92f), 12));
	Contents->AddChildToVerticalBox(NamePlate);

	Frame->SetContent(Card);
	SizeBox->SetContent(Frame);
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

	Choices->AddChildToVerticalBox(CreateCardText(WidgetTree, TEXT("Title"),
		TEXT("PICK AN UPGRADE"), FLinearColor(0.95f, 0.84f, 0.72f), 32));
	Choices->AddChildToVerticalBox(CreateCardText(WidgetTree, TEXT("Subtitle"),
		TEXT("New cards. New tricks. Stronger hands."), FLinearColor(0.45f, 0.7f, 1.f), 16));

	UHorizontalBox* Cards = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Cards"));
	Choices->AddChildToVerticalBox(Cards);

	RapidCastingButton = CreateUpgradeCard(WidgetTree, TEXT("RapidCasting"), TEXT("K\nSPADE"),
		TEXT("ARCANE TEMPO"), TEXT("+25% ATTACK SPEED"), TEXT("Throw your spell cards faster."),
		FLinearColor(0.025f, 0.07f, 0.17f), FLinearColor(0.25f, 0.9f, 0.75f));
	RapidCastingButton->OnClicked.AddDynamic(this, &UWizardOsAbilityWidget::SelectRapidCasting);
	UHorizontalBoxSlot* RapidSlot = Cards->AddChildToHorizontalBox(RapidCastingButton);
	RapidSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	RapidSlot->SetPadding(FMargin(8.f));

	EmpoweredBoltButton = CreateUpgradeCard(WidgetTree, TEXT("EmpoweredBolt"), TEXT("A\nDIAMOND"),
		TEXT("STARFIRE"), TEXT("+2 CARD DAMAGE"), TEXT("A deadlier hand of spell cards."),
		FLinearColor(0.17f, 0.02f, 0.04f), FLinearColor(1.f, 0.45f, 0.35f));
	EmpoweredBoltButton->OnClicked.AddDynamic(this, &UWizardOsAbilityWidget::SelectEmpoweredBolt);
	UHorizontalBoxSlot* BoltSlot = Cards->AddChildToHorizontalBox(EmpoweredBoltButton);
	BoltSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	BoltSlot->SetPadding(FMargin(8.f));

	VitalityButton = CreateUpgradeCard(WidgetTree, TEXT("Vitality"), TEXT("10\nCLUB"),
		TEXT("SECOND WIND"), TEXT("+5 HEALTH, +2 MAX"), TEXT("Stay in the game a little longer."),
		FLinearColor(0.01f, 0.13f, 0.1f), FLinearColor(0.4f, 1.f, 0.6f));
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
