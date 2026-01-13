// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() : 
    ThrowWeaponTime{1.f},bFalling{false}, Ammo{30}, MagazineCapacity{30}, WeaponType{EWeaponType::EWT_SubmachineGun},
    AmmoType{EAmmoType::EAT_9mm}, ReloadMontageSection{FName(TEXT("Reload SMG"))}, ClipBoneName{TEXT("smg_clip")}, BoneToHide{FName("")},
    SlideDisplacement{0.f}, SlideDisplacementTime{0.2f}, MaxSlideDisplacement{8.f}, bAutomatic{true}
{
    PrimaryActorTick.bCanEverTick = true;

    SetItemType(EItemType::EIT_Weapon);

    ARSuppressor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AR Suppressor"));
    ARSuppressor->SetupAttachment(GetItemMesh()); 
    
    ARRedDot = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AR Red Dot"));
    ARRedDot->SetupAttachment(GetItemMesh());
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    if (BoneToHide != FName(""))
    {
        GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
    }

    SetUpAccessories(false);
}

void AWeapon::FinishMovingSlide()
{
    bMovingSlide = false;
}

void AWeapon::UpdateSlideDisplacement()
{
    if (SlideDisplacementCurve && bMovingSlide)
    {
        const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(SlideTimer);
        const float CurveValue = SlideDisplacementCurve->GetFloatValue(ElapsedTime);
        SlideDisplacement = CurveValue * MaxSlideDisplacement;
    }
}

void AWeapon::SetWeaponParameters()
{
    const FString WeaponTablePath{ TEXT("DataTable'/Game/_Game/DataTable/WeaponDataTable.WeaponDataTable'") };
    UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

    if (WeaponTableObject)
    {
        FWeaponDataTable* WeaponRow = nullptr;
        switch (WeaponType)
        {
            case EWeaponType::EWT_SubmachineGun:
                WeaponRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
                break;
            case EWeaponType::EWT_AssaultRifle:
                WeaponRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
                break;
            case EWeaponType::EWT_Pistol:
                WeaponRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
                break;
        }

        if (WeaponRow)
        {
            AmmoType = WeaponRow->AmmoType;
            Ammo = WeaponRow->WeaponAmmo;
            MagazineCapacity = WeaponRow->MagazineCapacity;
            SetPickupSound(WeaponRow->PickupSound);
            SetEquipSound(WeaponRow->EquipSound);
            GetItemMesh()->SetSkeletalMesh(WeaponRow->ItemMesh);
            SetItemName(WeaponRow->ItemName);
            SetIconItem(WeaponRow->InventoryIcon);
            SetAmmoItem(WeaponRow->AmmoIcon);

            SetMaterialInstance(WeaponRow->MaterialInstance);
            PreviousMaterialIndex = GetMaterialIndex();
            GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
            SetMaterialIndex(WeaponRow->MaterialIndex);
            SetClipBoneName(WeaponRow->ClipBoneName);
            SetReloadMontageSection(WeaponRow->ReloadMontageSection);
            GetItemMesh()->SetAnimInstanceClass(WeaponRow->AnimBP);
            CrosshairsMiddle = WeaponRow->CrosshairsMiddle;
            CrosshairsTop = WeaponRow->CrosshairsTop;
            CrosshairsBottom = WeaponRow->CrosshairsBottom;
            CrosshairsLeft = WeaponRow->CrosshairsLeft;
            CrosshairsRight = WeaponRow->CrosshairsRight;
            AutoFireRate = WeaponRow->AutoFireRate;
            MuzzleFlash = WeaponRow->MuzzleFlash;
            FireSound = WeaponRow->FireSound;
            BoneToHide = WeaponRow->BoneToHide;
            bAutomatic = WeaponRow->bAutomatic;
            Damage = WeaponRow->Damage;
            HeadShotDamage = WeaponRow->HeadShotDamage;
        }

        if (GetMaterialInstance())
        {
            SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance(), this));
            GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FersnelColor"), GetGlowColor());
            GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());

            EnableGlowMaterial();
        }
    }
}

void AWeapon::SetWeaponDamage()
{
    if (WeaponType == EWeaponType::EWT_Pistol)
    {
        switch (GetItemRarity())
        {
            case EItemRarity::EIR_Damaged:
                Damage = 10.f;
                HeadShotDamage = 15.f;
            break;
            case EItemRarity::EIR_Common:
                Damage = 12.f;
                HeadShotDamage = 17.f;
            break;
            case EItemRarity::EIR_Uncommon:
                Damage = 14.f;
                HeadShotDamage = 20.f;
            break;
            case EItemRarity::EIR_Rare:
                Damage = 15.f;
                HeadShotDamage = 25.f;
            break;
            case EItemRarity::EIR_Legendary:
                Damage = 50.f;
                HeadShotDamage = 100.f;
            break;
        }
    }
    else if (WeaponType == EWeaponType::EWT_SubmachineGun)
    {
        switch (GetItemRarity())
        {
            case EItemRarity::EIR_Damaged:
                Damage = 11.f;
                HeadShotDamage = 16.f;
            break;
            case EItemRarity::EIR_Common:
                Damage = 13.f;
                HeadShotDamage = 18.f;
            break;
            case EItemRarity::EIR_Uncommon:
                Damage = 15.f;
                HeadShotDamage = 20.f;
            break;
            case EItemRarity::EIR_Rare:
                Damage = 18.f;
                HeadShotDamage = 25.f;
            break;
            case EItemRarity::EIR_Legendary:
                Damage = 35.f;
                HeadShotDamage = 42.f;
            break;
        }
    }
    else if (WeaponType == EWeaponType::EWT_AssaultRifle)
    {
        switch (GetItemRarity())
        {
            case EItemRarity::EIR_Damaged:
                Damage = 15.f;
                HeadShotDamage = 20.f;
            break;
            case EItemRarity::EIR_Common:
                Damage = 18.f;
                HeadShotDamage = 25.f;
            break;
            case EItemRarity::EIR_Uncommon:
                Damage = 20.f;
                HeadShotDamage = 30.f;
            break;
            case EItemRarity::EIR_Rare:
                Damage = 24.f;
                HeadShotDamage = 34.f;
            break;
            case EItemRarity::EIR_Legendary:
                Damage = 40.f;
                HeadShotDamage = 50.f;
            break;
        }
    }
}

void AWeapon::HideAccessories()
{
    if (WeaponType == EWeaponType::EWT_AssaultRifle)
    {
        if (ARSuppressor)
        {
            ARSuppressor->SetHiddenInGame(true);
            ARSuppressor->SetVisibility(false);
        }
        
        if (ARRedDot)
        {
            ARRedDot->SetHiddenInGame(true);
            ARRedDot->SetVisibility(false);
        }
    }
}

void AWeapon::ShowAccessories()
{
    if (WeaponType == EWeaponType::EWT_AssaultRifle)
    {
        if (ARSuppressor)
        {
            ARSuppressor->SetHiddenInGame(false);
            ARSuppressor->SetVisibility(true);
        }
        
        if (ARRedDot)
        {
            ARRedDot->SetHiddenInGame(false);
            ARRedDot->SetVisibility(true);
        }
    }
}

void AWeapon::SetUpAccessories(bool Spawned)
{
    if (WeaponType == EWeaponType::EWT_AssaultRifle)
    {
        if (ARSuppressor && ARSuppressor->GetStaticMesh())
        {
            ARSuppressor->AttachToComponent(GetItemMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("SuppressorSocket"));
            ARSuppressor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ARSuppressor->SetComponentTickEnabled(false);
        }
        
        if (ARRedDot && ARRedDot->GetStaticMesh())
        {
            ARRedDot->AttachToComponent(GetItemMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("RedDotSocket"));
            ARRedDot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ARRedDot->SetComponentTickEnabled(false);
        }

        if (Spawned)
        {
            ShowAccessories();
        }
    }
    else
    {
        if (ARSuppressor)
        {
            ARSuppressor->SetHiddenInGame(true);
            ARSuppressor->SetVisibility(false);
            ARSuppressor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ARSuppressor->SetComponentTickEnabled(false);
        }
        
        if (ARRedDot)
        {
            ARRedDot->SetHiddenInGame(true);
            ARRedDot->SetVisibility(false);
            ARRedDot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            ARRedDot->SetComponentTickEnabled(false);
        }
    }
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //! Keep the Weapon upright
    if (GetItemState() == EItemState::EIS_Falling && bFalling)
    {
        const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    UpdateSlideDisplacement();
}

void AWeapon::ThrowWeapon()
{
    bFalling = true;
    FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

    const FVector MeshForward{ GetItemMesh()->GetForwardVector() };
    const FVector MeshRight{ GetItemMesh()->GetRightVector() };
    //! Direction in which we throw the weapon
    FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

    float RandomRotation{ FMath::FRandRange(-15.f,15.f) }; 
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
    if (WeaponType == EWeaponType::EWT_AssaultRifle)
    {
        ImpulseDirection *= 2'000.f;
    }
    else
    {
        ImpulseDirection *= 5'000.f;
    }
    GetItemMesh()->AddImpulse(ImpulseDirection);

    GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);

    EnableGlowMaterial();
} 

void AWeapon::DecrementAmmo()
{
    if (Ammo - 1 <= 0)
    {
        Ammo = 0;
    }
    else
    {
        --Ammo;
    }
}

void AWeapon::StopFalling()
{
    SetItemState(EItemState::EIS_Pickup);
    bFalling = false;
    StartPulseTimer();
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    //! Initialize all parameters from data table
    SetWeaponParameters();
    //! Set the Damage based on Item Rarity
    SetWeaponDamage();
}

void AWeapon::StartSlideTimer()
{
    bMovingSlide = true;
    GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, SlideDisplacementTime);
}

void AWeapon::ReloadAmmo(int32 Amount)
{
    checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attempted to reload with more than magazine capacity!"));
    Ammo += Amount;
}

bool AWeapon::ClipIsFull()
{
    return Ammo >= MagazineCapacity;
}

void AWeapon::SetUpSpawnedWeapon()
{
    int WeaponTypeNum = FMath::RandRange(1,3);
    EWeaponType Type = EWeaponType::EWT_DefaultMAX;
    EItemRarity Rarity = EItemRarity::EIR_MAX;

    //! 0.001% 4.9% 25% 70% 
    float RarityNum = FMath::FRandRange(0.f,1.f);
    if (RarityNum > 0.999f)
    {
        Rarity = EItemRarity::EIR_Legendary;
    }
    else if (RarityNum > 0.95f)
    {
        Rarity = EItemRarity::EIR_Rare;
    }
    else if (RarityNum > 0.7)
    {
        Rarity = EItemRarity::EIR_Uncommon;
    }
    else
    {
        Rarity = EItemRarity::EIR_Common;
    }

    switch(WeaponTypeNum)
    {
        case 1:
            Type = EWeaponType::EWT_AssaultRifle;
            break;
        case 2:
            Type = EWeaponType::EWT_SubmachineGun;
            break;
        case 3:
            Type = EWeaponType::EWT_Pistol;
        break;
    }

    if (Type != EWeaponType::EWT_DefaultMAX && Rarity != EItemRarity::EIR_MAX)
    {
        SetItemRarity(Rarity);
        WeaponType = Type;

        SetWeaponParameters();
        SetWeaponDamage();

        if (BoneToHide != FName(""))
        {
            GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
        }

        SetUpAccessories(true);
    }
}