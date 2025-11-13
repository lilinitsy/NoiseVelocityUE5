// Fill out your copyright notice in the Description page of Project Settings.


#include "GaussianBlurTestCharacter.h"


// Sets default values
AGaussianBlurTestCharacter::AGaussianBlurTestCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGaussianBlurTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	


	const FVector2f foveation_center(0.5f, 0.5f); // middle
	const float radius_fovea = 0.1f;
	const float radius_periphery = 0.2f;
	const float screen_width_cm = 60.0f;
	const float screen_height_cm = 30.0f;
	const float distance_from_screen = 71.0f;

	view_extension = FSceneViewExtensions::NewExtension<FGaussianBlurViewExtension>(
		foveation_center,
		radius_fovea,
		radius_periphery,
		screen_width_cm,
		screen_height_cm,
		distance_from_screen,
		blur_rate_arcmin_per_degree,
		use_radially_increasing_blur);
}

// Called every frame
void AGaussianBlurTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGaussianBlurTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

