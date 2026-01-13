// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "UltimateShooter/Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UltimateShooter/Weapons/Weapon.h"
#include "UltimateShooter/Enums/WeaponType.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UShooterAnimInstance::UShooterAnimInstance() :
    Speed{0.f}, bIsInAir{false}, bIsAccelerating{false}, MovementOffsetYaw{0.f}, LastMovementOffsetYaw{0.f},
    bAiming{false}, TIPCharacterYaw{0.f}, TIPCharacterYawLastFrame{0.f}, RootYawOffset{0.f}, RotationCurve{-90.f}, 
    RotationCurveLastFrame{0.f}, Pitch{0.f}, bReloading{0.f}, OffsetState{EOffsetState::EOS_Hip}, CharacterRotation{FRotator(0.f)},
    CharacterRotationLastFrame{FRotator(0.f)}, YawDelta{0.f}, RecoilWeight{1.0f}, bTurningInPlace{false}, 
    EquippedWeaponType{EWeaponType::EWT_DefaultMAX}, bShouldUseFABRIK{false}
{

}

//! Works like tick
void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if(ShooterCharacter == nullptr)
    {
        ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
    }

    if (ShooterCharacter)
    {
        bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
        bEquipping = ShooterCharacter->GetCombatState() == ECombatState::ECS_Equipping;
        bCrouching = ShooterCharacter->GetCrouching();
        if (ShooterCharacter->GetGameStartAnimation())
        {
            bShouldUseFABRIK = false;
        }
        else
        {
            bShouldUseFABRIK = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied || ShooterCharacter->GetCombatState() == ECombatState::ECS_FireTimerInProgress;
        }
        
        //! Get the lateral speed of the character
        FVector Velocity{ ShooterCharacter->GetVelocity() };
        Velocity.Z = 0.f;
        Speed = Velocity.Size();

        //! Is the character in the air
        bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

        //! Is the character Accelerating
        if(ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
        {
            bIsAccelerating = true;
        }
        else
        {
            bIsAccelerating = false;
        }

        //! Character Aiming Direction
        FRotator AimRotation = ShooterCharacter->GetBaseAimRotation(); 
        //! Character Movement Direction
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
        //! Subtract Movement Rotation from Aiming Rotation to get Movement Yaw Offset for Animation Blend Space
        MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,AimRotation).Yaw;

        if(Velocity.Size() > 0.f)
        {
            LastMovementOffsetYaw = MovementOffsetYaw;
        }

        bAiming = ShooterCharacter->GetAiming();

        if (bReloading)
        {
            OffsetState = EOffsetState::EOS_Reloading;
        }
        else if (bIsInAir)
        {
            OffsetState = EOffsetState::EOS_InAir;
        }
        else if (bAiming)
        {
            OffsetState = EOffsetState::EOS_Aiming;
        }
        else 
        {
            OffsetState = EOffsetState::EOS_Hip;
        }

        if (ShooterCharacter->GetEquippedWeapon())
        {
            EquippedWeaponType = ShooterCharacter->GetEquippedWeapon()->GetWeaponType();
        }
    }

    TurnInPlace();
    Lean(DeltaTime);
}

//! Works like Constructor
void UShooterAnimInstance::NativeInitializeAnimation()
{
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
    bAiming = false;
}

void UShooterAnimInstance::TurnInPlace()
{
    if (ShooterCharacter == nullptr) return;

    Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

    if (Speed > 0 || bIsInAir)
    {
        //! Don't want to turn in place character is moving
        RootYawOffset = 0.f;
        TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        RotationCurve = 0.f;
        RotationCurveLastFrame = 0.f;
    }
    else
    {
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

        const float TIPYawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;

        //! RootYaw Offset, updated and clamped to [-180,180]
        RootYawOffset = UKismetMathLibrary::NormalizeAxis( RootYawOffset - TIPYawDelta );

        const float Turning{ GetCurveValue(TEXT("Turning")) };
        if (Turning > 0)
        {
            bTurningInPlace = true;
            RotationCurveLastFrame = RotationCurve;
            RotationCurve = GetCurveValue(TEXT("Rotation"));
            const float DeltaRotation{ RotationCurve - RotationCurveLastFrame };

            //! RootYawOffset > 0 => Turning Left || RootYawOffset < 0 => Turning Right
            RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation; 

            const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
            if (ABSRootYawOffset > 90.f)
            {
                const float YawExess{ ABSRootYawOffset - 90.f };
                RootYawOffset > 0 ? RootYawOffset -= YawExess : RootYawOffset += YawExess;
            }
        }
        else
        {
            bTurningInPlace = false;
        }
    }

    if (bTurningInPlace)
    {
        if(bReloading || bEquipping)
        {
            RecoilWeight = 1.0f;
        }
        else
        {
            RecoilWeight = 0.f;
        }
    }
    else //! not turning in place
    {
        if (bCrouching)
        {
            if(bReloading || bEquipping)
            {
                RecoilWeight = 1.0f;
            }
            else
            {
                RecoilWeight = 0.3f;
            }
        }
        else
        {
            RecoilWeight = 1.0f;
        }
        //! else 
        // {
            //! if (bAiming)
            // {
            //!     RecoilWeight = 1.f;
            // }
            //! else
            // {
            //!     RecoilWeight = 0.5f; 
            // }
        // }
    }
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
    if (ShooterCharacter == nullptr) return; 
    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = ShooterCharacter->GetActorRotation();

    FRotator Delta { UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame) };

    const float Target{ static_cast<float>(Delta.Yaw) / DeltaTime };
    const float Interp{ FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
    YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
}
