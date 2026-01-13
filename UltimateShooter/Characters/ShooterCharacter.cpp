// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "UltimateShooter/Weapons/Item.h"
#include "Components/WidgetComponent.h"
#include "UltimateShooter/Weapons/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h" 
#include "UltimateShooter/Weapons/Ammo.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UltimateShooter/UltimateShooter.h"
#include "UltimateShooter/Interfaces/BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "UltimateShooter/GameModes/UltimateShooterGameModeBase.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	//? Base rates for Turning/LookingUp
	BaseTurnRate{20.f}, BaseLookUpRate{20.f},
	//? Turn rates for Aiming/Not Aiming
	HipTurnRate{50.f}, HipLookUpRate{50.f}, AimingTurnRate{15.f}, AimingLookUpRate{15.f},
	//? MouseLook sensitivity Scale factors
	MouseHipTurnRate{0.25f}, MouseHipLookUpRate{0.25f}, MouseAimingTurnRate{0.2f}, MouseAimingLookUpRate{0.2f},
	//? Camera field of view values
	bAiming{false}, CameraDefaultFOV{0.f}, CameraZoomedFOV{25.f}, CameraCurrentFOV{0.f}, ZoomInterpSpeed{30.f},
	//? Crosshair factors
	CrosshairSpreadMultiplier{0.f} ,CrosshairInAirFactor{0.f}, CrosshairAimFactor{0.f}, CrosshairShootingFactor{0.f},
	//? Bullet fire timer variable
	ShootTimeDuration{0.05f}, bFiringBullet{false},
	//? Automatic fire variables
	bFireButtonPressed{false}, bShouldFire{true},
	//? Item trace variables
	bShouldTraceForItems{false}, TraceHitItemLastFrame{NULL},
	//? CameraInterpLocation variables
	CameraInterpDistance{250.f}, CameraInterpElevation{65.f},
	//? Ammo 
	Starting9mmAmmo{15}, StartingARAmmo{30},
	//? Combat variables
	CombatState{ECombatState::ECS_Unoccupied}, bCrouching{false}, BaseMovementSpeed{650.f}, CrouchMovementSpeed{300.f},
	StandingCapsuleHalfHeight{88.f}, CrouchingCapsuleHalfHeight{44.f}, BaseGroundFriction{8.f}, CrouchingGroundFriction{100.f},
	//? Aiming variables
	bAimButtonPressed{false},
	//? Pickup Sound properties
	bShouldPlayPickupSound{true}, bShouldPlayEquipSound{true}, PickupSoundResetTime{0.15f}, EquipSoundResetTime{0.15f},
	//? Icon animation property
	HighlightedSlot{-1},
	//? Game Start Animation
	GameStartTime{5.f}, GameStartAnimation{true},
	//? Health
	Health{100.f}, MaxHealth{100.f},
	// //? Blood Splatter Socket Names
	LeftBloodSocketName{TEXT("LeftArmBloodSocket")}, RightBloodSocketName{TEXT("RightArmBloodSocket")},
	ForwardBloodSocketName{TEXT("ForwardBloodSocket")}, BackBloodSocketName{TEXT("BackBloodSocket")},
	//? Stun variables
	StunChance{0.25f},
	//? Dead
	bDead{false}, 
	//? Stun
	StunnedWidget{nullptr},  StunnedWidgetDuration{1.5f},
	//? Game Ending
	bGameEnded{false}

{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//! Create a Camera Boom (pulls in towards the character if there is collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 200.f; //! The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //! Rotate the arm based on the Controller
	CameraBoom->SocketOffset = FVector(0.f,40.f,80.f); //! Setting camera offset

	//! Create a Follow Camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom); //! Attach it to spring arm

	//! Don't rotate when the Controller rotates. Let controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = true;
	bUseControllerRotationYaw = false;

	//! Configure Character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; //! Character Moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.f,540.f,0.f); //! ... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.5f;

	//! Create Hand Scene Component
	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("HandSceneComp"));

	//! Create Interpolation Components
	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Component"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interpolation Component 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
}

//! This changes the shooting from the point and direction of gun barrel, 
//! To a point and direction of crosshair, the rest stays the same .

//! If we don't have the desired behaviour that we specified in c++, we need to...
//TODO: Check the settings on ShooterCharacter in Blueprint Use Controller Rotation Yaw
//TODO: Check the settings on Character Movement Orient in Blueprint Rotation To Movement

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if(FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	
	//! Spawn the default weapon and Equip it
	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);

	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	//! Create FInterpLocation structs for each interp location. Add to array 
	InitializeInterpLocations();

	GetWorldTimerManager().SetTimer(GameStartTimerHandle, this, &AShooterCharacter::GameStartAnimationFinished, GameStartTime);

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        DisableInput(PlayerController);
    }
}

void AShooterCharacter::MoveForward(float Value)
{
	//! Find out which way is forward (Forward of the Controller (Camera) not of the Actor)
	if((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0};

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction,Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	//! Find out which way is right
	if((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0};

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction,Value);
	}
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float ScaleFactor{0.f};
	if (bAiming)
	{
		ScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		ScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * ScaleFactor);
}

void AShooterCharacter::LookUp(float Value)
{
	float ScaleFactor{0.f};
	if (bAiming)
	{
		ScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		ScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * ScaleFactor);
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		PlayGunFireMontage();
		SendBullet();
		EquippedWeapon->DecrementAmmo();
		//! Start bullet fire timer for crosshairs
		StartCrosshairBulletFire();

		if (!bGameEnded)
		{
			StartFireTimer();
		}

		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			//! Start moving slide timer
			EquippedWeapon->StartSlideTimer();
		}
	}
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FVector OutBeamLocation;

	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshair(CrosshairHitResult,OutBeamLocation);

	if (bCrosshairHit)
	{
		//! Tenative beam location - still need to trace from gun 
		OutBeamLocation = CrosshairHitResult.Location;
	}
	//! OutBeam Location is End if we didnt hit anything in TraceUnderCrosshair 

	//TODO Weapon Barrel Line Trace
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd { MuzzleSocketLocation + StartToEnd * 1.25f };

	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if(!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading && CombatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimButtonPressed = false;
	StopAiming();
}

//! Smoothening the Camera Zoom by Interpolating
void AShooterCharacter::CameraZooming(float DeltaTime)
{
	if(bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV,CameraZoomedFOV,DeltaTime,ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV,CameraDefaultFOV,DeltaTime,ZoomInterpSpeed);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if(bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 0.45f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;
	//! Mapping velocity value to range {0,1}
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,VelocityMultiplierRange,Velocity.Size());

	if (GetCharacterMovement()->IsFalling()) //! in the air?
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor,0.6f,DeltaTime,10.f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor,0.f,DeltaTime,30.f);
	}

	if (bAiming) //! are we aiming?
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor,-0.6f,DeltaTime,30.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor,0.f,DeltaTime,30.f);
	}

	
	if(bFiringBullet) //! True till 0.05 sec after firing
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor,0.25f,DeltaTime,45.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor,0.f,DeltaTime,45.f);
	}

	CrosshairSpreadMultiplier = 0.2 + CrosshairVelocityFactor + CrosshairInAirFactor 
		+ CrosshairAimFactor + CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer,this,&AShooterCharacter::FinishCrosshairBulletFire,ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if (EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_FireTimerInProgress;
	
	GetWorldTimerManager().SetTimer(AutoFireTimer,this,&AShooterCharacter::AutoFireReset,EquippedWeapon->GetAutoFireRate());
}

void AShooterCharacter::AutoFireReset()
{
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Stunned) return;
	
	CombatState = ECombatState::ECS_Unoccupied;
	if (EquippedWeapon == nullptr) return;
	
	if (WeaponHasAmmo())
	{
		if (bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}
	else
	{
		//! Reload Weapon
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshair(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	//! Get current size of the viewport
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) 
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	//! Get screen space location of crosshair
	FVector2D CrosshairLocation(ViewportSize.X / 2.f,ViewportSize.Y / 2.f);
	// CrosshairLocation.Y -= 25.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	//! Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld( UGameplayStatics::GetPlayerController(this,0), 
		CrosshairLocation,CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		//! Trace from crosshair world location outward
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;

		GetWorld()->LineTraceSingleByChannel(OutHitResult,Start,End,ECollisionChannel::ECC_Visibility);
		if(OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (bShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		bool result = TraceUnderCrosshair(ItemTraceResult,HitLocation);
		if(result)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
			const AWeapon* TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			if (TraceHitWeapon)
			{
				if (HighlightedSlot == -1)
				{
					//! Not currently highlighting slot; highlight it
					HighlightInventorySlot();
				}
			}
			else
			{
				//! Is a slot being highlighted
				if (HighlightedSlot != -1)
				{
					UnHighlightInventorySlot();
				}
			}

			if (TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}

			if(TraceHitItem != nullptr && TraceHitItem->GetPickupWidget() != nullptr)
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();

				if (Inventory.Num() >= INVENTORY_CAPACITY)
				{
					//! Inventory full
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					//! Inventory has space
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			} 

			if (TraceHitItemLastFrame != TraceHitItem)
			{
				if (TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		} 
	}
	else if (TraceHitItemLastFrame)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItemLastFrame->DisableCustomDepth();
		TraceHitItemLastFrame = nullptr;
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	//! Check the TsubClassOf variable 
	if (DefaultWeaponClass)
	{
		//! Spawn the Weapon
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if (WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			//! Attach the weapon to the RightHandSocket
			HandSocket->AttachActor(WeaponToEquip,GetMesh());
		}

		// EquippedWeapon == nullptr
		if (EquippedWeapon == nullptr && Inventory.Num() == 0)
		{
			//! -1  == No EquippedWeapon yet. No need to reserve the icon animation
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if (!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		//! Set Equiped weapon to newly spawned weapon
		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentRules);

		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
		EquippedWeapon = nullptr;
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
}

void AShooterCharacter::SelectButtonReleased()
{
	
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{

	if (Inventory.Num() > EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}

	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm,Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR,StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	return EquippedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	//! Play fire sound
	if (EquippedWeapon->GetFireSound()) 
	{
		UGameplayStatics::PlaySound2D(this,EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	if (EquippedWeapon->GetMuzzleFlash())
	{
		UGameplayStatics::SpawnEmitterAttached(EquippedWeapon->GetMuzzleFlash(),EquippedWeapon->GetItemMesh(),TEXT("BarrelSocket"));
	}

	//! Send Bullet
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName(TEXT("BarrelSocket"));
	if(BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		FHitResult BeamHitResult;

		bool bBeamEndSuccess = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);
		if (bBeamEndSuccess)
		{

			if (BeamHitResult.GetActor())
			{
				IBulletHitInterface* HitInterface = Cast<IBulletHitInterface>(BeamHitResult.GetActor());
				if (HitInterface)
				{
					HitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}

				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.GetActor());
				if (HitEnemy)
				{
					int32 Damage{};
					bool HeadShot{};
					if (BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBone())
					{
						//! Head Shot
						Damage = EquippedWeapon->GetHeadShotDamage();
						HeadShot = true;
					}
					else
					{
						//! Body Shot
						Damage = EquippedWeapon->GetDamage();
						HeadShot = false;
					}

					HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, HeadShot);
					// UE_LOG(LogTemp, Warning, TEXT("Bone hit: %s"), *BeamHitResult.BoneName.ToString());
					UGameplayStatics::ApplyDamage(BeamHitResult.GetActor(), Damage, GetController(), this, UDamageType::StaticClass());

				}
				else
				{
					//! Spawn default particles
					if(ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
					}
				}
			}
			else
			{
				//! Spawn default particles
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}
			}
			
			 
			//! After Line Traces spawn Impact and Beam particles
			if(BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				
				if(Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
				}
			}
		}	

	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	//! Play Animation Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	} 
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied && CombatState != ECombatState::ECS_FireTimerInProgress) return;
	if (EquippedWeapon == nullptr) return;
	
	//? Do we have ammo of the correct type?
	if (CarryingAmmo() && !EquippedWeapon->ClipIsFull()) 
	{
		if (bAiming)
		{
			StopAiming();
		}

		CombatState = ECombatState::ECS_Reloading;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
	else
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void AShooterCharacter::FinishReloading()
{
	if (CombatState == ECombatState::ECS_Stunned) return;

	//! Update the combat state 
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimButtonPressed)
	{
		Aim();
	}

	if (EquippedWeapon == nullptr) return;

	const EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	//TODO: Update AmmoMap
	if (AmmoMap.Contains(AmmoType))
	{
		//! Amount of ammo the Character is carrying of the EquippedWeapon type
		int32 CarriedAmmo = AmmoMap[AmmoType];

		const int32 MagEmptySpace = EquippedWeapon->GetMagazineCapacity() - EquippedWeapon->GetAmmo(); 

		if (MagEmptySpace > CarriedAmmo)
		{
			//! Reload the magazine with all the ammo we are carrying
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
		}
		else
		{
			//! fill the magazine
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
		}
		AmmoMap[AmmoType] = CarriedAmmo;
	}

	if (bFireButtonPressed)
	{
		FireWeapon();
	}
}

void AShooterCharacter::FinishEquipping()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if (bAimButtonPressed)
	{
		Aim();
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	
	EAmmoType AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	//! Index for the clip bone on the equipped weapon
	int32 ClipBoneIndex{ EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName()) };
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("Hand_L")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReleaseClip()
{
	EquippedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if(!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}

	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
	}
	else
	{
		if(!bAiming)
		{
			GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		}
	}
}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	float TargetCapsuleHalfHeight{};
	if (bCrouching)
	{
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
		GetCharacterMovement()->GroundFriction = CrouchingGroundFriction;
	}
	else
	{
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}

	const float InterpHalfHeight{ FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 20.f) };

	//! Negative when Stand->Crouch; Positive when Crouch->Standing
	const float DeltaCapsuleHalfHeight{ InterpHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	const FVector MeshOffset(0.f, 0.f, -DeltaCapsuleHalfHeight);

	GetMesh()->AddLocalOffset(MeshOffset);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::PickupAmmo(class AAmmo* Ammo)
{
	//! Check to see if AmmoMap contains Ammo's AmmoType
	if (AmmoMap.Contains(Ammo->GetAmmoType()))
	{
		//! Get Amount of ammo in our AmmoMap for Ammo's type
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		//! Check to see if the gun is empty
		if (EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{WeaponInterpComp, 0};
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLocation1{InterpComp1, 0}; 
	InterpLocations.Add(InterpLocation1);

	FInterpLocation InterpLocation2{InterpComp2, 0}; 
	InterpLocations.Add(InterpLocation2);

	FInterpLocation InterpLocation3{InterpComp3, 0}; 
	InterpLocations.Add(InterpLocation3);

	FInterpLocation InterpLocation4{InterpComp4, 0}; 
	InterpLocations.Add(InterpLocation4);

	FInterpLocation InterpLocation5{InterpComp5, 0}; 
	InterpLocations.Add(InterpLocation5);

	FInterpLocation InterpLocation6{InterpComp6, 0}; 
	InterpLocations.Add(InterpLocation6);
}

void AShooterCharacter::FKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}

void AShooterCharacter::OneKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}

void AShooterCharacter::TwoKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
}

void AShooterCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
}

void AShooterCharacter::FourKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
}

void AShooterCharacter::FiveKeyPressed()
{
	if (EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if (CurrentItemIndex != NewItemIndex && NewItemIndex < Inventory.Num() && (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping))
	{

		if (bAiming)
		{
			StopAiming();
		}
		AWeapon* OldEquippedWeapon = EquippedWeapon;
		AWeapon* NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);

		EquipWeapon(NewWeapon);

		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		OldEquippedWeapon->HideAccessories();
		NewWeapon->SetItemState(EItemState::EIS_Equipped);
		NewWeapon->ShowAccessories();

		CombatState = ECombatState::ECS_Equipping;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage);
			AnimInstance->Montage_JumpToSection(FName("Equip"));
		}

		NewWeapon->PlayEquipSound(true);
	}
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex{ 1 };
	int32 LowestCount{ INT_MAX };

	for (int i = 1;i < InterpLocations.Num();i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for (int32 i = 0;i < Inventory.Num();i++)
	{
		if (Inventory[i] == nullptr)
		{
			return i;
		}
	}

	if (Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}

	return -1; //! Inventory is full
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot{ GetEmptyInventorySlot() };
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start{ GetActorLocation() };
	const FVector End{ Start + FVector(0.f, 0.f, -400.f) };
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

void AShooterCharacter::GameStartAnimationFinished()
{
	GameStartAnimation = false;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController)
    {
        EnableInput(PlayerController);
    }
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimButtonPressed)
	{
		Aim();
	}

	if (EquippedWeapon->GetAmmo() <= 0)
	{
		ReloadWeapon();
	}
}

void AShooterCharacter::Die()
{
	bGameEnded = true;
	bDead = true;
	bFireButtonPressed = false;
	GetWorldTimerManager().ClearTimer(AutoFireTimer);

	RemoveStunnedWidget();

	AUltimateShooterGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AUltimateShooterGameModeBase>();
	if(GameMode != nullptr)
	{
		GameMode->CharacterKilled(this);
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
	
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;

}

void AShooterCharacter::ShowStunnedWidget()
{
	if (StunnedWidgetClass)
	{
		GetWorldTimerManager().ClearTimer(StunnedWidgetTimer);

		if (IsValid(StunnedWidget))
		{
			StunnedWidget->RemoveFromParent();
			StunnedWidget = nullptr;
		}
		
		StunnedWidget = CreateWidget<UUserWidget>(GetWorld(), StunnedWidgetClass);
		if (StunnedWidget)
		{
			StunnedWidget->AddToViewport();
		}

		GetWorldTimerManager().SetTimer(StunnedWidgetTimer, this, &AShooterCharacter::RemoveStunnedWidget, StunnedWidgetDuration);
	}
}

void AShooterCharacter::RemoveStunnedWidget()
{
	if (IsValid(StunnedWidget))
	{
		StunnedWidget->RemoveFromParent();
		StunnedWidget = nullptr;
	}
}
void AShooterCharacter::FinishWinning()
{
	GetMesh()->bPauseAnims = true;
}


// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//! Handle Interpolation for zoom when aiming
	CameraZooming(DeltaTime);
	//! Change look sensitivity when aiming
	SetLookRates();
	//! Calculate crosshair spread mulitplier
	CalculateCrosshairSpread(DeltaTime);
	//! Cheched OverlappedItemCount, then trace for items 
	TraceForItems();
	//! Interpolate the capsule half height based on crouching/standing
	InterpCapsuleHalfHeight(DeltaTime);

	if (Health <= 0.f)
	{
		float Length = FMath::FInterpTo(CameraBoom->TargetArmLength, 1200.f, DeltaTime, 0.5f);
		CameraBoom->TargetArmLength = Length;
	}
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"),this,&AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"),this,&AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookUpRate"),this,&AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis(TEXT("TurnRate"),this,&AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis(TEXT("LookUp"),this,&AShooterCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("Turn"),this,&AShooterCharacter::Turn);

	PlayerInputComponent->BindAction(TEXT("Jump"),IE_Pressed,this,&AShooterCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"),IE_Released,this,&ACharacter::StopJumping);
	
	PlayerInputComponent->BindAction(TEXT("FireButton"),IE_Pressed,this,&AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("FireButton"),IE_Released,this,&AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction(TEXT("AimingButton"),IE_Pressed,this,&AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction(TEXT("AimingButton"),IE_Released,this,&AShooterCharacter::AimingButtonReleased);
	
	PlayerInputComponent->BindAction(TEXT("Select"),IE_Pressed,this,&AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Select"),IE_Released,this,&AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction(TEXT("ReloadButton"),IE_Pressed,this,&AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction(TEXT("Crouch"),IE_Pressed,this,&AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction(TEXT("FKey"),IE_Pressed,this,&AShooterCharacter::FKeyPressed);
	PlayerInputComponent->BindAction(TEXT("1Key"),IE_Pressed,this,&AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction(TEXT("2Key"),IE_Pressed,this,&AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction(TEXT("3Key"),IE_Pressed,this,&AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction(TEXT("4Key"),IE_Pressed,this,&AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction(TEXT("5Key"),IE_Pressed,this,&AShooterCharacter::FiveKeyPressed);
}

float AShooterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, 
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount >= Health)
	{
		Health = 0.f;
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		Die();

		AEnemyController* EnemyController = Cast<AEnemyController>(EventInstigator);
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CharacterDead"), true);
			EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), nullptr);
		}
	}
	else
	{
		Health -= DamageAmount;
	}

	return DamageAmount;
}

void AShooterCharacter::Heal(float Amount)
{
	if (Health + Amount >= MaxHealth)
	{
		Health = MaxHealth;
	}
	else
	{
		Health += Amount;
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if (OverlappedItemCount + Amount <= 0)
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

//! No longer needed AItem has its own GetInterpLocation
// FVector AShooterCharacter::GetCameraInterpLocation()
// {
// 	//! Camera world location, can get location like this of all components
// 	const FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
// 	const FVector CameraForward{ FollowCamera->GetForwardVector() };
// 	//! Desired = CameraWorldLocation + Forward * A + Up * B

// 	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector(0.f, 0.f, CameraInterpElevation);
// }

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	if (Item)
	{
		Item->PlayEquipSound();
	}

	AWeapon* Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		if (Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
			Weapon->HideAccessories();
		}
		else //! Inventory is full; Swap with EquippedWeapon
		{
			SwapWeapon(Weapon);
		}
	}

	AAmmo* Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index >= 0 && Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;

	if (InterpLocations.Num() >= Index && Index >= 0)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}

void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}

void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	//! timer
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}

void AShooterCharacter::StartEquipSoundTimer()
{
	bShouldPlayEquipSound = false;
	//! timer
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

void AShooterCharacter::Stun(EHitDirection Direction)
{
	if (Health <= 0.f) return;

	CombatState = ECombatState::ECS_Stunned;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		FName SectionName;
		switch(Direction)
		{
			case EHitDirection::Front:
				SectionName = FName("HitReactFront");
				break;
			case EHitDirection::Back:
				SectionName = FName("HitReactBack");
				break;
			case EHitDirection::Right:
				SectionName = FName("HitReactRight");
				break;
			case EHitDirection::Left:
				SectionName = FName("HitReactLeft");
			break;
		}

		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);

		ShowStunnedWidget();
	}
}

void AShooterCharacter::CharacterWon()
{
	bGameEnded = true;
	bFireButtonPressed = false;
	GetWorldTimerManager().ClearTimer(AutoFireTimer);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && WinningMontage)
	{
		AnimInstance->Montage_Play(WinningMontage);
	}
	
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		DisableInput(PlayerController);
	}
}