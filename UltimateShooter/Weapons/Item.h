// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Pickup UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),

	EIS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),

	EIT_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
};


UCLASS()
class ULTIMATESHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	/**
	 * @brief Constructor for AItem, initializes all components and default values.
	 */
	AItem();

protected:
	// Called when the game starts or when spawned
	/**
	 * @brief Called when the game starts or the item is spawned.
	 * Initializes UI, binds overlap events, sets item properties and pulse effect.
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief Called when another actor begins overlap with AreaSphere.
	 * Increments the overlapped item count on the character.
	 * 
	 * @param OverlappedComponent The component being overlapped.
	 * @param OtherActor The actor that overlapped.
	 * @param OtherComp The specific component of the actor that overlapped.
	 * @param OtherBodyIndex Index of the body that was involved.
	 * @param bFromSweep Whether it was a sweep.
	 * @param SweepResult The result of the sweep.
	 */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * @brief Called when another actor ends overlap with AreaSphere.
	 * Decrements overlapped item count and unhighlights inventory slot.
	 */
	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//! Sets the ActiveStars array of bools based on rarirty
	/**
	 * @brief Sets the ActiveStars array based on the current item rarity.
	 */
	void SetActiveStars();

	/**
	 * @brief Applies mesh and collision settings based on item state.
	 * 
	 * @param State The new state to apply.
	 */
	//! Sets the properties of the item's components based on State
	virtual void SetItemProperties(EItemState State);
	
	//! Called when ItemInterpTimer is finished
	/**
	 * @brief Called when interpolation ends; resets state and notifies character.
	 */
	void FinishInterping();

	//! Handles item interpolation when in the EquipInterping state
	/**
	 * @brief Handles item movement and rotation interpolation when picked up.
	 * 
	 * @param DeltaTime Time since last frame.
	 */
	void ItemInterp(float DeltaTime);

	//! Get interp location based on the item type
	/**
	 * @brief Returns the interpolated location for the item based on its type.
	 * 
	 * @return FVector Location to interpolate to.
	 */	
	FVector GetInterpLocation();

	/**
	 * @brief Plays the pickup sound for the item.
	 * 
	 * @param bForcePlaySound Whether to play sound regardless of conditions.
	 */
	void PlayPickupSound(bool bForcePlaySound = false);

	/**
	 * @brief Initializes custom depth rendering by disabling it.
	 */
	virtual void InitializeCustomDepth();

	/**
	 * @brief Called in the editor to initialize data and update material.
	 * 
	 * @param Transform Current actor transform in the scene.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 * @brief Enables the glow material effect by adjusting material parameters.
	 */
	void EnableGlowMaterial();

	/**
	 * @brief Resets the pulse timer to loop the pulse effect.
	 */
	void ResetPulseTimer();

	/**
	 * @brief Starts the pulse timer which periodically refreshes visual effects.
	 */
	void StartPulseTimer();

	/**
	 * @brief Updates the dynamic material parameters based on the pulse curves.
	 */
	void UpdatePulse();

	/**
	 * @brief Loads data from Item Rarity Data Table and updates visuals accordingly.
	 */
	void SetRarityParameters();
	
public:	
	// Called every frame
	/**
	 * @brief Called every frame to handle interpolation and pulse animation.
	 * 
	 * @param DeltaTime Time passed since last frame.
	 */
	virtual void Tick(float DeltaTime) override;

private:
	//! Skeletal Mesh for the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;
	
	//! Line trace collides with box to show HUD widgets
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	//! Popup widget for when the player looks at the item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;
	
	//!	Enables item tracing when overlapped
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere; 

	//! The name which appears on the PickupWidget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	//! Item count (ammo,etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	//! Item rarity determines number of stars in pickup widget
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	//! State of the item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;

	//! Starting Location when interping begins
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;
	//! Target interp location in front of the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;
	//! True when interping
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	bool bInterping;
	
	//! Plays when we start interping
	FTimerHandle ItemInterpTimer;
	//! Pointer to the character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* Character;

	//! Initial Yaw offset between the camera and interping item
	float InterpInitialYawOffset;

	//! Curve used to scale the item when interping 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;

	//! Enum for the type of item this item is
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocIndex;

	//! Index for the material we would like to change in runtime
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	//! Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;

	//! Material Instance used with the dynamic material instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;

	//! Curve to drive the dynamic material parameters
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	class UCurveVector* PulseCurve;
	
	//! Curve to drive the dynamic material parameters while interping
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpPulseCurve;

	FTimerHandle PulseTimer;

	//! Time for the PulseTimer
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FersnelExponent;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FersnelReflectFraction;
	
	//! Icon for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;
	
	//! Ammo Icon for this item in the inventory
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoItem;

	//! Slot in the inventory array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;
	
	//! True when character's inventory is full
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;
	
	//! Item rarity data table
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	class UDataTable* ItemRarityDataTable;
	
	//! Color in the Glow Material
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;
	
	//! LightColor in the pickup widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;
	
	//! DarkColor in the pickup widget
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;
	
	//! Number of stars in the pickup widget 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;
	
	//! Background icon for the inventory 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE UBoxComponent* GetCollisionBox () const { return CollisionBox; }
	FORCEINLINE EItemState GetItemState () const { return ItemState; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound; }
	FORCEINLINE void SetPickupSound(USoundCue* Sound) { PickupSound = Sound; }
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound; }
	FORCEINLINE void SetEquipSound(USoundCue* Sound) { EquipSound = Sound; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE void SetCharacter(AShooterCharacter* Char) { Character = Char; }
	FORCEINLINE void SetCharacterInventoryFull(bool bFull) { bCharacterInventoryFull = bFull; }
	FORCEINLINE void SetItemName(FString Name) { ItemName = Name; }
	//! Set item icon for the inventory
	FORCEINLINE void SetIconItem(UTexture2D* Icon) { IconItem = Icon; }
	//! Set ammo icon for the pickup widget
	FORCEINLINE void SetAmmoItem(UTexture2D* Icon) { AmmoItem = Icon; }
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) { MaterialInstance = Instance; }
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const { return MaterialInstance; }
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance) { DynamicMaterialInstance = Instance; }
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const { return DynamicMaterialInstance; }
	FORCEINLINE FLinearColor GetGlowColor() const { return GlowColor; }
	FORCEINLINE int32 GetMaterialIndex() const { return MaterialIndex; }
	FORCEINLINE void SetMaterialIndex(int32 Index) { MaterialIndex = Index; }
	FORCEINLINE EItemRarity GetItemRarity() const { return ItemRarity; }
	FORCEINLINE void SetItemRarity(EItemRarity Rarity) { ItemRarity = Rarity; SetRarityParameters(); }
	FORCEINLINE void SetItemCount(int Amount) { ItemCount = Amount; }
	FORCEINLINE void SetItemType(EItemType newType) { ItemType = newType; }
 	
	/**
	 * @brief Sets the item state and updates its properties accordingly.
	 * 
	 * @param NewState The new item state.
	 */
	void SetItemState(EItemState NewState);

	//! Called from AShooterCharacter class
	/**
	 * @brief Starts the interpolation process when item is picked up.
	 * 
	 * @param newCharacter The character picking up the item.
	 * @param bForcePlaySound Whether to force playing pickup sound.
	 */
	void StartItemCurve(AShooterCharacter* newCharacter, bool bForcePlaySound = false);

	//! Called in AShooterCharacter GetPickupItem
	/**
	 * @brief Plays the equip sound for the item.
	 * 
	 * @param bForcePlaySound Whether to play sound regardless of conditions.
	 */
	void PlayEquipSound(bool bForcePlaySound = false);

	/**
	 * @brief Enables custom depth rendering for the item.
	 */
	virtual void EnableCustomDepth();

	/**
	 * @brief Disables custom depth rendering for the item.
	 */
	virtual void DisableCustomDepth(); 

	/**
	 * @brief Disables the glow material effect by adjusting material parameters.
	 */
	void DisableGlowMaterial();

};

