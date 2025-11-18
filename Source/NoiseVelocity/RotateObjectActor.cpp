// Fill out your copyright notice in the Description page of Project Settings.


#include "RotateObjectActor.h"


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

	for (AActor* actor : GetWorld()->GetLevel(0)->Actors)
	{
		if (actor)
		{
			if (actor->Tags.Contains(tag_to_find))
			{
				rotating_object = actor;
			}
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
	}

	else
	{
		UE_LOG(LogTemp, Log, TEXT("NOT FOUND"));
	}
}

