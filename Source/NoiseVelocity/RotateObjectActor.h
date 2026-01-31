// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RotateObjectActor.generated.h"

UCLASS()
class NOISEVELOCITY_API ARotateObjectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARotateObjectActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FRotator left_rotation_deg_per_second = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Settings")
	FVector left_translation_meters_per_second = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Settings")
	FRotator right_rotation_deg_per_second = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Settings")
	FVector right_translation_meters_per_second = FVector(0.0f, 0.0f, 0.0f);


	UPROPERTY(EditAnywhere, Category = "Settings")
	float oscillation_time = 1.0f; // how long to let it move before it bounces back and forth

	/* how long to delay moving the right one so the motion isn't synced.
		It's probably good to not sync it evenly, ie not have it be half the period, but something
		that doesn't visually break well
	*/ 
	UPROPERTY(EditAnywhere, Category = "Settings")
	float right_delay_time = 0.35f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Actors")
	AActor* left_moving_object;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Actors")
	AActor* right_moving_object;


	// hidden for modulating the oscillation time
	float left_running_oscillation_timer = 0.0f;
	float right_running_oscillation_timer = 0.0f;
	float total_time = 0.0f;

	void set_actor_to_mobile(AActor *actor);

};
