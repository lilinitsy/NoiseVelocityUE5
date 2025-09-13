// Fill out your copyright notice in the Description page of Project Settings.

#include "MovingCameraPawn.h"

#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"



// Sets default values
AMovingCameraPawn::AMovingCameraPawn()
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
void AMovingCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	// Figure out screen viewport size
	UGameViewportClient* viewport_client = GetWorld()->GetGameViewport();

	// If this isn't checked, then the editor will crash
	if (viewport_client)
	{
		FVector2D viewport_size;
		viewport_client->GetViewportSize(viewport_size);

		UE_LOG(LogTemp, Warning, TEXT("Viewport size: %d %d\n"), (int32)viewport_size.X, (int32)viewport_size.Y);


		// Create render targets
		UTextureRenderTarget2D* left_rendertarget = NewObject<UTextureRenderTarget2D>(this);
		UTextureRenderTarget2D* right_rendertarget = NewObject<UTextureRenderTarget2D>(this);



		// Set the render targets to use half width of this
		//left_rendertarget->InitAutoFormat(viewport_size.X / 2, viewport_size.Y);
		//right_rendertarget->InitAutoFormat(viewport_size.X / 2, viewport_size.Y);
		left_rendertarget->InitAutoFormat(viewport_size.X, viewport_size.Y);
		right_rendertarget->InitAutoFormat(viewport_size.X, viewport_size.Y);

		// Assign render targets
		left_scenecapture->TextureTarget = left_rendertarget;
		right_scenecapture->TextureTarget = right_rendertarget;

		// Object filtering so each scene capture only renders certain objects
		TArray<AActor*> left_actors;
		TArray<AActor*> right_actors;

		// Tag actors in the editor
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("left"), left_actors);
		UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("right"), right_actors);

		// Have to manually change PrimitiveRenderMode to use ShowOnlyActors
		left_scenecapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		right_scenecapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		left_scenecapture->ShowOnlyActors = left_actors;
		right_scenecapture->ShowOnlyActors = right_actors;

		UE_LOG(LogTemp, Warning, TEXT("Number of actors (left, right): (%d, %d)\n"), left_actors.Num(), right_actors.Num());


		if (composite_material)
		{
			UMaterialInstanceDynamic* dynamic_material = UMaterialInstanceDynamic::Create(composite_material, this);
			dynamic_material->SetTextureParameterValue(FName("left_texture"), left_scenecapture->TextureTarget);
			dynamic_material->SetTextureParameterValue(FName("right_texture"), right_scenecapture->TextureTarget);

			camera->PostProcessSettings.AddBlendable(dynamic_material, 1.0f);
		}

		UE_LOG(LogTemp, Warning, TEXT("MovingCameraPawn set up to play!"));
	}
}

// Called every frame
void AMovingCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (num_ticks > 300)
	{
		move_scene_capture_component2d_forward(left_scenecapture, DeltaTime);
	}

	if (num_ticks > 400)
	{
		move_scene_capture_component2d_forward(right_scenecapture, DeltaTime);
	}


	// UE_LOG(LogTemp, Warning, TEXT("Num left showonlyactors: %u\n"), left_scenecapture->ShowOnlyActors.Num());
	// UE_LOG(LogTemp, Warning, TEXT("Num left showonlycomponents: %u\n"), left_scenecapture->ShowOnlyComponents.Num());

	num_ticks++;
}

// Called to bind functionality to input
void AMovingCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AMovingCameraPawn::move_scene_capture_component2d_forward(USceneCaptureComponent2D* scene_capture, float dt)
{
	// Get camera forward vector; will move along this
	FVector forward = camera->GetForwardVector();
	FVector new_position = scene_capture->GetComponentLocation() + movement_speed * forward * dt;
	scene_capture->SetWorldLocation(new_position);

	UE_LOG(LogTemp, Warning, TEXT("Scenecap pos: %f %f %f\n"), scene_capture->GetComponentLocation().X, scene_capture->GetComponentLocation().Y, scene_capture->GetComponentLocation().Z);
}