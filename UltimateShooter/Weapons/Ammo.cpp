// Fill out your copyright notice in the Description page of Project Settings.


#include "Ammo.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "UltimateShooter/Characters/ShooterCharacter.h"

AAmmo::AAmmo() :
    AmmoType{EAmmoType::EAT_9mm},
	ThrowAmmoTime{1.f}
{
	PrimaryActorTick.bCanEverTick = true;
	
    AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AmmoMesh"));
    SetRootComponent(AmmoMesh);

    GetCollisionBox()->SetupAttachment(GetRootComponent());
    GetPickupWidget()->SetupAttachment(GetRootComponent());
    GetAreaSphere()->SetupAttachment(GetRootComponent());

	AmmoCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AmmoCollisionSphere"));
	AmmoCollisionSphere->SetupAttachment(GetRootComponent());
	AmmoCollisionSphere->SetSphereRadius(50.f);

	SetItemType(EItemType::EIT_Ammo);

	GetAmmoMesh()->SetMassOverrideInKg(NAME_None, 15.f, true);
}

void AAmmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	if (GetItemState() == EItemState::EIS_Falling && bFalling)
    {
        const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
        GetAmmoMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }
}

void AAmmo::BeginPlay()
{
    Super::BeginPlay();

	AmmoCollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmo::AmmoSphereOverlap);
}

void AAmmo::SetItemProperties(EItemState State)
{
    Super::SetItemProperties(State);

    switch(State)
	{
		case EItemState::EIS_Pickup:
			//! Set mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			break;
		case EItemState::EIS_Equipped:
			//! Set mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;

		case EItemState::EIS_Falling:
			//! Set mesh properties
			AmmoMesh->SetSimulatePhysics(true);
			AmmoMesh->SetEnableGravity(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			AmmoMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic,ECollisionResponse::ECR_Block);

			break;
		case EItemState::EIS_EquipInterping:
			//! Set Item mesh properties
			AmmoMesh->SetSimulatePhysics(false);
			AmmoMesh->SetEnableGravity(false);
			AmmoMesh->SetVisibility(true);
			AmmoMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			AmmoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			break;
	}
}

void AAmmo::AmmoSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* OverlappedCharacter = Cast<AShooterCharacter>(OtherActor);
		if (OverlappedCharacter)
		{
			StartItemCurve(OverlappedCharacter);
			AmmoCollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AAmmo::EnableCustomDepth()
{
	AmmoMesh->SetRenderCustomDepth(true);
}

void AAmmo::DisableCustomDepth()
{
	AmmoMesh->SetRenderCustomDepth(false);
}

void AAmmo::ThrowAmmo()
{
	SetItemState(EItemState::EIS_Falling);

	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

    const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
    const FVector MeshRight{ GetItemMesh()->GetRightVector() };
    //! Direction in which we throw the weapon
    FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

    float RandomRotation{ FMath::FRandRange(-15.f,15.f) }; 
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
    ImpulseDirection *= 10'000.f;
    GetItemMesh()->AddImpulse(ImpulseDirection);

    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowAmmoTimer, this, &AAmmo::StopFalling, ThrowAmmoTime);

    EnableGlowMaterial();
}

void AAmmo::StopFalling()
{
	SetItemState(EItemState::EIS_Pickup);
    bFalling = false;
    StartPulseTimer();
}