
// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "UltimateShooter/Enums/AmmoType.h"
#include "Engine/DataTable.h"
#include "UltimateShooter/Enums/WeaponType.h"
#include "Weapon.generated.h"

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WeaponAmmo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundCue* PickupSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* AmmoIcon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsMiddle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsTop;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsBottom;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsLeft;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsRight;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFireRate;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UParticleSystem* MuzzleFlash;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* FireSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomatic;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotDamage;
};

/**
 * 
 */
UCLASS()
class ULTIMATESHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	/**
	 * @brief Constructor for the AWeapon class.
	 * 
	 * Initializes default values for the weapon, such as throw time, ammo, magazine capacity,
	 * weapon type, ammo type, and other specific attributes. It also creates subobjects 
	 * for accessories like the suppressor (ARSuppressor) and red dot sight (ARRedDot).
	 */
	AWeapon();

	/**
	 * @brief Called every frame.
	 * 
	 * Keeps the weapon upright while it is in the EIS_Falling state.
	 * Also, continuously calls UpdateSlideDisplacement to update the slide animation.
	 * 
	 * @param DeltaTime The time elapsed since the previous frame.
	 */
	virtual void Tick(float DeltaTime) override;

protected:

	/**
	 * @brief Stops the weapon's falling state.
	 * 
	 * Sets the item state to 'Pickup' (ready to be picked up), sets bFalling to 'false', and starts
	 * the pulse timer for the glow effect. This is called by a timer after the weapon is thrown.
	 */
	void StopFalling();

	/**
	 * @brief Called on construction or when the actor's transform changes in the editor.
	 * 
	 * Initializes all weapon parameters by calling SetWeaponParameters() and sets the damage based on
	 * rarity by calling SetWeaponDamage(). This allows changes to be visible in real-time in the editor.
	 * 
	 * @param Transform The actor's transform.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 * @brief Called when the game starts or when the object is spawned.
	 * 
	 * Inherits base functionality from Super::BeginPlay(). If a bone name to hide (BoneToHide) is defined,
	 * that bone on the weapon's Skeletal Mesh will be hidden. It also calls the function to set up accessories.
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief Finishes the slide movement animation.
	 * 
	 * Sets the boolean variable bMovingSlide to 'false'. This is typically called by a timer
	 * after the animation duration has passed.
	 */
	void FinishMovingSlide();

	/**
	 * @brief Updates the position of the weapon's slide during the firing animation.
	 * 
	 * If a SlideDisplacementCurve exists and the slide is currently moving (bMovingSlide is true),
	 * this function calculates and sets the new slide position based on elapsed time.
	 */
	void UpdateSlideDisplacement();

	/**
	 * @brief Sets the weapon's parameters based on data from a DataTable.
	 * 
	 * Loads the 'WeaponDataTable' and finds the corresponding row based on the current WeaponType.
	 * It then sets all relevant attributes from that row: ammo type, ammo count, sounds, mesh,
	 * icons, materials, animations, crosshairs, fire rate, and damage. It also creates and applies a dynamic
	 * material instance for a glow effect.
	 */
	void SetWeaponParameters();

	/**
	 * @brief Sets the damage values (base and headshot) based on the weapon's type and rarity.
	 * 
	 * Uses nested 'switch' and 'if-else' structures to define specific damage values
	 * for each combination of weapon type (Pistol, SMG, Assault Rifle) and item rarity (from Damaged to Legendary).
	 */
	void SetWeaponDamage();

	/**
	 * @brief Sets up and configures the weapon's accessories.
	 * 
	 * If the weapon is an assault rifle, it attaches the suppressor and red dot sight to their
	 * respective sockets on the weapon mesh, disabling their collision and ticking. If the 'Spawned' parameter is true,
	 * the accessories are made visible. For all other weapon types, the accessories are hidden.
	 * 
	 * @param Spawned A boolean indicating if the accessories should be shown immediately.
	 */
	void SetUpAccessories(bool Spawned = false);

private:

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

	//! Ammo count for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	int32 Ammo;

	//! Maximum ammo that weapon can hold
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity;

	//! The type of weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType;

	//! The type of ammo for this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;

	//! FName for the reload montage section
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection;

	//! True when moving the clip while reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName; 

	int32 PreviousMaterialIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsMiddle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsTop;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsBottom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsLeft;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UTexture2D* CrosshairsRight;
	
	//! The speed at which automatic fire happens
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	float AutoFireRate;
	
	//! Particle system spawned at the barrel socket
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* MuzzleFlash;
	
	//! Sound played when the weapon is fired
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	USoundCue* FireSound;
	
	//! Name of the bone to hide on the weapon mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	FName BoneToHide;
	
	//! Amount that the slide is pushed back during pistol fire
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	float SlideDisplacement;
	
	//! Curve for the slide displacement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* SlideDisplacementCurve;
	
	//! Timer handle for updating SlideDisplacement
	FTimerHandle SlideTimer;
	
	//! Time for displacing the slide during pistol fire
	float SlideDisplacementTime;
	
	//! True when moving the clip
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	bool bMovingSlide;
	
	//! Max distance for the slide on the pistol
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	float MaxSlideDisplacement;
	
	//! True for automatic gun fire
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bAutomatic;
	
	//! Amount of damage caused by a bullet
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float Damage;
	
	//! Amount of damage when the bullet hits the head
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	float HeadShotDamage;
	
	//! Static Mesh Suppressor for the Assault Rifle 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ARSuppressor;
	
	//! Static Mesh Red Dot for the AssaultRifle 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ARRedDot;

public:

	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagazineCapacity() const { return MagazineCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE FName GetReloadMontageSection() const { return ReloadMontageSection; }
	FORCEINLINE void SetReloadMontageSection(FName Section) { ReloadMontageSection = Section; } 
	FORCEINLINE FName GetClipBoneName() const { return ClipBoneName; }
	FORCEINLINE void SetClipBoneName(FName Name) { ClipBoneName = Name; }
	FORCEINLINE float GetAutoFireRate() const { return AutoFireRate; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return MuzzleFlash; }
	FORCEINLINE USoundCue* GetFireSound() const { return FireSound; }
	FORCEINLINE bool GetAutomatic() const { return bAutomatic; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE void SetMovingClip(bool Move) { bMovingClip = Move; }

	//! Adds an impulse to the weapon
	/**
	 * @brief Initiates throwing the weapon.
	 * 
	 * Sets the weapon's state to falling (bFalling), corrects its rotation, and then applies
	 * an impulse to throw it in a specific direction with some random rotation. The impulse magnitude
	 * depends on the weapon type. It starts a timer that will stop the falling state after a set time.
	 * It also enables the glow material.
	 */
	void ThrowWeapon();
	
	//! Called from character class when firing weapon
	/**
	 * @brief Decrements the ammo count in the magazine by one.
	 * 
	 * If the ammo count is greater than 1, it decrements it. If it is 1 or less, it sets the ammo count to 0.
	 */
	void DecrementAmmo();
	
	/**
	 * @brief Starts the timer for the slide animation.
	 * 
	 * Sets bMovingSlide to 'true' and starts a timer that will call FinishMovingSlide
	 * after the duration defined in SlideDisplacementTime.
	 */
	void StartSlideTimer();

	/**
	 * @brief Reloads ammunition into the magazine.
	 * 
	 * Adds the specified amount of ammo (Amount) to the current ammo count (Ammo). It includes a check (checkf)
	 * to ensure the total ammo does not exceed the magazine capacity.
	 * 
	 * @param Amount The amount of ammunition to add.
	 */
	void ReloadAmmo(int32 Amount);

	/**
	 * @brief Checks if the magazine is full.
	 * 
	 * @return Returns 'true' if the ammo count is greater than or equal to the magazine capacity, otherwise returns 'false'.
	 */
	bool ClipIsFull();

	/**
	 * @brief Configures a randomly generated weapon that spawns in the world.
	 * 
	 * Randomly determines the weapon type (AssaultRifle, SubmachineGun, Pistol) and its rarity
	 * (Common, Uncommon, Rare, Legendary) based on defined probabilities. Afterward, it calls
	 * functions to set parameters, damage, and accessories according to the generated values.
	 */
	void SetUpSpawnedWeapon();

	/**
	 * @brief Hides the accessories (suppressor and red dot sight) on the assault rifle.
	 * 
	 * If the weapon type is an Assault Rifle, this function sets the visibility
	 * of the ARSuppressor and ARRedDot components to hidden.
	 */
	void HideAccessories();

	/**
	 * @brief Shows the accessories (suppressor and red dot sight) on the assault rifle.
	 * 
	 * If the weapon type is an Assault Rifle, this function sets the visibility
	 * of the ARSuppressor and ARRedDot components to visible.
	 */
	void ShowAccessories();
};
