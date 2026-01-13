// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UltimateShooter/Interfaces/BulletHitInterface.h"
#include "BreakableWall.generated.h"

UCLASS()
class ULTIMATESHOOTER_API ABreakableWall : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	/**
	 * @brief Default constructor. Sets initial values for this BreakableWall's properties.
	 */
	ABreakableWall();

protected:
	/**
	 * @brief Called when the game starts or the actor is spawned.
	 */
	virtual void BeginPlay() override;

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;

	//! Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

public:	
	/**
	 * @brief Called every frame
	 * 
	 * @param DeltaTime The time elapsed since the last frame.
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Handles logic when the wall is hit by a bullet.
	 * 
	 * Spawns impact particles and sound at the hit location, and destoys the wall
	 * 
	 * @param HitResult The result of the bullet hit, including location and impact data.
	 * @param Shooter The actor that fired the bullet.
	 * @param ShooterController The controller responsible for the shooting actor.
	 */
	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;
};
