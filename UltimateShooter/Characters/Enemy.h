// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UltimateShooter/Interfaces/BulletHitInterface.h"
#include "UltimateShooter/Enums/HitDirection.h"
#include "Enemy.generated.h"


UCLASS()
class ULTIMATESHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	/**
	 * @brief Default constructor. Sets initial values for this enemie’s properties.
	 */
	AEnemy();

protected:
	/**
	 * @brief Called when the game starts or the character is spawned.
	 * 
	 * Used for initialization, binding inputs, setting up references, and other runtime setups.
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief Declared as Blueprint Native Event and has it's implementation in blueprint.
	 * 
	 * Sets Health Bar widget visibility to Visible.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void ShowHealthBar();

	/**
	 * @brief C++ side of the ShowHealthBar event implementation, called automaticlly when ShowHealthBar is called.
	 * 
	 * Sets the timer for the HideHealthBar function call.
	 * 
	 * @see ShowHealthBar()
	 * @see HideHealthBar()
	 */
	void ShowHealthBar_Implementation();
	
	/**
	 * @brief Declared as BlueprintImplementableEvent, implementation is done in the Blueprint.
	 * 
	 * Sets the Health Bar widget visibility to Hidden.
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void HideHealthBar();

	/**
	 * @brief Handles the enemy's death logic, including animation, effects, cleanup, and notifications.
	 * 
	 * This function is called when the enemy's health reaches zero. It prevents duplicate death logic using `bDying`,
	 * notifies the game mode about the kill, plays death animations depending on enemy type and orientation relative to the killer,
	 * and triggers effects for special cases like headshots.
	 * 
	 * Also disables weapons, hides the health bar, and marks the AI as dead in the blackboard, stopping its movement.
	 * 
	 * @param Character The player character responsible for the kill.
	 * @param Headshot Whether the kill was a headshot (affects death effects for specific enemy types).
	 * 
	 * @see AUltimateShooterGameModeBase::CharacterKilled()
	 * @see SpawnWeaponAndAmmo()
	 * @see DeactivateLeftWeapon()
	 * @see DeactivateRightWeapon()
	 */
	void Die(class AShooterCharacter* Character, bool Headshot = false);

	/**
	 * @brief Plays HitReactMontage and disables playing it for the HitReactTime amount (random (HitReactTimeMin, HitReactTimeMax)).
	 * 
	 * @param Section which montage section to jump to
	 * @param PlayRate montage play rate
	 * 
	 * @see ResetHitReactTimer()
	 */
	void PlayHitMontage(FName Section, float PlayRate = 1.0f);

	/**
	 * @brief Gets the section name for the direction of the bullet hit.
	 * 
	 * Performs a DotProduct of self forward and bullet Impact Direction vector to determine if it is in front or back section name
	 * and Z component from CrossProduct of the self forward  and bullet Impact Direction vector to determine if it is left or right
	 * section name.
	 * 
	 * @param HitResult result from the succesfull Line Trace performed by ShooterCharacter
	 * 
	 * @return FName HitReact montage section name to jump to
	 */
	FName GetHitReactDirection(const FHitResult& HitResult);

	/**
	 * @brief Allows Hit Montage to be played
	 * 
	 */
	void ResetHitReactTimer();

	/**
	 * @brief Stores the widget and location to HitNumbers map and sets the timer for DestroyHitNumber function call after the
	 * HitNumberDestroyTime amount
	 * 
	 * @param HitNumber widget to store and destroy after
	 * @param Location location at which Hit Number widget is spawned
	 */
	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

	/**
	 * @brief Removes widget from HitNumbers map and removes it from its parent
	 * 
	 */
	UFUNCTION()
	void DestroyHitNumber(UUserWidget* HitNumber);

	/**
	 * @brief Projects the location from the Hit Numbers map to the World space and set the HitNumber widget position in Viewport
	 * 
	 */
	void UpdateHitNumbers();

	//! Called when something overlaps with the agro sphere
	/**
	 * @brief Triggered when another actor overlaps the enemy's aggro detection sphere.
	 * 
	 * If the overlapping actor is a player character, sets the blackboard "Target" key
	 * in the AI controller to start engaging the character.
	 * 
	 * @param OverlappedComponent The component on the enemy that was overlapped (AgroSphere).
	 * @param OtherActor The actor that entered the overlap area.
	 * @param OtherComp The specific component of the other actor involved in the overlap.
	 * @param OtherBodyIndex Index of the other body (if multiple).
	 * @param bFromSweep Whether the overlap was from a sweep movement.
	 * @param SweepResult Information about the sweep hit (if applicable).
	 * 
	 * @see UBlackboardComponent::SetValueAsObject()
	 */
	UFUNCTION()
	void AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * @brief Set the Blackboard key Stunned to parameter value
	 * 
	 * @param Stunned value set to blackboard key, true when stunned
	 */
	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);

	/**
	 * @brief Triggered when another actor overlaps the enemy's combat detection sphere.
	 * 
	 * If the overlapping actor is a player character, sets the blackboard "InAttackRange" key
	 * in the AI controller to start attacking the character.
	 * 
	 * @param OverlappedComponent The component on the enemy that was overlapped (AgroSphere).
	 * @param OtherActor The actor that entered the overlap area.
	 * @param OtherComp The specific component of the other actor involved in the overlap.
	 * @param OtherBodyIndex Index of the other body (if multiple).
	 * @param bFromSweep Whether the overlap was from a sweep movement.
	 * @param SweepResult Information about the sweep hit (if applicable).
	 * 
	 * @see UBlackboardComponent::SetValueAsObject()
	 */
	UFUNCTION()
	void CombatSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); 

	/**
	 * @brief Triggered when another actor overlaps the enemy's combat detection sphere.
	 * 
	 * If the overlapping actor is a player character, sets the blackboard "InAttackRange" key
	 * in the AI controller to stop attacking the character because he is out of range.
	 * 
	 * @param OverlappedComponent The component on the enemy that was overlapped (AgroSphere).
	 * @param OtherActor The actor that entered the overlap area.
	 * @param OtherComp The specific component of the other actor involved in the overlap.
	 * @param OtherBodyIndex Index of the other body (if multiple).
	 * 
	 * @see UBlackboardComponent::SetValueAsObject()
	 */	
	UFUNCTION()
	void CombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/**
	 * @brief Plays the AttackMontage, jumps to Section and disables attacking for AttackWaitTime amount.
	 * 
	 * @param Section section name to jump to
	 * @param PlayRate montage play rate
	 */
	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate = 1.0f);

	/**
	 * @brief Based on the Enemy Type it will choose random section name for the AttackMontage
	 * 
	 */
	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	/**
	 * @brief Called when the enemy's left weapon collider overlaps with another actor.
	 * 
	 * If the overlapped actor is the player character, calculates the hit direction,
	 * applies damage, spawns blood effects, and potentially stuns the character
	 * based on the attack and impact direction.
	 * 
	 * @param OverlappedComponent The enemy's weapon collision component that registered the overlap.
	 * @param OtherActor The actor overlapped by the weapon (typically the player).
	 * @param OtherComp The specific component of the other actor involved in the overlap.
	 * @param OtherBodyIndex Index for the overlapping body (usually unused).
	 * @param bFromSweep Whether the overlap was due to a sweep movement.
	 * @param SweepResult Information about the sweep hit (if applicable).
	 * 
	 * @see DoDamage()
	 * @see GetCharacterDirection()
	 * @see SpawnBlood()
	 * @see StunCharacter()
	 */
	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
		
	/**
	 * @brief Called when the enemy's right weapon collider overlaps with another actor.
	 * 
	 * If the overlapped actor is the player character, calculates the hit direction,
	 * applies damage, spawns blood effects, and potentially stuns the character
	 * based on the attack and impact direction.
	 * 
	 * @param OverlappedComponent The enemy's weapon collision component that registered the overlap.
	 * @param OtherActor The actor overlapped by the weapon (typically the player).
	 * @param OtherComp The specific component of the other actor involved in the overlap.
	 * @param OtherBodyIndex Index for the overlapping body (usually unused).
	 * @param bFromSweep Whether the overlap was due to a sweep movement.
	 * @param SweepResult Information about the sweep hit (if applicable).
	 * 
	 * @see DoDamage()
	 * @see GetCharacterDirection()
	 * @see SpawnBlood()
	 * @see StunCharacter()
	 */
	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * @brief Enables collision on the LeftWeaponCollision 
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void ActivateLeftWeapon();

	/**
	 * @brief Disables collision on LeftWeaponCollision 
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void DeactivateLeftWeapon();
	
	/**
	 * @brief Enables collision on the RightWeaponCollision 
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void ActivateRightWeapon();

	/**
	 * @brief Disables collision on the RightWeaponCollision 
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void DeactivateRightWeapon();

	/**
	 * @brief Applys damage to Victim aka ShooterCharacter and plays Mele Impact Sound
	 * 
	 * @param Victim 
	 */
	void DoDamage(AShooterCharacter* Victim);

	/**
	 * @brief Based on the HitDirection and based on which weapon overlapped spawns Blood Particles at Victim's correct location
	 * 
	 * @param Victim character onto which to spawn blood
	 * @param Direction enemies direction to the character
	 * @param LeftWeapon true if left weapon is responsible for damage
	 * @param MontageDirection if stun appears tells character in which direction to plays HitReactMontage
	 */
	void SpawnBlood(AShooterCharacter* Victim, EHitDirection Direction, bool LeftWeapon, EHitDirection& MontageDirection);

	/**
	 * @brief Determines the direction from which the player character is facing the enemy.
	 * 
	 * Calculates the vector from the character to the enemy, then uses dot products
	 * with the character's forward and right vectors to determine whether the enemy
	 * is in front, behind, to the left, or to the right of the character.
	 * 
	 * @param Character The player character to compare orientation with.
	 * @param Direction (Out) The resulting relative direction from the character's perspective.
	 *                  Set to Front, Back, Left, Right, or None if Character is null.
	 */
	void GetCharacterDirection(AShooterCharacter* Character, EHitDirection& Direction);

	//! Attempt to stun character
	/**
	 * @brief Attepmts to stun character
	 * 
	 * @param Character character to stun
	 * @param Direction direction in which character should play HitReactMontage
	 */
	void StunCharacter(AShooterCharacter* Character, EHitDirection Direction);

	/**
	 * @brief Enables performing attack task
	 * 
	 */
	void ResetCanAttack();

	/**
	 * @brief Destroys this character
	 * 
	 */
	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	/**
	 * @brief Pauses anims
	 * 
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void FreezeInDeathPose();

	/**
	 * @brief Spawns a weapon and ammo at the enemy's location upon death.
	 * 
	 * Based on a randomized drop chance (`LootDropRate`), this function spawns a weapon of class `WeaponClass`
	 * and randomly decides between two types of ammo (`AmmoSMGClass` or `AmmoARClass`).
	 * Both weapon and ammo are spawned slightly above the enemy's current location and are thrown outward
	 * using physics to simulate a loot drop.
	 * 
	 * @note Weapon spawning is conditional and depends on `LootDropRate`, while ammo always spawns.
	 * 
	 * @see AWeapon::SetUpSpawnedWeapon()
	 * @see AWeapon::ThrowWeapon()
	 * @see AAmmo::ThrowAmmo()
	 */
	void SpawnWeaponAndAmmo();

private:

	//! Particles to spawn when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ImpactParticles;
	
	//! Sound to play when hit by bullets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* ImpactSound;

	//! Current Health of the enemy
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Health;
	
	//! Maximum Health of the enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float MaxHealth;
	
	//! Name of the Head bone
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FString HeadBone;
	
	//! Time to display health bar once shot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HealthBarDisplayTime;
	
	FTimerHandle HealthBarTimer;
	
	//! Montage containing hit and death animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HitMontage;
	
	FTimerHandle HitReactTimer;
	
	bool bCanHitReact;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMin;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitReactTimeMax;
	
	//! Map to store HitNumber widgets and their loocations
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitNumbers;
	
	//! Time before a HitNumber is removed from the screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float HitNumberDestroyTime;
	
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;
	
	//! Point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;
	
	//! Second point for the enemy to move to
	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;
	
	class AEnemyController* EnemyController;
	
	//! Overlap sphere for when the enemy becomes hostile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AgroSphere;
	
	//! True when playing get hit animation
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bStunned;
	
	//! Chance of being stunned; 0: no stun chance 1: 100% stun chance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float StunChance;
	
	//! True when in Attack Range; Time to Attack.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bInAttackRange;
	
	//! Sphere for attack range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USphereComponent* CombatRangeSphere;
	
	//! Montage containing different attacks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AttackMontage;
	
	//! AttackMontage section names
	FName AttackLFast;
	FName AttackRFast;
	FName AttackL;
	FName AttackR;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName AttackCFast;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName AttackC;
	
	//! Collision Volume for the left weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* LeftWeaponCollision;
	
	//! Collision Volume for the right weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* RightWeaponCollision;
	
	//! Damage amount dealt to character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float BaseDamage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket;
	
	//! True when Enemy can attack
	UPROPERTY(VisibleAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool bCanAttack;
	
	FTimerHandle AttackWaitTimer;
	
	//! Minimum wait time between attacks
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float AttackWaitTime;
	
	//! Death Anim Montage for the Enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* DeathMontage;
	
	bool bDying;
	
	//! Direction to play DeathMontage to
	EHitDirection DeathDirection;
	
	//! Which type of enemy for the purpose of knowing which montage sections to play and etc.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	FName EnemyType;
	
	/** Tells if last bullet hit was headshot */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool IsLastHeadshot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* MinionDeathParticles;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundCue* HeadshotExplosionSound;
	
	/** Weapon class to spawn upon enemy death. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AWeapon> WeaponClass;
	
	/** Ammo class for SMG-type weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AAmmo> AmmoSMGClass;

	/** Ammo class for AR-type weapons. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AAmmo> AmmoARClass;
	
	/** Probability (0–1) that the enemy will drop a weapon on death. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float LootDropRate;

public:	
	//! Called every frame
	/**
	 * @brief Called every frame.
	 * 
	 * Handles per-frame updates such as updateing hit numbers.
	 * 
	 * @param DeltaTime The time elapsed since the last frame.
	 * 
	 * @see UpdateHitNumbers()
	 */
	virtual void Tick(float DeltaTime) override;

	//! Called to bind functionality to input
	/**
	 * @brief Binds input actions and axes to character functions.
	 * 
	 * Called automatically by the engine to configure input bindings such as movement, looking, jumping, firing, etc.
	 * 
	 * @param PlayerInputComponent The input component to bind controls to.
	 */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	 * @brief Handles logic when the enemy is hit by a bullet.
	 * 
	 * Spawns impact particles and sound at the hit location, and applies a chance-based stun effect.
	 * If the stun chance succeeds and the enemy can currently react, it plays the hit reaction montage
	 * and sets the enemy's stunned state. Overriden function from IBulletHitInterface.
	 * 
	 * @param HitResult The result of the bullet hit, including location and impact data.
	 * @param Shooter The actor that fired the bullet.
	 * @param ShooterController The controller responsible for the shooting actor.
	 * 
	 * @see PlayHitMontage()
	 * @see GetHitReactDirection()
	 * @see SetStunned()
	 */
	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, class AController* ShooterController) override;

	//! Take combat damage
	/**
	 * @brief Applies incoming damage to the character and handles death if health reaches zero.
	 * 
	 * Overrides the base `TakeDamage` function to reduce health, trigger death logic, and inform Blackboard component about DamageCauser (Target).
	 * 
	 * @param DamageAmount The amount of damage to apply.
	 * @param DamageEvent The event data associated with the damage (e.g., type, hit info).
	 * @param EventInstigator The controller responsible for causing the damage.
	 * @param DamageCauser The actor that directly caused the damage (e.g., projectile or enemy).
	 * 
	 * @return The final amount of damage applied.
	 * 
	 * @see Die(class AShooterCharacter* Character, bool Headshot = false)
	 * @see AController
	 * @see AEnemyController
	 */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/**
	 * @brief Event implemented in the Blueprint, responsible for showing the Hit Number widget
	 * 
	 * @param Damage damage amount to show
	 * @param HitLocation location at which widget should be shown
	 * @param HeadShot determines the color of the widget (true = yellow, false = white)
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(int32 Damage, FVector HitLocation, bool HeadShot);

	FORCEINLINE FString GetHeadBone() const { return HeadBone; }

	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetDying() const { return bDying; }
	
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	FORCEINLINE bool IsDead() const { return bDying; }
};
