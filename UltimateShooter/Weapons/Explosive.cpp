// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"

// Sets default values
AExplosive::AExplosive() :
	Damage{100.f}
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Explosive Mesh"));
	SetRootComponent(ExplosiveMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{
	if (ExplosiveParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosiveParticles, HitResult.Location, FRotator(0.f), true);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitResult.Location);
	}

	// TODO: Apply explosive damage
	TArray<AActor*> OverlappingActors;
	OverlapSphere->GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for (AActor* Actor : OverlappingActors)
	{
		if (Actor == Shooter)
		{
			Actor->TakeDamage(Damage, FDamageEvent::FDamageEvent(), ShooterController, this);
		}
		else
		{
			UGameplayStatics::ApplyDamage(Actor, Damage, ShooterController, Shooter, UDamageType::StaticClass());
		}
	}

	Destroy();
}