// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GaborEnhanceWithRerenderTestChar.h"

#include "Experiment1Manager.generated.h"


// Many things will be similar to RotateObjectActor.h/cpp, but this 
// provides a more comprehensive way to manage the experiment.
// STILL use RotateObjectActor for testing features!!!
UCLASS()
class NOISEVELOCITY_API AExperiment1Manager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExperiment1Manager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "User Settings")
	AGaborEnhanceWithRerenderTestChar *user;

	UPROPERTY(EditAnywhere, Category = "User Settings")
	float blur_rate_arcmin_per_degree = 0.34;

	// This should always be on EXCEPT for testing.
	UPROPERTY(EditAnywhere, Category = "User Settings")
	bool use_radially_increasing_blur = true;
	
	UPROPERTY(EditAnywhere, Category = "User Settings")
	float radius_fovea = 0.1f; // TODO: Make this use degrees

	UPROPERTY(EditAnywhere, Category = "User Settings")
	float radius_periphery = 0.2f; // TODO: Make this use degrees

	UPROPERTY(EditAnywhere, Category = "Setup Settings")
	float screen_width_cm = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Setup Settings")
	float screen_height_cm = 30.0f;
	UPROPERTY(EditAnywhere, Category = "Setup Settings")
	float distance_from_screen_cm = 71.0f;


	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FRotator left_rotation_deg_per_second = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FVector left_translation_meters_per_second = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FRotator right_rotation_deg_per_second = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FVector right_translation_meters_per_second = FVector(0.0f, 0.0f, 0.0f);


	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	float oscillation_time = 1.0f; // how long to let it move before it bounces back and forth

	/* how long to delay moving the right one so the motion isn't synced.
		It's probably good to not sync it evenly, ie not have it be half the period, but something
		that doesn't visually break well
	*/ 
	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	float right_delay_time = 0.35f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stimulus Settings")
	AActor* left_moving_object;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stimulus Settings")
	AActor* right_moving_object;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stimulus Settings")
	bool render_same_fps = true; 

	// If render_same_fps is true, this is discarded.
	// if !render_same_fps, only gaussian blur should be used.
	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	unsigned int render_every_n_frames = 0;

	// for tracking whether to move
	unsigned int left_framecount = 0;
	FVector left_delta_movement = FVector(0.0f, 0.0f, 0.0f);

	// hidden for modulating the oscillation time
	float left_running_oscillation_timer = 0.0f;
	float right_running_oscillation_timer = 0.0f;
	float total_time = 0.0f;

	void set_actor_to_mobile(AActor *actor);	
};
