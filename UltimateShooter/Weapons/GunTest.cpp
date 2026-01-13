// Fill out your copyright notice in the Description page of Project Settings.


#include "GunTest.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
AGunTest::AGunTest()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	RootComponent = Root;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	Mesh->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AGunTest::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGunTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

