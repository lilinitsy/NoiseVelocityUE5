// Fill out your copyright notice in the Description page of Project Settings.


#include "GaborEnhancementTestCharacter.h"


// Sets default values
AGaborEnhancementTestCharacter::AGaborEnhancementTestCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGaborEnhancementTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	

	const FVector2f foveation_center(0.5f, 0.5f); // middle
	const float radius_fovea = 0.1f;
	const float radius_periphery = 0.2f;
	const float screen_width_cm = 60.0f;
	const float screen_height_cm = 30.0f;
	const float distance_from_screen_cm = 71.0f;
	const float blur_rate_arcmin_per_degree = 0.34f;
	const float s_k = 21.02f;
	const unsigned int cells = 64;
	const unsigned int impulses_per_cell = 8;
	const unsigned int seed = 10;

	view_extension = FSceneViewExtensions::NewExtension<FGaborEnhancementViewExtension>(
		foveation_center,
		radius_fovea,
		radius_periphery,
		screen_width_cm,
		screen_height_cm,
		distance_from_screen_cm,
		blur_rate_arcmin_per_degree,
		s_k,
		cells,
		impulses_per_cell,
		seed);

}

// Called every frame
void AGaborEnhancementTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGaborEnhancementTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

