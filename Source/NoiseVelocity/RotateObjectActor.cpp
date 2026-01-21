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

	if (left_moving_object)
	{
		FVector delta_translation = translation_meters_per_second * DeltaTime;
		FRotator delta_rotation = rotation_deg_per_second * DeltaTime;
		FTransform delta_transform = FTransform(delta_rotation, delta_translation, FVector(1.0f, 1.0f, 1.0f));

		// rotate around z axis and translate in given direction
		if (left_moving_object)
		{
			left_moving_object->AddActorLocalTransform(delta_transform);
		}

		if (right_moving_object)
		{
			right_moving_object->AddActorLocalTransform(delta_transform);
		}
	}

	else
	{
		UE_LOG(LogTemp, Log, TEXT("NO ACTOR FOUND"));

	}

	running_oscillation_timer += DeltaTime;
	if (running_oscillation_timer >= oscillation_time)
	{
		running_oscillation_timer = 0.0f;
		translation_meters_per_second *= -1.0f;
	}
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
