// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	/**
	 * @brief Construct a new AEnemyController object
	 * 
	 * Creates Blackboard and Behavior Tree components
	 */
	AEnemyController();

	/**
	 * @brief Called when the AI controller takes control of a pawn.
	 * 
	 * Casts the possessed pawn to an AEnemy and initializes the blackboard
	 * using the enemy's assigned behavior tree, if available.
	 * 
	 * @param InPawn The pawn that was just possessed by this controller.
	 * 
	 * @see AAIController::OnPossess()
	 * @see UBehaviorTree
	 * @see UBlackboardComponent
	 */
	virtual void OnPossess(APawn* InPawn) override;

private:
	//! Blackboard component for this enemy
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBlackboardComponent* BlackboardComponent;
	
	//! Behavior tree component for this enemy
	UPROPERTY(BlueprintReadWrite, Category = "AI Behavior", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTreeComponent* BehaviorTreeComponent;

public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }

	/**
	 * @brief Checks if possesed Enemy is dead
	 * 
	 * @return true if enemy is dead
	 * @return false if enemy is alive
	 */
	bool IsDead();
};
