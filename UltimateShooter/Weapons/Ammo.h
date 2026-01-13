// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "UltimateShooter/Enums/AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()

public:
	/** 
	 * Default constructor for the Ammo actor.
	 * Initializes components, sets default ammo type and mass, and configures the collision sphere.
	 */
	AAmmo();

	/** 
	 * Called every frame.
	 * Keeps the ammo mesh upright during the falling state.
	 */
	virtual void Tick(float DeltaTime) override;

private:

	//! Mesh for the ammo pickup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* AmmoMesh;

	//! AmmoType for the ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	//! The texture for the Ammo Icon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoIconTexture;

	//! Overlap Sphere for picking up the Ammo
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AmmoCollisionSphere;

	bool bFalling;

	float ThrowAmmoTime;

	FTimerHandle ThrowAmmoTimer;

protected:

	/** 
	 * Called when the game starts or when the actor is spawned.
	 * Binds the overlap event for the collision sphere.
	 */
	virtual void BeginPlay() override;

	/** 
	 * Configures the ammo mesh and collision settings based on the current item state.
	 * 
	 * @param State The current state of the item (Pickup, Equipped, Falling, EquipInterping).
	 */
	virtual void SetItemProperties(EItemState State) override;

	/**
	 * Callback for when another actor overlaps with the AmmoCollisionSphere.
	 * If the overlapping actor is the player character, initiates pickup logic.
	 * 
	 * @param OverlappedComponent The component that was overlapped.
	 * @param OtherActor The actor that initiated the overlap.
	 * @param OtherComp The component of the other actor.
	 * @param OtherBodyIndex Index of the body in the other component.
	 * @param bFromSweep Whether the overlap was from a sweep.
	 * @param SweepResult Hit result data.
	 */
	UFUNCTION()
	void AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 
	 * Called after the throw timer ends.
	 * Stops the falling state and enables the pickup pulse effect.
	 */
	void StopFalling();
	
public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	/** 
	 * Enables custom depth rendering on the ammo mesh for outline effects.
	 */
	virtual void EnableCustomDepth() override;

	/** 
	 * Disables custom depth rendering on the ammo mesh.
	 */
	virtual void DisableCustomDepth() override;

	/** 
	 * Simulates throwing the ammo item into the world.
	 * Applies an impulse to the mesh and sets its state to Falling.
	 */
	void ThrowAmmo();
};
