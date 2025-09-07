// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"


// Sets default values
ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	origin = CreateDefaultSubobject<USceneComponent>(TEXT("origin"));
	origin->SetupAttachment(RootComponent);

	left_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("left_camera"));
	right_camera = CreateDefaultSubobject<UCameraComponent>(TEXT("right_camera"));
	left_camera->SetupAttachment(origin);
	right_camera->SetupAttachment(origin);
}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

