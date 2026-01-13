// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AShooterPlayerController::AShooterPlayerController() :
    bGameEnded{false}
{

}

void AShooterPlayerController::GameHasEnded(class AActor* EndGameFocus, bool bIsWinner)
{
    Super::GameHasEnded(EndGameFocus,bIsWinner);
    
    bGameEnded = true;
    HUDOverlay->RemoveFromParent();

    if (bIsWinner)
    {
        if (WinningScreenWidgetClass)
        {
            UUserWidget* WinningWidget = CreateWidget<UUserWidget>(GetWorld(), WinningScreenWidgetClass);
            if (WinningWidget)
            {
                WinningWidget->AddToViewport();
            }
        }

        if (WinningSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), WinningSound);
        }

        AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
        if (ShooterCharacter)
        {
            ShooterCharacter->CharacterWon();
        }
    }
    else
    {
        if (LosingScreenWidgetClass)
        {
            UUserWidget* LosingWidget = CreateWidget<UUserWidget>(GetWorld(), LosingScreenWidgetClass);
            if (LosingWidget)
            {
                LosingWidget->AddToViewport();
            }
        }

        if (GameOverSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), GameOverSound);
        }
    }

}

void AShooterPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (HUDOverlayClass)
    {
        HUDOverlay = CreateWidget<UUserWidget>(this,HUDOverlayClass);
        if (HUDOverlay)
        {
            HUDOverlay->AddToViewport();
            HUDOverlay->SetVisibility(ESlateVisibility::Visible);
        }
    }
}