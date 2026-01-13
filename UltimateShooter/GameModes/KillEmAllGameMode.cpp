// Fill out your copyright notice in the Description page of Project Settings.


#include "KillEmAllGameMode.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "UltimateShooter/Characters/EnemyController.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"

void AKillEmAllGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (BackgroundMusic)
    {
        MusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), BackgroundMusic, 1.f, 1.f, 0.f, nullptr, true);
        if (MusicComponent)
        {
            MusicComponent->bIsUISound = true; // Ako ne želiš da se priguši kada pauziraš igru
            MusicComponent->Play();
        }
    }
}

void AKillEmAllGameMode::CharacterKilled(ACharacter* Character)
{
    Super::CharacterKilled(Character);

    APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
    if (PlayerController)
    {
        EndGame(false);
    }

    for (AEnemyController* Controller : TActorRange<AEnemyController>(GetWorld()))
    {
        if (!Controller->IsDead())
        {
            return;
        }
    }

    EndGame(true);
}

void AKillEmAllGameMode::EndGame(bool bIsPlayerWinner)
{
    if (MusicComponent && MusicComponent->IsPlaying())
    {
        MusicComponent->Stop();
    }

    for (AController* Controller : TActorRange<AController>(GetWorld())) 
    {
        if (Controller->IsPlayerController())
        {
            Controller->GameHasEnded(Controller->GetPawn(),bIsPlayerWinner);
        }
        else
        {
            Controller->GameHasEnded(Controller->GetPawn(),!bIsPlayerWinner);
        }
    }
}