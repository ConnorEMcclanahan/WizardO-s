// Copyright Epic Games, Inc. All Rights Reserved.

#include "WizardOsHomeWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Fonts/CompositeFont.h"
#include "Kismet/KismetSystemLibrary.h"
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

UTextBlock* CreateMenuText(UWidgetTree* WidgetTree, const FName Name, const FString& Text, int32 FontSize,
	const FLinearColor& Color)
{
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Label->SetText(FText::FromString(Text));
	Label->SetFont(FSlateFontInfo(GetWizardOsFont(), FontSize, TEXT("Regular")));
	Label->SetColorAndOpacity(FSlateColor(Color));
	Label->SetJustification(ETextJustify::Center);
	return Label;
}

UButton* CreateMenuButton(UWidgetTree* WidgetTree, const FName Name, const FString& Text)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor(0.13f, 0.06f, 0.22f));
	Button->SetContent(CreateMenuText(WidgetTree, *FString::Printf(TEXT("%sText"), *Name.ToString()), Text, 24,
		FLinearColor(1.f, 0.84f, 0.4f)));
	return Button;
}
}

void UWizardOsHomeWidget::Configure(AWizardOsCharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;
}

void UWizardOsHomeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (WidgetTree->RootWidget)
	{
		UButton* PlayButton = Cast<UButton>(GetWidgetFromName(TEXT("PlayButton")));
		UButton* QuitButton = Cast<UButton>(GetWidgetFromName(TEXT("QuitButton")));
		if (PlayButton && QuitButton)
		{
			PlayButton->OnClicked.AddUniqueDynamic(this, &UWizardOsHomeWidget::StartGame);
			QuitButton->OnClicked.AddUniqueDynamic(this, &UWizardOsHomeWidget::QuitGame);
			return;
		}

		WidgetTree->RootWidget = nullptr;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("HomeScreen"));
	WidgetTree->RootWidget = Root;
	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
	Background->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.06f, 0.9f));
	UCanvasPanelSlot* BackgroundSlot = Root->AddChildToCanvas(Background);
	BackgroundSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	BackgroundSlot->SetOffsets(FMargin(0.f));

	UVerticalBox* Menu = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Menu"));
	UCanvasPanelSlot* MenuSlot = Root->AddChildToCanvas(Menu);
	MenuSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	MenuSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	MenuSlot->SetSize(FVector2D(420.f, 340.f));

	Menu->AddChildToVerticalBox(CreateMenuText(WidgetTree, TEXT("GameTitle"), TEXT("WIZARDOS"), 54,
		FLinearColor(1.f, 0.82f, 0.35f)));
	Menu->AddChildToVerticalBox(CreateMenuText(WidgetTree, TEXT("GameSubtitle"), TEXT("BUILD YOUR HAND. SURVIVE THE WAVES."), 16,
		FLinearColor(0.55f, 0.75f, 1.f)));

	UButton* PlayButton = CreateMenuButton(WidgetTree, TEXT("PlayButton"), TEXT("PLAY"));
	PlayButton->OnClicked.AddDynamic(this, &UWizardOsHomeWidget::StartGame);
	Menu->AddChildToVerticalBox(PlayButton)->SetPadding(FMargin(40.f, 55.f, 40.f, 10.f));

	UButton* QuitButton = CreateMenuButton(WidgetTree, TEXT("QuitButton"), TEXT("QUIT"));
	QuitButton->OnClicked.AddDynamic(this, &UWizardOsHomeWidget::QuitGame);
	Menu->AddChildToVerticalBox(QuitButton)->SetPadding(FMargin(40.f, 10.f));
}

void UWizardOsHomeWidget::StartGame()
{
	if (OwnerCharacter)
	{
		OwnerCharacter->StartGameFromHome();
	}
}

void UWizardOsHomeWidget::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
