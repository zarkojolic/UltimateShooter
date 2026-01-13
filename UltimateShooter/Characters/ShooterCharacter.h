// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UltimateShooter/Enums/AmmoType.h"
#include "UltimateShooter/Enums/HitDirection.h"
#include "ShooterCharacter.generated.h"

/**
 * @brief Represents the current combat state of the character.
 * 
 * Used to manage state transitions such as shooting, reloading, equipping, and being stunned.
 */
UENUM(BlueprintType)
enum class ECombatState: uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_FireTimerInProgress UMETA(DisplayName = "FireTimerInProgress"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_Equipping UMETA(DisplayName = "Equipping"),
	ECS_Stunned UMETA(DisplayName = "Stunned"),

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};

/**
 * @brief Represents a location in the world used for interpolating items (e.g. when picking them up).
 */
USTRUCT(BlueprintType)
struct FInterpLocation
{
	GENERATED_BODY()

	//! Scene component to use for its location for interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent;

	//! Number of items interping to/at this scene comp location
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 ItemCount;
};

/**
 * @brief Broadcasts when an item is equipped, passing current and new slot indices.
 * 
 * @param CurrentSlotIndex Index of the slot the item was previously in.
 * @param NewSlotIndex Index of the slot the item is now equipped in.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);

/**
 * @brief Broadcasts when a UI icon should be highlighted or animated.
 * 
 * @param SlotIndex Index of the inventory or quick slot icon.
 * @param bStartAnimation Whether to start (true) or stop (false) the animation.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHighlightIconDelegate, int32, SlotIndex, bool, bStartAnimation);

UCLASS()
class ULTIMATESHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	/**
	 * @brief Default constructor. Sets initial values for this character’s properties.
	 */
	AShooterCharacter();

protected:
	/**
	 * @brief Called when the game starts or the character is spawned.
	 * 
	 * Used for initialization, binding inputs, setting up references, and other runtime setups.
	 */
	virtual void BeginPlay() override;

	//! Called for forwards/backwards movement
	/**
	 * @brief Function responsible for moving Forward and Backwards
	 * 
	 * Called when the W or S key is pressed and based on that it will pass value
	 * after we determine which way is forward on the Controller we will add Movement Input in that course
	 * and in the direction based on the value.
	 * 
	 * @param Value between (-1,1) 
	 */
	void MoveForward(float Value);

	//! Called for side movement
	/**
	 * @brief Function responsible for moving Right and Left
	 * 
	 * Called when the A or D key is pressed and based on that it will pass value
	 * after we determine which way is Right on the Controller we will add Movement Input in that course
	 * and in the direction based on the value.
	 * 
	 * @param Value between (-1,1) 
	 */
	void MoveRight(float Value);

	//! Called via input to look up/down at a given rate. 
	//! @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	/**
	 * @brief Called via input to look up/down at a given rate.
	 * 
	 * @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	//! Called via input to turn at a given rate. 
	//! @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	/**
	 * @brief Called via input to turn at a given rate.
	 * 
	 * @param Rate This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	//! Rotate Controller based on mouse X movement
	//! @param Value -> The input value from mouse movement
	/**
	 * @brief Rotate Controller based on mouse X movement
	 * 
	 * @param Value -> The input value from mouse movement
	 */
	void Turn(float Value);

	//! Rotate Controller based on mouse Y movement
	//! @param Value -> The input value from mouse movement
	/**
	 * @brief Rotate Controller based on mouse Y movement
	 * 
	 * @param Value -> The input value from mouse movement
	 */
	void LookUp(float Value);


	//! Fire 1 Bullet
	/**
	 * @brief Handles firing logic for the currently equipped weapon
	  
	 * Checks if the player has a valid equipped weapon and is allowed to fire based on current combat state.
 	 * If firing is allowed and there is available ammo, the function plays firing animations and sounds,
 	 * spawns the bullet, decrements the weapon ammo, starts the crosshair fire effect, and handles timers
 	 * for continuous fire and pistol slide movement (if applicable).
	 * 
	 * @see PlayFireSound();
	 * @see PlayGunFireMontage();
	 * @see SendBullet();
	 * @see StartCrosshairBulletFire()
	 * @see StartFireTimer()
	 */
	void FireWeapon();

	/**
	 * @brief Get the Beam End Location object
	 * 
	 * After the Trace Under Crosshair function call which will return the result of the Line Trace from camera to crosshair
	 * we will perform the second line trace which will go from the Muzzle Socket Location to the End of the first line trace to
	 * make sure there are no objects in between the crosshair and muzzle socket that could block the bullet.
	 * 
	 * @param MuzzleSocketLocation Location of the Muzzle Socket that is placed on top of the Gun Barrel
	 * @param OutHitResult FHitResult output parameter that will be used in Send Bullet function
	 * @return true 
	 * @return false 
	 * 
	 * @see TraceUnderCrosshair(FHitResult& OutHitResult,FVector& OutHitLocation)
	 */
	bool GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult);

	//! Set bAiming and zoom camera FOV in and out
	/**
	 * @brief Called when the Aiming Button is Pressed
	 * 
	 * Function bound to the Aiming Button Action and will be called when the Right Mouse Button is pressed
	 * Setting the bAimButtonPressd to true and checks if we are not in Reloading or Stun state and if we 
	 * are not we will call Aim function.
	 */
	void AimingButtonPressed();

	/**
	 * @brief Called when the Aiming Button is Released
	 * 
	 * Function bound to the Aiming Button Action and will be called when the Right Mouse Button is released
	 * Setting the bAimButtonPressd to false and calls Stop Aiming function. 
	 */
	void AimingButtonReleased();

	/**
	 * @brief Reducing and Expanding Field of View
	 * 
	 * Function called in Tick. 
	 * Checks the bAiming state and if it's true it will reduce the Field of View or
	 * if it's false it will Expand it back to normal.
	 * 
	 * @param DeltaTime represents the time elapsed since the last frame was rendered (in seconds)
	 */
	void CameraZooming(float DeltaTime);

	//! Set Base Turn and LookUp rates based on aiming
	/**
	 * @brief Set's the Base LookUp and Turn rate's based on bAiming state.
	 */
	void SetLookRates();

	/**
	 * @brief Calculating the spread between the center and it's parts
	 * 
	 * Function called in Tick.
	 * Calculating how much the crosshair's parts will be spread based on the characters movement, if he is in the air, 
	 * if he is shooting or aiming. All of this parameters will have impact on Crosshair Spreading amount.
	 * 
	 * @param DeltaTime represents the time elapsed since the last frame was rendered (in seconds)
	 */
	void CalculateCrosshairSpread(float DeltaTime);

	/**
	 * @brief Lets Calculate Crosshair Spread know that Bullet Firing started
	 * 
	 * Sets the bFiringBullet to true, this will have impact on CalculateCrosshairSpread, and
	 * sets the timer to call the FinishCrosshairBulletFire function after ShootTimeDuration amount.
	 * 
	 * @see CalculateCrosshairSpread(float DeltaTime)
	 */
	void StartCrosshairBulletFire();

	/**
	 * @brief Lets CalculateCrosshairSpread know that Bullet Firing ended
	 * 
	 * Sets the bFiringBullet to false, this will have impact on CalculateCrosshairSpread.
	 * 
	 * @see CalculateCrosshairSpread(float DeltaTime)
	 */
	UFUNCTION()
	void FinishCrosshairBulletFire();

	/**
	 * @brief Sets the bFireButtonPressed to true and calls FireWeapon function.
	 * 
	 * Function bound to the Input Action "FireButton" and gets called from editor when LeftMouseButton is pressed.
	 * 
	 * @see FireWeapon()
	 */
	void FireButtonPressed();

	/**
	 * @brief  Sets the bFireButtonPressed to false
	 * 
	 * Function bound to the Input Action "FireButton" and gets called from editor when LeftMouseButton is released.
	 */
	void FireButtonReleased();

	/**
	 * @brief Starting the timer to prevent fire weapon from being called until AutoFireRate amount passes
	 * 
	 * Setting the CombatState to FireTimerInProgress and starting the timer to call the AutoFireReset function 
	 * after the AutoFireRate of the EquippedWeapon which will set the CombatState to Unoccupied.
	 * 
	 * @see AutoFireReset()
	 */
	void StartFireTimer();

	/**
	 * @brief Responsible for setting CombatState to Unoccupied and calling FireWeapon or ReloadWeapon based on condition 
	 * 
	 * If Combat State is Reloading or Stunned we will return, and if we are not in those states, we will set the CombatState to 
	 * Unoccupied and call WeaponHasAmmo function. If weapon doesn't have ammo, we will call ReloadWeapon function, but if it has ammo
	 * we will check if bFireButtonPressed and EquippedWeapon is Automatic, if both cases are true we will call FireWeapon
	 * to repeat the Automatic Fire cycle.
	 * 
	 * @see WeaponHasAmmo()
	 * @see FireWeapon()
	 * @see ReloadWeapon()  
	 */
	UFUNCTION()
	void AutoFireReset();

	//! Trace for items under crosshair
	/**
	 * @brief Performs a Line Trace through center of the screen where crosshair is positioned
	 * 
	 * To get the CrosshairLocation we will get the viewport size and divide it to get the center of the screen we will than call
	 * DeprojectScreenToWorld to project the CrosshairLocation into the world and if it is successfull we will perform a line trace 
	 * wich CrosshairWorldPosition into the CrosshairWorldDirection and set the both OutHitResult and OutHitLocation output parameters
	 * based on the result of the line trace.
	 * 
	 * @param OutHitResult Output parameter passed into the Line Trace and will contain the result of it
	 * @param OutHitLocation If Line trace is successfull it will have it's Location, if not it will have End passed into Line Trace
	 * @return true If DeprojectScreenToWorld is successfull and Line Trace has a Blocking Hit
	 * @return false If DeprojectScreenToWorld is not successfull
	 */
	bool TraceUnderCrosshair(FHitResult& OutHitResult,FVector& OutHitLocation);

	//! Trace for items if OverlappedItemCount > 0
	/**
	 * @brief If Items are nearby it will perform a line trace and display its widget and highlight the inventory slot
	 * 
	 * If bShouldTraceForItems is true than we will call Trace Under Crosshair function and if it is successfull we will use its output
	 * parameters to determine if we traced Item, if it is we will Display his widget and if it is also a weapon, 
	 * we will Highlight the Inventory slot and if it isnt we will Unhighlight it. Also if we traced multiple items in a row
	 * we will remember Traced Item Last Frame and Hide its Widget, we will also do it if bShouldTraceForItems is false.
	 * 
	 * @see TraceUnderCrosshair(FHitResult& OutHitResult,FVector& OutHitLocation)
	 * @see HighlightInventorySlot()
	 * @see UnHighlightInventorySlot()
	 */
	void TraceForItems();

	//! Spawns default weapon
	/**
	 * @brief If DefaultWeaponClass is set we will spawn weapon based on that class and return it
	 * 
	 * @return AWeapon* nullptr if DefaultWeaponClass is not set or newly created weapon based on Default Weapon Class
	 */
	class AWeapon* SpawnDefaultWeapon();

	//! Takes a weapon and attaches it to the mesh
	/**
	 * @brief Attaches WeaponToEquip to Skeletal Mesh hand socket and sets the EquippedWeapon to WeaponToEquip
	 * 
	 * If WeapontoEquip is valid we will attach it to Skeletal Meshes RightHandSocket and if we are not swapping we will
	 * Broadcast to EquipItemDelegate to display inventory slot animations and set the EquippedWeapon to WeaponToEquip and also
	 * set WeaponToEquip state to Equipped.
	 *  
	 * @param WeaponToEquip Weapon which will be Equipped
	 * @param bSwapping true if we are swapping, false if we are just equipping
	 */
	void EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false);

	//! Detach Weapon and let it fall to the ground
	/**
	 * @brief Detach EquippedWeapon and Throw it to fall on the ground
	 * 
	 * We will Detach EquippedWeapon from the Mesh and set it's state to Falling and call ThrowWeapon on it.
	 */
	void DropWeapon();
	
	/**
	 * @brief If we Traced for Items and TraceHitItem is valid we will call StartItemCurve on it
	 * 
	 * Function bound to the "Select" Input Action which gets called from editor when "E" key is pressed.
	 */
	void SelectButtonPressed();
	/// @brief Not Used
	void SelectButtonReleased();

	//! Drops currently eqiped weapon and Equips TraceHitItem
	/**
	 * @brief Replaces EquippedWeapon with WeaponToSwap in the Inventory and drops EquippedWeapon and Equips WeaponToSwap
	 * 
	 * @param WeaponToSwap Weapon to replace EquippedWeapon with
	 * 
	 * @see DropWeapon()
	 * @see EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false)
	 */
	void SwapWeapon(AWeapon* WeaponToSwap);

	//! Initialize the Ammo Map with ammo values
	/// @brief Adds 9mm and AR ammo types to our AmmoMap and gives them starting values
	void InitializeAmmoMap();
 
	//! Function for Firing bullet
	/// @brief Checks if EquippedWeapon ammo is greater than 0
	bool WeaponHasAmmo();

	/// @brief Checks if EquippedWeapon has Fire Sound, and if it has plays it
	void PlayFireSound();

	/**
	 * @brief Spawns Particles and calss BulletHit_Implementation on hit actor and applyes damage to it
	 * 
	 * Spawns Muzzle Flash Particles, calls GetBeamEndLocation function and passes BarrelSocketTransform. If it is 
	 * succesfull we will get the hit actor, if that Actor Implements IBulletHitInterface calls BulletHit_Implementation on it,
	 * and if Actor is Enemy we will ApplyDamage. Also spawns Impact and Beam Particles at the end.
	 * 
	 * @see GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
	 */
	void SendBullet();

	/// @brief If HipFireMontage is set we will Play it
	void PlayGunFireMontage();

	//! Bound to the R key and Gamepad Face button left
	/**
	 * @brief Calls ReloadWeapon function
	 * 
	 * Function bound to the "ReloadButton" Input Action and gets called when "R" key is pressed.
	 *   
	 * @see ReloadWeapon()
	 */
	void ReloadButtonPressed();

	//! Handle Reloading the weapon
	/**
	 * @brief Checks if we have ammo for EquippedWeapon AmmoType, if we do, we will update it and play the ReloadMontage
	 * 
	 * If CombatState is Unoccupied or FireTimerInProgress, calls CarryingAmmo function and chekcs if clip is not full
	 * on the Equipped Weapon, if they are true calls StopAiming if character is already aiming, set CombatState to Reloading and 
	 * play ReloadMontage.
	 * 
	 * @see CarryingAmmo()
	 * @see StopAiming()
	 */
	void ReloadWeapon();

	//! Checks to see if we have ammo of the EquippedWeapon's ammo type
	/**
	 * @brief Gets the EquippedWeapon AmmoType and checks the value in the AmmoMap for that AmmoType
	 * 
	 * @return true if value in AmmoMap is greater than 0
	 * @return false if value in AmmoMap is less than or equal to 0
	 */
	bool CarryingAmmo();

	//! Called from animation blueprint with grab clip notify
	UFUNCTION(BlueprintCallable)
	/**
	 * @brief Finds the EquippedWeapon Clip bone, and attaches it to the Hand_L bone
	 * 
	 * Called from animation blueprint with grab clip notify. Gets the Clip Bone name from EquippedWeapon and finds it's index
	 * from the Mesh, gets clip transform and attaches it to the Hand_L in it's transform, and sets 
	 * Moving Clip boolean to true on EquippedWeapon.
	 */
	void GrabClip();

	//! Called from animation blueprint with release clip notify
	/**
	 * @brief Sets Mobing Clip to false on EquippedWeapon
	 * 
	 * Called from animation blueprint with release clip notify
	 */
	UFUNCTION(BlueprintCallable)
	void ReleaseClip();

	/**
	 * @brief Check if character is not in the Air and performs Crouch or Stand Up action based on the previous state
	 * 
	 * Sets the bCrouching to negation of it and sets the Walking speed base on the current state we are in
	 */
	void CrouchButtonPressed();

	/**
	 * @brief Cheks if we are crouching, and if we are we will stand up and if we are not we will jump
	 * 
	 */
	virtual void Jump() override;

	//! Interp capsule half height when crouching/standing
	/**
	 * @brief Interp capsule half height when crouching/standing
	 * 
	 * Function called from Tick.
	 * If character is crouching it lowers the capsule height, and  if character is standing it makes capsule height go back to normal.
	 * It's done with FInterpTo function to smoothen the transition, and adjusts the mesh local offset by the delta capsule half height.
	 * 
	 * @param DeltaTime represents the time elapsed since the last frame was rendered (in seconds)
	 */
	void InterpCapsuleHalfHeight(float DeltaTime);

	/**
	 * @brief Sets the bAiming to true and reduces the Walk Speed
	 * 
	 */
	void Aim();

	/**
	 * @brief Sets the bAiming to false and checks if we are not crouching and sets Walk Speed back to normal
	 * 
	 */
	void StopAiming();

	/**
	 * @brief Adds the Ammo amount to our Map and if Equipped Weapon is empty we will Reload it 
	 * 
	 * @param Ammo Ammo to pickup
	 * 
	 * @see ReloadWeapon()
	 */
	void PickupAmmo(class AAmmo* Ammo);

	/**
	 * @brief Called in BeginPlay to add Interp Location to which weapons and ammo will fly when picked up
	 * 
	 */
	void InitializeInterpLocations();

	/**
	 * @brief Calls ExchangeInventoryItems function to swaps the current weapon in the hands with the weapon on index 0
	 * 
	 * Function bound to the "FKey" Input Action and called when "F" key is pressed
	 * 
	 * @see ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
	 */
	void FKeyPressed();

	/**
	 * @brief Calls ExchangeInventoryItems function to swaps the current weapon in the hands with the weapon on index 1
	 * 
	 * Function bound to the "1Key" Input Action and called when "1" key is pressed
	 * 
	 * @see ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
	 */
	void OneKeyPressed();

	/**
	 * @brief Calls ExchangeInventoryItems function to swaps the current weapon in the hands with the weapon on index 2
	 * 
	 * Function bound to the "2Key" Input Action and called when "2" key is pressed
	 * 
	 * @see ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
	 */
	void TwoKeyPressed();

	/**
	 * @brief Calls ExchangeInventoryItems function to swaps the current weapon in the hands with the weapon on index 3
	 * 
	 * Function bound to the "3Key" Input Action and called when "3" key is pressed
	 * 
	 * @see ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
	 */
	void ThreeKeyPressed();

	/**
	 * @brief Calls ExchangeInventoryItems function to swaps the current weapon in the hands with the weapon on index 4
	 * 
	 * Function bound to the "4Key" Input Action and called when "4" key is pressed
	 * 
	 * @see ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
	 */
	void FourKeyPressed();

	/**
	 * @brief Calls ExchangeInventoryItems function to swaps the current weapon in the hands with the weapon on index 5
	 * 
	 * Function bound to the "5Key" Input Action and called when "5" key is pressed
	 * 
	 * @see ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
	 */
	void FiveKeyPressed();

	/**
	 * @brief Replaces the EuippedWeapon with weapon on the NewItemIndex from the inventory and sets their states accordingly
	 * 
	 * if CurrentItemIndex and NewItemIndex are different and NewItemIndex is less than inventory size and
	 * CombatState is Unoccupied or Equipping we will check if we are aiming and if we are we will call StopAiming and
	 * set the state of EquippedWeapon to PcikedUp to hide it, and Equip the weapon on NewItemIndex from the inventory
	 * and set its state to Equipped to make it visible, set CombatState to Equipping and Play EquipMontage
	 * 
	 * @param CurrentItemIndex Index of the Weapon that is in the hands
	 * @param NewItemIndex Index of the Weapon that will be in the hands
	 * 
	 * @see StopAiming()
	 * @see EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping = false)
	 */
	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);

	/**
	 * @brief Gets the first empty inventory slot
	 * 
	 * @return int32 index of the empty slot in the inventory or -1 if inventory is full
	 */
	int32 GetEmptyInventorySlot();

	/**
	 * @brief Gets the Empty Inventory Slot and Broadcast it to HighlightIconDelegate to play inventory animation in blueprint
	 * 
	 * @see GetEmptyInventorySlot()
	 */
	void HighlightInventorySlot();

	/**
	 * @brief Called from the editor, determines the surface type under the Character
	 * 
	 * Performs a Line Trace under the Character location and passes Physical Material 
	 * from Hit Result into DetermineSurfaceType, and returns the result of it.
	 * 
	 * @return EPhysicalSurface Physical surface type returned into the editor
	 */
	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

	/**
	 * @brief Called with anim notify to Enables the Input after Game Start Animation is played
	 * 
	 */
	UFUNCTION()
	void GameStartAnimationFinished();

	/**
	 * @brief Called from the editor, sets CombatState to Unoccupied and calls Aim or ReloadWeapon if needed
	 * 
	 * @see Aim()
	 * @see ReloadWeapon()
	 */
	UFUNCTION(BlueprintCallable)
	void EndStun();
	
	/**
	 * @brief Indicates to Game Mode that Character is Killed, Plays DeathMontage, Disables Input
	 * 
	 */
	void Die();
	
	/**
	 * @brief Called from the Editor, simply Pauses Anims on the Mesh
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void FinishDeath();
	
	/**
	 * @brief Displays StunnedWidget and sets timer to Remove it
	 * 
	 */
	void ShowStunnedWidget();
	
	/**
	 * @brief Removes Stuned Widget From the Parent
	 * 
	 */
	void RemoveStunnedWidget();

	/**
	 * @brief Called from the Editor, simply Pauses Anims on the Mesh
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void FinishWinning();

public:	

	//! Called every frame
	/**
	 * @brief Called every frame.
	 * 
	 * Handles per-frame updates such as movement logic, timers, aiming adjustments, and other gameplay-related behavior.
	 * 
	 * @param DeltaTime The time elapsed since the last frame.
	 */
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	/**
	 * @brief Binds input actions and axes to character functions.
	 * 
	 * Called automatically by the engine to configure input bindings such as movement, looking, jumping, firing, etc.
	 * 
	 * @param PlayerInputComponent The input component to bind controls to.
	 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//! Take combat damage
	/**
	 * @brief Applies incoming damage to the character and handles death if health reaches zero.
	 * 
	 * Overrides the base `TakeDamage` function to reduce health, trigger death logic, and inform AI controllers when this character dies.
	 * 
	 * @param DamageAmount The amount of damage to apply.
	 * @param DamageEvent The event data associated with the damage (e.g., type, hit info).
	 * @param EventInstigator The controller responsible for causing the damage.
	 * @param DamageCauser The actor that directly caused the damage (e.g., projectile or enemy).
	 * 
	 * @return The final amount of damage applied.
	 * 
	 * @see Die()
	 * @see AController
	 * @see AEnemyController
	 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
		class AController* EventInstigator, AActor* DamageCauser) override;

	/**
	 * @brief Adds the Amount to Health and makes sure to not exceed the MaxHealth 
	 * 
	 * @param Amount healing amount
	 */
	UFUNCTION(BlueprintCallable)
	void Heal(float Amount);

private:
	//! Camera Boom positioning the camera behind the character 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	//! Camera that follows the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	//! Base turn rate, in deg/sec. Other scaling may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseTurnRate;
	//! Base look up/down rate, in deg/sec. Other scaling may affect final turn rate
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float BaseLookUpRate;

	//! Turn Rate and LookUp Rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipLookUpRate;

	//! Turn Rate and LookUp Rate while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimingLookUpRate;

	//! Scale Factor for mouse look sensitivity. Turn rate and Look Up rate when not aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipLookUpRate;

	//! Scale Factor for mouse look sensitivity. Turn rate and Look Up rate while aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingTurnRate;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimingLookUpRate;

	//! Impact Effect upon hitting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	//! Shooting smoke trail
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticles;

	//! Fire Animation Montage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* HipFireMontage;

	//! True when Aiming
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	float CameraDefaultFOV;
	float CameraZoomedFOV;
	float CameraCurrentFOV;

	//! Interpolation speed for zooming when aiming
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomInterpSpeed;

	//! Determines the spread of the crosshair
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier;

	//! Velocity Component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairVelocityFactor;

	//! In air component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor;

	//! Aim component for crosshairs spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor;

	//! Shooting component for crosshairs spread 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootingFactor;

	float ShootTimeDuration;
	bool bFiringBullet;
	FTimerHandle CrosshairShootTimer;

	//! Left mouse button or right console trigger pressed
	bool bFireButtonPressed;

	//! True when we can fire. False when waiting for the timer
	bool bShouldFire;

	//! Sets a timer between gunshots
	FTimerHandle AutoFireTimer;

	//! True if we should trace every frame for items
	bool bShouldTraceForItems;

	//! Number of overlapped AItems
	int8 OverlappedItemCount;

	//! The AItem we hit last frame 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* TraceHitItemLastFrame;

	//! Currently equipped Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeaponClass;

	//! The Item currently hit by our trace in TraceForItems (could be null)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* TraceHitItem;

	//! Distance outward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	//! Distance upward from the camera for the interp destination
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	//! Map to keep track of ammo of the different ammo types
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	TMap<EAmmoType, int32> AmmoMap; 

	//! Starting amount of 9mm ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 Starting9mmAmmo;

	//! Starting amount of AR ammo
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Items, meta = (AllowPrivateAccess = "true"))
	int32 StartingARAmmo;

	//! Combat State -> Can only fire or reload if Unoccupied
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	ECombatState CombatState;

	//! Montage for reload animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ReloadMontage;
	
	//! Montage for Equip animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EquipMontage;

	//! Transform of the clip when we first grab the clip during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FTransform ClipTransform;

	//! Scene component to attach to the Character's hand during reloading
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* HandSceneComponent;

	//! True when crouching
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bCrouching;

	//! Regular movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseMovementSpeed;

	//! Crouch movement speed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchMovementSpeed;

	//! Half height of the capsule when not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float StandingCapsuleHalfHeight;

	//! Half height of the capsule when crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingCapsuleHalfHeight;

	//! Ground friction while not crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float BaseGroundFriction;

	//! Ground friciton while crouching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float CrouchingGroundFriction;

	//! Used for Aiming button pressed
	bool bAimButtonPressed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponInterpComp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp2;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp3;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp4;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* InterpComp6;

	//! Array of interp location structs
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TArray<FInterpLocation> InterpLocations;

	FTimerHandle PickupSoundTimer;
	FTimerHandle EquipSoundTimer;

	bool bShouldPlayPickupSound;
	bool bShouldPlayEquipSound;

	void ResetPickupSoundTimer();
	void ResetEquipSoundTimer();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float PickupSoundResetTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	float EquipSoundResetTime;

	//! An array of AItems for inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TArray<AItem*> Inventory;
	
	const int32 INVENTORY_CAPACITY{ 6 };
	
	//! Delegate for sending slot information to InventoryBar when equipping
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FEquipItemDelegate EquipItemDelegate;
	
	//! Delegate for sending slot information to InventoryBar when equipping
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
	FHighlightIconDelegate HighlightIconDelegate;
	
	//! The index for the currently highlighted slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 HighlightedSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float GameStartTime;
	
	FTimerHandle GameStartTimerHandle;
	
	bool GameStartAnimation;
	
	//! Character Health
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;
	//! Character Max Health
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;
	
	//! Sound made when Character gets hit by a mele attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* MeleImpactSound;
	
	//! Blood splatter particles for mele hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BloodParticles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName LeftBloodSocketName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightBloodSocketName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName ForwardBloodSocketName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName BackBloodSocketName;
	
	//! Hit React anim montage for when character is stunned
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitReactMontage;
	
	//! Chance of being stunned when hit by an enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;

	//! Death anim montage for when character Dies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;
	
	//! True when character dies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bDead;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> StunnedWidgetClass;
	
	UPROPERTY()
	UUserWidget* StunnedWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* WinningMontage;

	FTimerHandle StunnedWidgetTimer;
	float StunnedWidgetDuration;

	bool bGameEnded;
	
public:

	/**
	 * @brief Simple Getter called from editor
	 * 
	 * @return CrosshairSpreadMultiplier
	 */
	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	//! Add/Subtracts to/from OverlappedItemCount and updates bShouldTraceForItems
	/**
	 * @brief Called when character overlaps with Item's sphere to indicates if character should trace for items
	 * 
	 * @param Amount Add to OverlappedItemCount
	 */
	void IncrementOverlappedItemCount(int8 Amount);

	//! No Longer needed AItem has GetItemInterpLocation
	//FVector GetCameraInterpLocation();

	/**
	 * @brief Plays the Item's pickup sound and handles the case when Item is Weapon or Ammo
	 * 
	 * If Item is weapon it will be added to inventory or swapped with weapon in hands.
	 * If Item is Ammo it will just pickup.
	 * 
	 * @param Item Item to pickup
	 * 
	 * @see PickupAmmo(class AAmmo* Ammo)
	 */
	void GetPickupItem(AItem* Item);

	/**
	 * @brief Resets CombatState and handles the subtraction from the AmmoMap and adding to the Weapon Magazine
	 * 
	 * Checks if AimButtonPressed is true and calls Aim and if FireButtonPressed is true calls FireWeapon.
	 * Called from anim notify at the end of the reloading animation.
	 * 
	 * @see Aim()
	 * @see FireWeapon()
	 */
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/**
	 * @brief Resets Combat State and calls Aim if bAimingButtonPressed is true
	 * 
	 * Called from anim notify at the end of the equipping animation.
	 */
	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	/**
	 * @brief Gets Interp Location with least items currently interping to it
	 * 
	 * @param Index index of the interp location inside the InterpLocations array
	 * @return FInterpLocation interp location with least items
	 */
	FInterpLocation GetInterpLocation(int32 Index);

	//! Returns the index in interplocations array with the lowest item count
	/**
	 * @brief Returns the index in interplocations array with the lowest item count
	 * 
	 * @return int32 index in interplocations array
	 */
	int32 GetInterpLocationIndex();

	/**
	 * @brief Adds the Amount to the ItemCount of the InterpLocation at the InterpLocations arrays Index
	 * 
	 * @param Index index inside the InterpLocations array
	 * @param Amount amount that will be added to InterpLocations ItemCount
	 */
	void IncrementInterpLocItemCount(int32 Index, int32 Amount);

	/**
	 * @brief Starts the timer to reduce the number of times Pikcup Sound is played
	 * 
	 */
	void StartPickupSoundTimer();

	/**
	 * @brief Starts the timer to reduce the number of times Equip Sound is played
	 * 
	 */
	void StartEquipSoundTimer();
	
	/**
	 * @brief Broadcasts to HighlightIconDelegate to unhighlight the HighlightedSlot
	 * 
	 */
	void UnHighlightInventorySlot();

	/**
	 * @brief Plays HitReactMontage in the direction based on the input parameter
	 * 
	 * @param Direction direction in which to play HitReactMontage
	 */
	void Stun(EHitDirection Direction);

	/**
	 * @brief Indicates that game ended and plays WinningMontage and disables input
	 * 
	 */
	void CharacterWon();

	//! Returns CameraBoom subobject
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	//! Returns FollowCamera
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	//! Character Aiming
	FORCEINLINE bool GetAiming() const { return bAiming; }
	
	FORCEINLINE int8 GetOverlappedItemCount() const { return OverlappedItemCount; }

	FORCEINLINE ECombatState GetCombatState() const { return CombatState; }

	FORCEINLINE bool GetCrouching() const { return bCrouching; }

	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	FORCEINLINE bool ShouldPlayPickupSound() const { return bShouldPlayPickupSound; }

	FORCEINLINE bool ShouldPlayEquipSound() const { return bShouldPlayEquipSound; }

	FORCEINLINE bool GetGameStartAnimation() const { return GameStartAnimation; }

	FORCEINLINE USoundCue* GetMeleImpactSound() const { return MeleImpactSound; }

	FORCEINLINE UParticleSystem* GetBloodParticles() const { return BloodParticles; }

	FORCEINLINE FName GetLeftBloodSocketName() const { return LeftBloodSocketName; }

	FORCEINLINE FName GetRightBloodSocketName() const { return RightBloodSocketName; }

	FORCEINLINE FName GetForwardBloodSocketName() const { return ForwardBloodSocketName; }

	FORCEINLINE FName GetBackBloodSocketName() const { return BackBloodSocketName; }
	
	FORCEINLINE float GetStunChance() const { return StunChance; }

	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

};
