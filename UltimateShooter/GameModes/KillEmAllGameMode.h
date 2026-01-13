// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UltimateShooterGameModeBase.h"
#include "KillEmAllGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API AKillEmAllGameMode : public AUltimateShooterGameModeBase
{
	GENERATED_BODY()

private:
	/**
	 * @brief Ends the current match and notifies all controllers of the result.
	 * 
	 * Stops background music and calls `GameHasEnded` on all controllers, passing
	 * in whether the player has won or lost. AI and player controllers receive
	 * opposing values for the win condition.
	 * 
	 * @param bIsPlayerWinner Whether the player is considered the winner of the match.
	 * 
	 * @see AController::GameHasEnded()
	 */
	void EndGame(bool bIsPlayerWinner);

	UPROPERTY(EditAnywhere, Category = "Audio", meta = (AllowPritvateAccess = "true"));
	class USoundCue* BackgroundMusic;

	UPROPERTY()
	class UAudioComponent* MusicComponent;

protected:
	/**
	 * @brief Called when the game starts or the character is spawned.
	 * 
	 * Used for initializing playing the background music.
	 */
	virtual void BeginPlay() override;

public:
	/**
	 * @brief Called when any character dies during the game.
	 * 
	 * If the killed character is controlled by a player, the game ends in defeat.
	 * Otherwise, checks if all enemy AI are dead — if so, ends the game in victory.
	 * 
	 * @param Character The character that has been killed.
	 * 
	 * @see EndGame()
	 */
	virtual void CharacterKilled(ACharacter* Character);
};
