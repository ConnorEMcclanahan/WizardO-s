// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsMerchantWidget.h"

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
#include "Fonts/CompositeFont.h"
#include "Misc/Paths.h"
#include "WizardOsCharacter.h"

namespace
{
const TSharedPtr<const FCompositeFont>& GetWizardOsFont()
{
	static const TSharedPtr<const FCompositeFont> Font = MakeShared<FCompositeFont>(
		TEXT("Regular"), FPaths::ProjectDir() / TEXT("Fonts/UnifrakturCook-Bold.ttf"),
		EFontHinting::Default, EFontLoadingPolicy::LazyLoad);
	return Font;
}

UTextBlock* CreateText(UWidgetTree* WidgetTree, const FName Name, const FString& Text, const FLinearColor& Color, int32 FontSize)
{
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Label->SetText(FText::FromString(Text));
	Label->SetColorAndOpacity(FSlateColor(Color));
	Label->SetFont(FSlateFontInfo(GetWizardOsFont(), FontSize, TEXT("Regular")));
	Label->SetJustification(ETextJustify::Center);
	return Label;
}

UButton* CreateItemCard(UWidgetTree* WidgetTree, const FName Name, const FString& Title,
	const FString& Effect, int32 Cost, const FLinearColor& Color)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor::Transparent);

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%sSize"), *Name.ToString()));
	SizeBox->SetWidthOverride(220.f);
	SizeBox->SetHeightOverride(250.f);

	UBorder* Frame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%sFrame"), *Name.ToString()));
	Frame->SetBrushColor(FLinearColor(0.92f, 0.68f, 0.2f));
	Frame->SetPadding(FMargin(4.f));
	UBorder* Card = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), *FString::Printf(TEXT("%sCard"), *Name.ToString()));
	Card->SetBrushColor(Color);
	Card->SetPadding(FMargin(14.f));
	UVerticalBox* Contents = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("%sContents"), *Name.ToString()));
	Contents->AddChildToVerticalBox(CreateText(WidgetTree, *FString::Printf(TEXT("%sTitle"), *Name.ToString()), Title, FLinearColor(1.f, 0.85f, 0.45f), 21));
	Contents->AddChildToVerticalBox(WidgetTree->ConstructWidget<USpacer>(USpacer::StaticClass(), *FString::Printf(TEXT("%sSpace"), *Name.ToString())))
		->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	Contents->AddChildToVerticalBox(CreateText(WidgetTree, *FString::Printf(TEXT("%sEffect"), *Name.ToString()), Effect, FLinearColor::White, 15));
	Contents->AddChildToVerticalBox(CreateText(WidgetTree, *FString::Printf(TEXT("%sCost"), *Name.ToString()),
		FString::Printf(TEXT("COST: %d CHIPS"), Cost), FLinearColor(0.45f, 1.f, 0.65f), 16));
	Card->SetContent(Contents);
	Frame->SetContent(Card);
	SizeBox->SetContent(Frame);
	Button->SetContent(SizeBox);
	return Button;
}
}

void UWizardOsMerchantWidget::Configure(AWizardOsCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
	UpdateAvailability();
}

void UWizardOsMerchantWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree->RootWidget)
	{
		ChipLabel = Cast<UTextBlock>(GetWidgetFromName(TEXT("ChipLabel")));
		ItemButtons.SetNum(3);
		ItemButtons[0] = Cast<UButton>(GetWidgetFromName(TEXT("ChainSpark")));
		ItemButtons[1] = Cast<UButton>(GetWidgetFromName(TEXT("Runebomb")));
		ItemButtons[2] = Cast<UButton>(GetWidgetFromName(TEXT("TwinCast")));
		UButton* LeaveButton = Cast<UButton>(GetWidgetFromName(TEXT("LeaveMerchant")));
		if (ItemButtons[0] && ItemButtons[1] && ItemButtons[2] && LeaveButton)
		{
			ItemButtons[0]->OnClicked.AddUniqueDynamic(this, &UWizardOsMerchantWidget::BuyChainSpark);
			ItemButtons[1]->OnClicked.AddUniqueDynamic(this, &UWizardOsMerchantWidget::BuyRunebomb);
			ItemButtons[2]->OnClicked.AddUniqueDynamic(this, &UWizardOsMerchantWidget::BuyTwinCast);
			LeaveButton->OnClicked.AddUniqueDynamic(this, &UWizardOsMerchantWidget::LeaveShop);
			return;
		}

		WidgetTree->RootWidget = nullptr;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MerchantScreen"));
	WidgetTree->RootWidget = Root;

	UBorder* Scrim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MerchantScrim"));
	Scrim->SetBrushColor(FLinearColor(0.025f, 0.01f, 0.04f, 0.78f));
	UCanvasPanelSlot* ScrimSlot = Root->AddChildToCanvas(Scrim);
	ScrimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	ScrimSlot->SetOffsets(FMargin(0.f));

	USizeBox* PanelSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("MerchantPanelSize"));
	PanelSize->SetWidthOverride(850.f);
	PanelSize->SetHeightOverride(440.f);
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("MerchantPanel"));
	Panel->SetBrushColor(FLinearColor(0.08f, 0.025f, 0.11f, 0.98f));
	Panel->SetPadding(FMargin(28.f, 20.f));
	UVerticalBox* Contents = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MerchantContents"));
	Panel->SetContent(Contents);
	PanelSize->SetContent(Panel);
	UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(PanelSize);
	PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));

	Contents->AddChildToVerticalBox(CreateText(WidgetTree, TEXT("MerchantTitle"),
		TEXT("THE TRAVELING MERCHANT IS VISITING"), FLinearColor(1.f, 0.8f, 0.3f), 29));
	Contents->AddChildToVerticalBox(CreateText(WidgetTree, TEXT("MerchantSubtitle"),
		TEXT("Spend your pocket chips on one special item."), FLinearColor(0.8f, 0.7f, 0.9f), 15));
	ChipLabel = CreateText(WidgetTree, TEXT("ChipLabel"), TEXT("POCKET CHIPS: 0"), FLinearColor(0.4f, 1.f, 0.62f), 18);
	Contents->AddChildToVerticalBox(ChipLabel);

	UHorizontalBox* Items = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("MerchantItems"));
	Contents->AddChildToVerticalBox(Items);

	UButton* ChainSpark = CreateItemCard(WidgetTree, TEXT("ChainSpark"), TEXT("CHAIN SPARK"), TEXT("BOLTS JUMP TO 2 NEARBY ENEMIES"), 5, FLinearColor(0.1f, 0.05f, 0.16f));
	ChainSpark->OnClicked.AddDynamic(this, &UWizardOsMerchantWidget::BuyChainSpark);
	Items->AddChildToHorizontalBox(ChainSpark)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ItemButtons.Add(ChainSpark);

	UButton* Runebomb = CreateItemCard(WidgetTree, TEXT("Runebomb"), TEXT("RUNEBOMB"), TEXT("25% CHANCE TO EXPLODE ON HIT"), 7, FLinearColor(0.04f, 0.11f, 0.16f));
	Runebomb->OnClicked.AddDynamic(this, &UWizardOsMerchantWidget::BuyRunebomb);
	Items->AddChildToHorizontalBox(Runebomb)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ItemButtons.Add(Runebomb);

	UButton* TwinCast = CreateItemCard(WidgetTree, TEXT("TwinCast"), TEXT("TWIN CAST"), TEXT("30% CHANCE TO FIRE A SECOND BOLT"), 6, FLinearColor(0.15f, 0.04f, 0.06f));
	TwinCast->OnClicked.AddDynamic(this, &UWizardOsMerchantWidget::BuyTwinCast);
	Items->AddChildToHorizontalBox(TwinCast)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	ItemButtons.Add(TwinCast);

	UButton* LeaveButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("LeaveMerchant"));
	LeaveButton->SetBackgroundColor(FLinearColor(0.2f, 0.12f, 0.26f));
	LeaveButton->SetContent(CreateText(WidgetTree, TEXT("LeaveMerchantText"), TEXT("LEAVE SHOP"), FLinearColor::White, 16));
	LeaveButton->OnClicked.AddDynamic(this, &UWizardOsMerchantWidget::LeaveShop);
	Contents->AddChildToVerticalBox(LeaveButton)->SetPadding(FMargin(250.f, 10.f, 250.f, 0.f));
}

void UWizardOsMerchantWidget::UpdateAvailability()
{
	if (!OwnerCharacter)
	{
		return;
	}

	const int32 Chips = OwnerCharacter->GetPocketChips();
	if (ChipLabel)
	{
		ChipLabel->SetText(FText::FromString(FString::Printf(TEXT("POCKET CHIPS: %d"), Chips)));
	}
	static constexpr int32 ItemCosts[] = { 5, 7, 6 };
	for (int32 Index = 0; Index < ItemButtons.Num(); ++Index)
	{
		if (ItemButtons[Index])
		{
			ItemButtons[Index]->SetIsEnabled(Chips >= ItemCosts[Index] && !OwnerCharacter->IsMerchantItemOwned(Index));
		}
	}
}

void UWizardOsMerchantWidget::BuyChainSpark()
{
	BuyItem(0);
}

void UWizardOsMerchantWidget::BuyRunebomb()
{
	BuyItem(1);
}

void UWizardOsMerchantWidget::BuyTwinCast()
{
	BuyItem(2);
}

void UWizardOsMerchantWidget::BuyItem(int32 ItemIndex)
{
	if (OwnerCharacter)
	{
		OwnerCharacter->BuyMerchantItem(ItemIndex);
	}
}

void UWizardOsMerchantWidget::LeaveShop()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->LeaveMerchant();
	}
}
