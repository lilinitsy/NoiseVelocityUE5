// Fill out your copyright notice in the Description page of Project Settings.


#include "GaborEnhanceWithRerenderTestChar.h"


// Sets default values
AGaborEnhanceWithRerenderTestChar::AGaborEnhanceWithRerenderTestChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGaborEnhanceWithRerenderTestChar::BeginPlay()
{
	Super::BeginPlay();
	
	const FVector2f foveation_center(0.5f, 0.5f); // middle
	const float radius_fovea = 0.1f;
	const float radius_periphery = 0.2f;
	const float screen_width_cm = 60.0f;
	const float screen_height_cm = 30.0f;
	const float distance_from_screen_cm = 71.0f;
	const float s_k = 21.02f;
	const unsigned int cells = 64;
	const unsigned int impulses_per_cell = 32;
	const unsigned int seed = 10;

	view_extension = FSceneViewExtensions::NewExtension<FGaborEnhancementWithRerenderingViewExtension>(
		render_every_n_frames,
		foveation_center,
		radius_fovea,
		radius_periphery,
		screen_width_cm,
		screen_height_cm,
		distance_from_screen_cm,
		blur_rate_arcmin_per_degree,
		use_radially_increasing_blur,
		s_k,
		cells,
		impulses_per_cell,
		seed,
		frequency_scale,
		phase_cycles_per_sec,
		phase_strength,
		region_mode);


}

// Called every frame
void AGaborEnhanceWithRerenderTestChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (take_screenshot)
	{
#if WITH_EDITOR
		const FString ImageDirectory = FString::Printf(TEXT("%s/%s"), *FPaths::ProjectDir(), TEXT("Screenshots"));
#else
		const FString ImageDirectory = FString::Printf(TEXT("%s/../%s"), *FPaths::ProjectDir(), TEXT("Screenshots"));
#endif
		const FString ImageFilename = FString::Printf(TEXT("%s/Screenshot_%d%02d%02d_%02d%02d%02d_%03d.png"), *ImageDirectory, FDateTime::Now().GetYear(), FDateTime::Now().GetMonth(), FDateTime::Now().GetDay(), FDateTime::Now().GetHour(), FDateTime::Now().GetMinute(), FDateTime::Now().GetSecond(), FDateTime::Now().GetMillisecond());
		FScreenshotRequest::RequestScreenshot(ImageFilename, false, false);

		UE_LOG(LogTemp, Log, TEXT("Screenshot taken"));
	}
}

// Called to bind functionality to input
void AGaborEnhanceWithRerenderTestChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("toggle_screenshot", IE_Pressed, this, &AGaborEnhanceWithRerenderTestChar::toggle_screenshot);

}


void AGaborEnhanceWithRerenderTestChar::toggle_screenshot()
{
	UE_LOG(LogTemp, Log, TEXT("Screenshot taken"));
	take_screenshot = !take_screenshot;
}