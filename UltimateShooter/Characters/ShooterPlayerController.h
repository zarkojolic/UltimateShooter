// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	/**
	 * @brief Construct a new AShooterPlayerController object
	 * 
	 */
	AShooterPlayerController();

	/**
	 * @brief Handles end-of-game logic for the player controller.
	 * 
	 * Called when the game has ended, either in victory or defeat. Removes the in-game HUD,
	 * displays the appropriate end screen widget, plays sound effects, and notifies the player's character
	 * if they won.
	 * 
	 * @param EndGameFocus Optional actor to focus the camera on when the game ends (usually unused here).
	 * @param bIsWinner Whether the player won the match.
	 * 
	 * @see AShooterCharacter::CharacterWon()
	 */
	virtual void GameHasEnded(class AActor* EndGameFocus = nullptr, bool bIsWinner = false) override;

protected:

	/**
	 * @brief Called when the game starts or the character is spawned.
	 * 
	 * Used for initialization, binding inputs, setting up references, and other runtime setups.
	 * Created HUDOverlay widget and added it to viewport
	 */
	virtual void BeginPlay() override;

private:
	//! Refetence to the Overall HUD Overlay Blueprint Class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> HUDOverlayClass;
	
	//! Variable to hold the HUDOverlay widget after creating it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	UUserWidget* HUDOverlay;
	
	//! Winning Screen widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> WinningScreenWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> LosingScreenWidgetClass;
	
	bool bGameEnded;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	class USoundCue* WinningSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true"))
	USoundCue* GameOverSound;
	
public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetGameEnded() const { return bGameEnded; }
};
