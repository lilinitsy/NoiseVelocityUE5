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

	//TArray<AActor*> found_actors;
	//UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*tag_to_find), found_actors);

	/*if (found_actors.Num() > 0)
	{
		rotating_object = found_actors[0];

		// Ensure that the actor is movable
		if (USceneComponent* root = rotating_object->GetRootComponent())
		{
			root->SetMobility(EComponentMobility::Movable);
		}
	}*/
	// Ensure that the actor is movable
	if(rotating_object)
	{
		USceneComponent* root = rotating_object->GetRootComponent();
		if(root)
		{
			root->SetMobility(EComponentMobility::Movable);
		}
	}
}

// Called every frame
void ARotateObjectActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (rotating_object)
	{
		UE_LOG(LogTemp, Log, TEXT("Actor found"));

		float delta_rotation = rotation_deg_per_second * DeltaTime;

		// rotate around z axis
		rotating_object->AddActorLocalRotation(FRotator(0.0f, 0.0f, delta_rotation));
	}

	else
	{
		UE_LOG(LogTemp, Log, TEXT("NO ACTOR FOUND"));

	}
}

