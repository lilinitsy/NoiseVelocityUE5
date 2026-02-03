// Fill out your copyright notice in the Description page of Project Settings.


#include "RotateObjectActor.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
ARotateObjectActor::ARotateObjectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARotateObjectActor::BeginPlay()
{
	Super::BeginPlay();

	set_actor_to_mobile(left_moving_object);
	set_actor_to_mobile(right_moving_object);
}

// Called every frame
void ARotateObjectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DeltaTime = 0.0167f;

	if (left_moving_object && right_moving_object)
	{
		FVector left_delta_translation = left_translation_meters_per_second * DeltaTime;
		FRotator left_delta_rotation = left_rotation_deg_per_second * DeltaTime;
		FTransform left_delta_transform = FTransform(left_delta_rotation, left_delta_translation, FVector(1.0f, 1.0f, 1.0f));
		left_delta_movement += left_delta_translation;

		// rotate around z axis and translate in given direction
		if (left_moving_object)
		{
			// Check if we should simulate low fps for the purpose of testing blurred vs blurred
			if(render_same_fps)
			{
				left_moving_object->AddActorLocalTransform(left_delta_transform);
			}

			else
			{
				if (left_framecount % render_every_n_frames == 0)
				{
					left_delta_transform = FTransform(left_delta_rotation, left_delta_movement, FVector(1.0f, 1.0f, 1.0f));

					left_delta_movement = FVector(0.0f, 0.0f, 0.0f);
					left_moving_object->AddActorLocalTransform(left_delta_transform);

				}
			}
		}

		FVector right_delta_translation = right_translation_meters_per_second * DeltaTime;
		FRotator right_delta_rotation = right_rotation_deg_per_second * DeltaTime;
		FTransform right_delta_transform = FTransform(right_delta_rotation, right_delta_translation, FVector(1.0f, 1.0f, 1.0f));

		if (right_moving_object && total_time > right_delay_time)
		{
			right_moving_object->AddActorLocalTransform(right_delta_transform);
		}

		left_framecount++;
	}

	else
	{
		UE_LOG(LogTemp, Log, TEXT("NO ACTOR FOUND"));

	}

	left_running_oscillation_timer += DeltaTime;
	if (left_running_oscillation_timer >= oscillation_time)
	{
		left_running_oscillation_timer = 0.0f;
		left_translation_meters_per_second *= -1.0f;
	}

	if(total_time >= right_delay_time)
	{
		right_running_oscillation_timer += DeltaTime;
	}

	if (right_running_oscillation_timer >= oscillation_time)
	{
		right_running_oscillation_timer = 0.0f;
		right_translation_meters_per_second *= -1.0f;
	}


	total_time += DeltaTime; // for the timer delay on start
}



void ARotateObjectActor::set_actor_to_mobile(AActor* actor)
{
	if (actor)
	{
		USceneComponent* root = actor->GetRootComponent();
		if (root)
		{
			root->SetMobility(EComponentMobility::Movable);
		}
	}
}
