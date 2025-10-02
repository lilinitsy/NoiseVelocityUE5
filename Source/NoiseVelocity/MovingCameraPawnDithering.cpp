// Fill out your copyright notice in the Description page of Project Settings.

#include "MovingCameraPawnDithering.h"

#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"



// Sets default values
AMovingCameraPawnDithering::AMovingCameraPawnDithering()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	origin = CreateDefaultSubobject<USceneComponent>(TEXT("origin"));
	origin->SetupAttachment(RootComponent);

	camera = CreateDefaultSubobject<UCameraComponent>(TEXT("camera"));
	camera->SetupAttachment(origin);


	left_scenecapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("left_scenecapture"));
	right_scenecapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("right_scenecapture"));
	left_scenecapture->SetupAttachment(camera);
	right_scenecapture->SetupAttachment(camera);

	left_scenecapture->bCaptureEveryFrame = true;
	right_scenecapture->bCaptureEveryFrame = true;

	// Ensure scene captures are enabled
	left_scenecapture->SetComponentTickEnabled(true);
	right_scenecapture->SetComponentTickEnabled(true);

	// Materials are set from the editor, with UPROPERTY ensuring that
}

// Called when the game starts or when spawned
void AMovingCameraPawnDithering::BeginPlay()
{
	Super::BeginPlay();


}


void AMovingCameraPawnDithering::initialize()
{
	// Figure out screen viewport size
	UGameViewportClient* viewport_client = GetWorld()->GetGameViewport();

	// If this isn't checked, then the editor will crash
	if (viewport_client)
	{
		FVector2D viewport_size;
		viewport_client->GetViewportSize(viewport_size);

		UE_LOG(LogTemp, Warning, TEXT("Viewport size: %d %d\n"), (int32)viewport_size.X, (int32)viewport_size.Y);


		if (composite_material)
		{
			UMaterialInstanceDynamic* dynamic_material = UMaterialInstanceDynamic::Create(composite_material, this);

			camera->PostProcessSettings.AddBlendable(dynamic_material, 1.0f);
		}

		UE_LOG(LogTemp, Warning, TEXT("MovingCameraPawnDithering set up to play!"));
	}
}



// Called every frame
void AMovingCameraPawnDithering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!initialized)
	{
		initialize();
		initialized = true;
	}


	if (num_ticks > 900)
	{
		move_scene_capture_component2d_forward(left_scenecapture, experiment_condition_movement_speed, DeltaTime);
	}

	if (num_ticks > 1000)
	{
		move_scene_capture_component2d_forward(right_scenecapture, reference_movement_speed, DeltaTime);
	}


	// UE_LOG(LogTemp, Warning, TEXT("Num left showonlyactors: %u\n"), left_scenecapture->ShowOnlyActors.Num());
	// UE_LOG(LogTemp, Warning, TEXT("Num left showonlycomponents: %u\n"), left_scenecapture->ShowOnlyComponents.Num());

	num_ticks++;
}

// Called to bind functionality to input
void AMovingCameraPawnDithering::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("UpArrow", IE_Pressed, this, &AMovingCameraPawnDithering::on_up_pressed);
	PlayerInputComponent->BindAction("DownArrow", IE_Pressed, this, &AMovingCameraPawnDithering::on_down_pressed);

}

void AMovingCameraPawnDithering::on_up_pressed()
{
	experiment_condition_movement_speed += 10.0f; // move speed up by .1m/s

	UE_LOG(LogTemp, Warning, TEXT("Movement speed: %f\n"), experiment_condition_movement_speed);
}

void AMovingCameraPawnDithering::on_down_pressed()
{
	experiment_condition_movement_speed -= 10.0f;

	UE_LOG(LogTemp, Warning, TEXT("Movement speed: %f\n"), experiment_condition_movement_speed);
}



void AMovingCameraPawnDithering::move_scene_capture_component2d_forward(USceneCaptureComponent2D* scene_capture, float movement_speed, float dt)
{
	// Get camera forward vector; will move along this
	FVector forward = camera->GetForwardVector();
	FVector new_position = scene_capture->GetComponentLocation() + movement_speed * forward * dt;
	scene_capture->SetWorldLocation(new_position);
}