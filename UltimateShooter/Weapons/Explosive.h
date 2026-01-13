// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UltimateShooter/Interfaces/BulletHitInterface.h"
#include "Explosive.generated.h"

UCLASS()
class ULTIMATESHOOTER_API AExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	/**
	 * @brief Default constructor. Sets initial values for this Explosive's properties.
	 */
	AExplosive();

protected:
	/**
	 * @brief Called when the game starts or the actor is spawned.
	 */
	virtual void BeginPlay() override;

private:
	//! Explosion when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ExplosiveParticles;

	//! Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ExplosiveMesh;
	
	//! Used to determine which Actors overlap during explosion
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* OverlapSphere;
	
	//! Default Amount of damage that will be dealt to Actors 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Damage;

public:	
	/**
	 * @brief Called every frame
	 * 
	 * @param DeltaTime The time elapsed since the last frame.
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Spawns explosive particles, impact sound, and applies damage to all actors that overlapped with OverlapSphere
	 * 
	 * @param HitResult The result of the bullet hit, including location and impact data.
	 * @param Shooter The actor that fired the bullet.
	 * @param ShooterController The controller responsible for the shooting actor.
	 */
	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override; 
};
