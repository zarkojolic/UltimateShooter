// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UltimateShooter/Weapons/Ammo.h"
#include "UltimateShooter/Weapons/Weapon.h"
#include "UltimateShooter/GameModes/UltimateShooterGameModeBase.h"

// Sets default values
AEnemy::AEnemy() :
	Health{100.f}, 
	MaxHealth{100.f}, 
	HealthBarDisplayTime{4.f}, 
	bCanHitReact{true}, 
	HitReactTimeMin{0.3f}, 
	HitReactTimeMax{0.6f}, 
	HitNumberDestroyTime{1.5f},
	bStunned{false}, 
	StunChance{0.5f}, 
	AttackLFast{TEXT("AttackLFast")}, 
	AttackRFast{TEXT("AttackRFast")}, 
	AttackL{TEXT("AttackL")}, 
	AttackR{TEXT("AttackR")},
	AttackCFast{TEXT("None")},
	AttackC{TEXT("None")},
	BaseDamage{20.f}, 
	LeftWeaponSocket{TEXT("FX_Trail_L_02")}, 
	RightWeaponSocket{TEXT("FX_Trail_R_02")},
	bCanAttack{true}, 
	AttackWaitTime{1.f}, 
	bDying{false},
	IsLastHeadshot{false},
	LootDropRate{0.1f}
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//! Create the Agro Sphere
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Agro Sphere"));
	AgroSphere->SetupAttachment(GetRootComponent());

	//! Create the Combat Range Sphere
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Combat Range Sphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());

	//! Construct Left and Right collision boxes
	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Left Weapon Box"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));

	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Right Weapon Box"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));	
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOverlap);

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereEndOverlap);

	//! Bind functions to overlap events for weapon boxes
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);
	
	//! Set collision presets for weapon boxes
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	//! Get AI Controller
	EnemyController = Cast<AEnemyController>(GetController());

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
	
	FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
	FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint"), WorldPatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolPoint2"), WorldPatrolPoint2);

		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

void AEnemy::ShowHealthBar_Implementation()
{
	GetWorldTimerManager().ClearTimer(HealthBarTimer);
	GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);
}

void AEnemy::Die(AShooterCharacter* Character, bool Headshot)
{
	if (bDying) return;
	bDying = true;

	AUltimateShooterGameModeBase* GameMode = GetWorld()->GetAuthGameMode<AUltimateShooterGameModeBase>();
	if(GameMode != nullptr)
	{
		GameMode->CharacterKilled(this);
	}

	DeactivateLeftWeapon();
	DeactivateRightWeapon();
	HideHealthBar();

	if (Character && EnemyType.IsEqual(FName("Grux")) || EnemyType.IsEqual(FName("Minion")))
	{
		FVector DirectionToCharacter = Character->GetActorLocation() - GetActorLocation();
		DirectionToCharacter.Normalize();

		FVector Forward = GetActorForwardVector();

		float ForwardDot = FVector::DotProduct(Forward, DirectionToCharacter);
		FName SectionName;

		if (EnemyType.IsEqual(FName("Minion")))
		{
			if (IsLastHeadshot)
			{
				SectionName = FName("DeathHeadshot");
				FTransform SocketTransform = GetMesh()->GetSocketTransform(FName("HeadExplosion"));
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MinionDeathParticles, SocketTransform);
				if (HeadshotExplosionSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HeadshotExplosionSound, SocketTransform.GetLocation(), 1.f);
				}
			}
			else
			{
				if (ForwardDot > 0)
				{
					int Number = FMath::RandRange(0,1);
					if (Number == 0)
					{
						SectionName = FName("DeathFront");
						DeathDirection = EHitDirection::Front;
					}
					else
					{
						SectionName = FName("DeathFrontTwist");
						DeathDirection = EHitDirection::Front;
					}
				}
				else
				{
					SectionName = FName("DeathBack");
					DeathDirection = EHitDirection::Back;
				}
			}
		}
		else if (EnemyType.IsEqual(FName("Grux")))
		{
			if (ForwardDot > 0)
			{
				SectionName = FName("DeathFront");
				DeathDirection = EHitDirection::Front;
			}
			else
			{
				SectionName = FName("DeathBack");
				DeathDirection = EHitDirection::Back;
			}
		}
		
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && DeathMontage)
		{
			AnimInstance->Montage_Play(DeathMontage);
			AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
		}

	}
	else if (EnemyType.IsEqual(FName("Khaimera")))
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && DeathMontage)
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
		EnemyController->StopMovement();
	}

	GetMovementComponent()->StopActiveMovement();

	SpawnWeaponAndAmmo();
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(HitMontage, PlayRate);
		AnimInstance->Montage_JumpToSection(Section,HitMontage); 
	}

	bCanHitReact = false;

	float HitReactTime = FMath::FRandRange(HitReactTimeMin, HitReactTimeMax);
	GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReactTimer, HitReactTime);  
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (EnemyType.IsEqual(FName("Minion")))
		{
			AnimInstance->Montage_Play(AttackMontage, 1.5f);
		}
		else
		{
			AnimInstance->Montage_Play(AttackMontage, PlayRate);
		}
		AnimInstance->Montage_JumpToSection(Section, AttackMontage);
	}

	bCanAttack = false;
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
	}
}

FName AEnemy::GetAttackSectionName()
{
	FName SectionName;
	if (AttackCFast.IsEqual(FName("None")))
	{
		const int32 SectionNum{ FMath::RandRange(1,4) };
		switch(SectionNum)
		{
			case 1:
				SectionName = AttackLFast;
				break;
			case 2:
				SectionName = AttackRFast;
				break;
			case 3:
				SectionName = AttackL;
				break; 
			case 4:
				SectionName = AttackR;
				break;
		}
	}
	else
	{
		const int32 SectionNum{ FMath::RandRange(1,6) };
		switch(SectionNum)
		{
			case 1:
				SectionName = AttackLFast;
				break;
			case 2:
				SectionName = AttackRFast;
				break;
			case 3:
				SectionName = AttackCFast;
				break; 
			case 4:
				SectionName = AttackL;
			break;
			case 5:
				SectionName = AttackR;
				break;
			case 6:
				SectionName = AttackC;
		}
	}
	

	return SectionName;
}

FName AEnemy::GetHitReactDirection(const FHitResult& HitResult)
{
	// Vektor koji pokazuje napred u odnosu na rotaciju karaktera
	const FVector Forward = GetActorForwardVector();
	
	// Vektor od karaktera do mesta udara
	const FVector ImpactDirection = (HitResult.ImpactPoint - GetActorLocation()).GetSafeNormal();

	// Skalarni proizvod daje cos(ugla) između dva vektora
	const float DotProduct = FVector::DotProduct(Forward, ImpactDirection);

	// Izračunaj ugao u stepenima
	const float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

	// Vektorski proizvod daje pravac – koristimo Z komponentu da odredimo levo/desno
	const FVector CrossProduct = FVector::CrossProduct(Forward, ImpactDirection);
	const float Direction = CrossProduct.Z;

	// Na osnovu ugla i pravca određujemo naziv sekcije
	if (AngleDegrees <= 45.f)
	{
		return FName("HitReactFront");
	}
	else if (AngleDegrees > 45.f && AngleDegrees <= 135.f)
	{
		return Direction > 0.f ? FName("HitReactRight") : FName("HitReactLeft");
	}
	else
	{
		return FName("HitReactBack");
	}
}

void AEnemy::ResetHitReactTimer()
{
	bCanHitReact = true;
}

void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber,Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitTimerDelegate;
	HitTimerDelegate.BindUFunction(this, FName("DestroyHitNumber"), HitNumber);

	GetWorld()->GetTimerManager().SetTimer(HitNumberTimer, HitTimerDelegate, HitNumberDestroyTime, false);
}

void AEnemy::DestroyHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	//! TPair<UUserWidget*, FVector>&
	for (auto& HitPair : HitNumbers)
	{
		UUserWidget* HitNumber{ HitPair.Key };
		const FVector Location{ HitPair.Value };

		FVector2D ScreenLocation;
		UGameplayStatics::ProjectWorldToScreen(GetWorld()->GetFirstPlayerController(), Location, ScreenLocation);

		HitNumber->SetPositionInViewport(ScreenLocation);
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitNumbers();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
{

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, HitResult.Location);
	}

	const float Stunned = FMath::FRandRange(0.f, 1.f);
	if (Stunned <= StunChance)
	{
		//! Stun the enemy
		if (bCanHitReact)
		{
			PlayHitMontage(GetHitReactDirection(HitResult));
		}

		SetStunned(true);
		//! bStunned = true;
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
	}

	if (Health - DamageAmount <= 0.f)
	{
		Health = 0.f;
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		Die(Cast<AShooterCharacter>(DamageCauser));
	}
	else
	{
		Health -= DamageAmount;
	}

	if (bDying) return DamageAmount;

	ShowHealthBar();

	return DamageAmount;
}

void AEnemy::AgroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;

	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		if (EnemyController)
		{
			if (EnemyController->GetBlackboardComponent())
			{
				EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
			}
		}
	}
}

void AEnemy::SetStunned(bool Stunned)
{
	bStunned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"), Stunned);
	}
}

void AEnemy::CombatSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) return;
	
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		bInAttackRange = true;	
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true);
		}
	}
}

void AEnemy::CombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) return;
	
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		bInAttackRange = false;	
		if (EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"),false);
		}
	}	
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		EHitDirection HitDirection;
		GetCharacterDirection(Character, HitDirection);

		DoDamage(Character);

		EHitDirection MontageDirection;
		SpawnBlood(Character, HitDirection, true, MontageDirection);

		StunCharacter(Character, MontageDirection);
	}
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		EHitDirection HitDirection;
		GetCharacterDirection(Character, HitDirection);
		
		DoDamage(Character);
		
		EHitDirection MontageDirection;
		SpawnBlood(Character, HitDirection, false, MontageDirection);

		StunCharacter(Character, MontageDirection);
	}

}

void AEnemy::ActivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateLeftWeapon()
{
	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateRightWeapon()
{
	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DoDamage(AShooterCharacter* Victim)
{
	if (Victim == nullptr) return;

	UGameplayStatics::ApplyDamage(Victim, BaseDamage, EnemyController, this, UDamageType::StaticClass());

	if (Victim->GetMeleImpactSound())
	{
		UGameplayStatics::PlaySoundAtLocation(this, Victim->GetMeleImpactSound(), Victim->GetActorLocation() );
	}
}

void AEnemy::SpawnBlood(AShooterCharacter* Victim, EHitDirection Direction, bool LeftWeapon, EHitDirection& MontageDirection)
{
	if (Direction == EHitDirection::None) return;

	if (Victim)
	{
		const USkeletalMeshSocket* CharacterBloodSocket = nullptr;
		
		switch(Direction)
		{
			case EHitDirection::Front:
				if (LeftWeapon)
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetRightBloodSocketName());
					MontageDirection = EHitDirection::Right;
				}
				else
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetLeftBloodSocketName());
					MontageDirection = EHitDirection::Left;
				}
				break;
			case EHitDirection::Back:
				if (LeftWeapon)
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetLeftBloodSocketName());
					MontageDirection = EHitDirection::Left;
				}
				else
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetRightBloodSocketName());
					MontageDirection = EHitDirection::Right;
				}
			break;
			case EHitDirection::Right:
				if (LeftWeapon)
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetBackBloodSocketName());
					MontageDirection = EHitDirection::Back;
				}
				else
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetForwardBloodSocketName());
					MontageDirection = EHitDirection::Front;
				}
				break;
			case EHitDirection::Left:
				if (LeftWeapon)
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetForwardBloodSocketName());
					MontageDirection = EHitDirection::Front;
				}
				else
				{
					CharacterBloodSocket = Victim->GetMesh()->GetSocketByName(Victim->GetBackBloodSocketName());
					MontageDirection = EHitDirection::Back;
				}
			break;
		}
		
		if (CharacterBloodSocket)
		{
			const FTransform SocketTransform{ CharacterBloodSocket->GetSocketTransform(Victim->GetMesh()) };
			if (Victim->GetBloodParticles())
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Victim->GetBloodParticles(), SocketTransform);
			}
		} 
	} 
}

void AEnemy::GetCharacterDirection(AShooterCharacter* Character, EHitDirection& Direction)
{
	if (Character)
	{
		FVector DirectionToEnemy = GetActorLocation() - Character->GetActorLocation();
		DirectionToEnemy.Normalize();

		FVector Forward = Character->GetActorForwardVector();
		FVector Right = Character->GetActorRightVector();

		float ForwardDot = FVector::DotProduct(Forward, DirectionToEnemy);
		float RightDot = FVector::DotProduct(Right, DirectionToEnemy);

		if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
		{
			Direction = (ForwardDot > 0) ? EHitDirection::Front: EHitDirection::Back;
		}
		else
		{
			Direction = (RightDot > 0) ? EHitDirection::Right : EHitDirection::Left;
		}
	}
	else
	{
		Direction = EHitDirection::None;
	}
	
}

void AEnemy::StunCharacter(AShooterCharacter* Character, EHitDirection Direction)
{
	if (Character)
	{
		const float Stun{ FMath::FRandRange(0.f, 1.f) };
		if (Stun <= Character->GetStunChance() )
		{
			Character->Stun(Direction);
		}
	}
}

void AEnemy::ResetCanAttack()
{
	bCanAttack = true;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
}

void AEnemy::FinishDeath()
{
	Destroy();
}

void AEnemy::FreezeInDeathPose()
{
	GetMesh()->bPauseAnims = true;
	
}

void AEnemy::SpawnWeaponAndAmmo()
{
	AWeapon* Weapon = nullptr;
	AAmmo* Ammo = nullptr;

	FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 55.f); //! (0.f, 0.f, CapsuleComponentHalfHeight*1.5f)
	FRotator SpawnRotation = FRotator(0.f, FMath::RandRange(-160.f, 160.f), 0.f);
	
	if (WeaponClass)
	{
		float DropPercent = FMath::FRandRange(0.f,1.f);
		if (DropPercent <= LootDropRate)
		{	
			Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnLocation, SpawnRotation);
			Weapon->SetItemState(EItemState::EIS_Falling);
			Weapon->SetUpSpawnedWeapon();
			Weapon->ThrowWeapon();
		}
	}

	if (AmmoSMGClass && AmmoARClass)
	{
		int Num = FMath::RandRange(1,2);
		if (Num == 1)
		{
			Ammo = GetWorld()->SpawnActor<AAmmo>(AmmoSMGClass, SpawnLocation, SpawnRotation);
		}
		else
		{
			Ammo = GetWorld()->SpawnActor<AAmmo>(AmmoARClass, SpawnLocation, SpawnRotation);
		}

		Ammo->ThrowAmmo();
	}

}