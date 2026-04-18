// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GaborEnhanceWithRerenderTestChar.h"
#include "foveation_utils.h"

#include "Experiment1AltManager.generated.h"



enum class EXP1_ALT_STIMULI
{
	STIMULI0, // placeholder
	STIMULI1, // placeholder
	COUNT
};

// deprecate
enum class EXP1_ALT_LEFTRIGHT
{
	LEFT,
	//RIGHT,
	COUNT
};


enum class EXP1_ALT_CONDITION
{
	GABOR_NOISE,
	GAUSSIAN_BLUR,
	COUNT,
};


struct Exp1AltTrial
{
	EXP1_ALT_STIMULI stimuli;
	EXP1_ALT_LEFTRIGHT leftright;
	EXP1_ALT_CONDITION condition;
	int render_every_n_fps; // 1 = same, 2, or 3 are options
	int eccentricity;
	float frequency;
	float velocity;
};





enum class EXP1_ALT_EXPERIMENT_STATE
{
	WAITING_FOR_INPUT, // only used at start
	BLACK_SCREEN,
	TRIAL_RUNNING,
};


// Many things will be similar to RotateObjectActor.h/cpp, but this 
// provides a more comprehensive way to manage the experiment.
// STILL use RotateObjectActor for testing features!!!
UCLASS()
class NOISEVELOCITY_API AExperiment1AltManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExperiment1AltManager();

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

	UPROPERTY(EditAnywhere, Category = "Gaze Settings")
	float max_gaze_eccentricity_deg = 10.0f; // how far from center is allowed

	bool screen_blacked_from_gaze = false;


	UPROPERTY(EditAnywhere, Category = "Setup Settings")
	float screen_width_cm = 120.0f;
	UPROPERTY(EditAnywhere, Category = "Setup Settings")
	float screen_height_cm = 60.0f;
	UPROPERTY(EditAnywhere, Category = "Setup Settings")
	float distance_from_screen_cm = 71.0f;


	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	UMaterialInterface* stimuli0_material;

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	UMaterialInterface* stimuli1_material;

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FRotator left_rotation_deg_per_second = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FVector left_translation_meters_per_second = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FRotator right_rotation_deg_per_second = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	FVector right_translation_meters_per_second = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	float oscillation_min_z = -100.0f;

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	float oscillation_max_z = 100.0f;



	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	float oscillation_time = 1.0f; // how long to let it move before it bounces back and forth

	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	float trial_length_time = 5.0f; // in seconds

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
	uint32 render_every_n_frames = 0;

	// Adjust as monitor refresh rate is changed
	UPROPERTY(EditAnywhere, Category = "Stimulus Settings")
	uint32 target_framerate = 60; 

	// for tracking whether to move
	uint32 left_framecount = 0;
	FVector left_delta_movement = FVector(0.0f, 0.0f, 0.0f);

	// hidden for modulating the oscillation time
	float left_running_oscillation_timer = 0.0f;
	float right_running_oscillation_timer = 0.0f;
	float total_trial_time = 0.0f;
	uint32  current_trial_index = 0;
	EXP1_ALT_EXPERIMENT_STATE experiment_state = EXP1_ALT_EXPERIMENT_STATE::WAITING_FOR_INPUT;

	// Original object locations for resetting the state
	FTransform left_object_original_transform;
	FTransform right_object_original_transform;

	float current_velocity_magnitude = 0.0f;


	TArray<Exp1AltTrial> trials;


	void set_actor_to_mobile(AActor *actor);	

	void initialize_trials();
	void start_trial();
	void on_response_recorded();
	void on_increase_velocity();
	void on_decrease_velocity();

	FVector eccentricity_to_world_pos(float eccentricity_deg, EXP1_ALT_LEFTRIGHT side, float z_cm);
	float choose_initial_velocity_for_stimuli(int tgt_framerate, int every_n_fps, float freq);
	void set_screen_black(bool black);
	void apply_material_for_stimuli(EXP1_ALT_STIMULI stimuli);

	void move_object_left();
	void move_object_right();

	void write_trial_to_csv(const Exp1AltTrial& trial);

};
