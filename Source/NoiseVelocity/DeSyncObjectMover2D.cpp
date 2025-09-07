// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"
#include "Engine/World.h"

#include "DeSyncObjectMover2D.h"



// Sets default values
ADeSyncObjectMover2D::ADeSyncObjectMover2D()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADeSyncObjectMover2D::BeginPlay()
{
	Super::BeginPlay();
	
	initialize_object_positions(start_eccentricity, object_dist_from_camera);

	UE_LOG(LogTemp, Warning, TEXT("initialize_object_positions happened"));
}

// Called every frame
void ADeSyncObjectMover2D::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (num_ticks > 300)
	{
		move_object_horizontally(left_object, start_eccentricity, end_eccentricity, object_dist_from_camera, 1.0f);
		move_object_horizontally(right_object, -start_eccentricity, -end_eccentricity, object_dist_from_camera, 1.0f);
	}

	num_ticks++;
}


// This will initialize objects a certain distance, where the movement happens
// in an ARC around the camera at a certain distance, not using a planar location.
void ADeSyncObjectMover2D::initialize_object_positions(float eccentricity, float dist_from_camera)
{
	if (!camera_pawn)
	{
		UE_LOG(LogTemp, Error, TEXT("No camera pawn found! Cannot initialize objects"));
		return;
	}

	if (!left_object)
	{
		UE_LOG(LogTemp, Error, TEXT("No left object found! Cannot initialize objects"));
		return;
	}

	if (!right_object)
	{
		UE_LOG(LogTemp, Error, TEXT("No left object found! Cannot initialize objects"));
		return;
	}

	// Make sure objects are movable
	left_object->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	right_object->GetRootComponent()->SetMobility(EComponentMobility::Movable);

	ACameraPawn *casted_camera_pawn = Cast<ACameraPawn>(camera_pawn);
	UCameraComponent *left_camera = casted_camera_pawn->left_camera;
	FVector camera_right = left_camera->GetRightVector();
	FVector center_position = get_center_position_of_camera_look(left_camera, dist_from_camera);

	float eccentricity_radians = FMath::DegreesToRadians(eccentricity);
	float offset_distance = FMath::Tan(eccentricity_radians) * dist_from_camera;

	FVector left_position = center_position + camera_right * -offset_distance;
	FVector right_position = center_position + camera_right * offset_distance;
	
	left_object->SetActorLocation(left_position);
	right_object->SetActorLocation(right_position);


	// Now figure out the start and end positions for the given start and end eccentricities

}

void ADeSyncObjectMover2D::move_object_vertically(AActor* object, float start_ecc, float end_ecc, float dist_from_camera, float period)
{
	float current_time = GetWorld()->GetTimeSeconds();
	float cycle_position = FMath::Fmod(current_time, period) / period;

	float linear_factor = 0.0f;
	if (cycle_position <= 0.5f)
	{
		linear_factor = cycle_position * 2.0f;
	}
	else
	{
		linear_factor = 2.0f - (cycle_position * 2.0f);
	}

	ACameraPawn* casted_camera_pawn = Cast<ACameraPawn>(camera_pawn);
	UCameraComponent* left_camera = casted_camera_pawn->left_camera;
	FVector camera_right = left_camera->GetRightVector();
	FVector center_position = get_center_position_of_camera_look(left_camera, dist_from_camera);

}


void ADeSyncObjectMover2D::move_object_horizontally(AActor* object, float start_ecc, float end_ecc, float dist_from_camera, float period)
{
	float current_time = GetWorld()->GetTimeSeconds();
	float cycle_position = FMath::Fmod(current_time, period) / period;

	float linear_factor = 0.0f;
	if (cycle_position <= 0.5f)
	{
		linear_factor = cycle_position * 2.0f;
	}
	else
	{
		linear_factor = 2.0f - (cycle_position * 2.0f);
	}

	ACameraPawn* casted_camera_pawn = Cast<ACameraPawn>(camera_pawn);
	UCameraComponent* left_camera = casted_camera_pawn->left_camera;
	FVector camera_right = left_camera->GetRightVector();
	FVector center_position = get_center_position_of_camera_look(left_camera, dist_from_camera);


	float current_ecc = FMath::Lerp(start_ecc, end_ecc, linear_factor);
	FVector final_position = center_position + camera_right * current_ecc;

	object->SetActorLocation(final_position);
}
