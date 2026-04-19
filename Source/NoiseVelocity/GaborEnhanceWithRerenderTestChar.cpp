// Fill out your copyright notice in the Description page of Project Settings.


#include "GaborEnhanceWithRerenderTestChar.h"


// Sets default values
AGaborEnhanceWithRerenderTestChar::AGaborEnhanceWithRerenderTestChar()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	CameraComponent->bUsePawnControlRotation = false;

}

// Called when the game starts or when spawned
void AGaborEnhanceWithRerenderTestChar::BeginPlay()
{
	Super::BeginPlay();
	
	// Eye tracker stuff
	FVector2D viewport_size;
	tobii_api = TobiiGameIntegration::GetApi("NoiseVelocity");
	GEngine->GameViewport->GetViewportSize(viewport_size);
	tobii_api->GetTrackerController()->TrackRectangle({ 0, 0, (int)viewport_size.X, (int)viewport_size.Y });


	const FVector2f foveation_center(0.5f, 0.5f); // middle
	const float radius_fovea = 0.1f;
	const float radius_periphery = 0.2f;
	const float screen_width_cm = 60.0f;
	const float screen_height_cm = 33.0f;
	const float distance_from_screen_cm = 42.8f;
	const float s_k = 21.02f;
	const unsigned int cells = 64;
	const unsigned int impulses_per_cell = 32;
	const unsigned int seed = 10;
	float fov_deg = 2.0f * FMath::RadiansToDegrees(FMath::Atan((screen_width_cm / 2.0f) / distance_from_screen_cm));
	//float fov_deg = 100.0f;
	CameraComponent->SetFieldOfView(fov_deg);

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
		region_mode,
		comparison_mode);

	if (fixation_cross_widget_class)
	{
		fixation_cross_widget = CreateWidget<UUserWidget>(GetWorld(), fixation_cross_widget_class);
		if (fixation_cross_widget)
		{
			fixation_cross_widget->AddToViewport(999); // high z-order so it's always on top
		}
	}
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

	// Eye tracker stuff
	if(use_eyetracking)
	{
		tobii_api->Update();
		TobiiGameIntegration::GazePoint gaze_point;
		if (tobii_api->GetStreamsProvider()->GetLatestGazePoint(gaze_point))
		{
			TobiiGameIntegration::GazePoint normalized_gaze;
			tobii_api->GetStreamsProvider()->ConvertGazePoint(
				gaze_point,
				normalized_gaze,
				TobiiGameIntegration::UnitType::SignedNormalized,
				TobiiGameIntegration::UnitType::Normalized);

			FVector2f gaze_uv = FVector2f(normalized_gaze.X, normalized_gaze.Y);
			gaze_uv.Y = 1.0f - gaze_uv.Y; // flip y coordinate
			//gaze_pos = FVector2f(gaze_point.X, gaze_point.Y);
			gaze_pos = gaze_uv;
			//view_extension->foveation_center = gaze_uv;
			//UE_LOG(LogTemp, Log, TEXT("Gaze uv: %f %f"), gaze_uv.X, gaze_uv.Y);
		}
		//UE_LOG(LogTemp, Log, TEXT("Gaze pixel: %f %f"), gaze_point.X, gaze_point.Y);
	}


	// This requires actor to be set to Movable
	/*FVector location = this->GetActorLocation();
	location.X += 1.0f;
	this->SetActorLocation(location);*/
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

void AGaborEnhanceWithRerenderTestChar::update_view_extension()
{
	view_extension->blur_rate_arcmin_per_degree = blur_rate_arcmin_per_degree;
	view_extension->use_radially_increasing_blur = use_radially_increasing_blur;
	view_extension->render_every_n_frames = render_every_n_frames;
	view_extension->frequency_scale = frequency_scale;
	view_extension->phase_cycles_per_sec = phase_cycles_per_sec;
	view_extension->phase_strength = phase_strength;
	view_extension->region_mode = region_mode;
	view_extension->comparison_mode = comparison_mode;
}
