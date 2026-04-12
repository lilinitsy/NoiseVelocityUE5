// Fill out your copyright notice in the Description page of Project Settings.


#include "Experiment1AltManager.h"

#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"


// Sets default values
AExperiment1AltManager::AExperiment1AltManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AExperiment1AltManager::BeginPlay()
{
	Super::BeginPlay();

	set_actor_to_mobile(left_moving_object);
	set_actor_to_mobile(right_moving_object);
	left_object_original_transform = left_moving_object->GetTransform();
	right_object_original_transform = right_moving_object->GetTransform();

	initialize_trials();

	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc)
	{
		EnableInput(pc);
		if (InputComponent)
		{
			InputComponent->BindAction("RecordResponse", IE_Pressed, this, &AExperiment1AltManager::on_response_recorded);
			InputComponent->BindAction("IncreaseVelocity", IE_Pressed, this, &AExperiment1AltManager::on_increase_velocity);
			InputComponent->BindAction("DecreaseVelocity", IE_Pressed, this, &AExperiment1AltManager::on_decrease_velocity);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Experiment ready. Press spacebar to start trial 1 of %d."), trials.Num());
}



void AExperiment1AltManager::on_response_recorded()
{
	UE_LOG(LogTemp, Log, TEXT("on_response_recorded triggered"));
	if (experiment_state == EXP1_ALT_EXPERIMENT_STATE::WAITING_FOR_INPUT)
	{
		start_trial();
	}
}

void AExperiment1AltManager::on_increase_velocity()
{
	current_velocity_magnitude += 50.0f;
	current_velocity_magnitude = FMath::Max(0.0f, current_velocity_magnitude);
	left_translation_meters_per_second.Z = FMath::Sign(left_translation_meters_per_second.Z) * current_velocity_magnitude;
	right_translation_meters_per_second.Z = FMath::Sign(right_translation_meters_per_second.Z) * current_velocity_magnitude;
}

void AExperiment1AltManager::on_decrease_velocity()
{
	current_velocity_magnitude -= 50.0f;
	current_velocity_magnitude = FMath::Max(0.0f, current_velocity_magnitude);
	left_translation_meters_per_second.Z = FMath::Sign(left_translation_meters_per_second.Z) * current_velocity_magnitude;
	right_translation_meters_per_second.Z = FMath::Sign(right_translation_meters_per_second.Z) * current_velocity_magnitude;
}

void AExperiment1AltManager::start_trial()
{
	if (current_trial_index >= static_cast<uint32>(trials.Num()))
	{
		UE_LOG(LogTemp, Log, TEXT("All trials complete."));
		return;
	}

	const Exp1AltTrial trial = trials[current_trial_index];

	// Reset all timers and motion state for the new trial
	total_trial_time = 0.0f;
	left_running_oscillation_timer = 0.0f;
	right_running_oscillation_timer = 0.0f;
	left_framecount = 0;
	left_delta_movement = FVector(0.0f, 0.0f, 0.0f);
	left_moving_object->SetActorTransform(left_object_original_transform);
	right_moving_object->SetActorTransform(right_object_original_transform);
	float z_distance = FMath::Abs(user->GetActorLocation().Y - left_moving_object->GetActorLocation().Y);

	FVector left_location = eccentricity_to_world_pos(trial.eccentricity, trial.leftright, z_distance);
	FVector right_location = left_location;
	right_location.Y *= -1.0f; // flip the y (leftright) axis
	left_moving_object->SetActorLocation(left_location);
	right_moving_object->SetActorLocation(right_location);
	current_velocity_magnitude = trial.velocity;
	left_translation_meters_per_second.Z = current_velocity_magnitude;
	right_translation_meters_per_second.Z = current_velocity_magnitude;

	// Assign new, and relevant, camera properties
	// Stimuli update?
	// Also need to get the right actor transforms based on eccentricity
	user->region_mode = (trial.leftright == EXP1_ALT_LEFTRIGHT::LEFT) ? 1 : 2; // set it to left (1) or right (2)
	user->update_view_extension();

	experiment_state = EXP1_ALT_EXPERIMENT_STATE::TRIAL_RUNNING;

	UE_LOG(LogTemp, Log, TEXT("Starting trial %d / %d"), current_trial_index, trials.Num());

}


// Called every frame
void AExperiment1AltManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//DeltaTime = 0.0167f;

	if (experiment_state != EXP1_ALT_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		return;
	}

	if (left_moving_object && right_moving_object)
	{
		float left_z = left_moving_object->GetActorLocation().Z;
		if (left_z >= oscillation_max_z && left_translation_meters_per_second.Z > 0.0f)
		{
			left_translation_meters_per_second.Z = -current_velocity_magnitude;
		}
		else if (left_z <= oscillation_min_z && left_translation_meters_per_second.Z < 0.0f)
		{
			left_translation_meters_per_second.Z = current_velocity_magnitude;
		}

		FVector left_delta_translation = left_translation_meters_per_second * DeltaTime;
		FRotator left_delta_rotation = left_rotation_deg_per_second * DeltaTime;
		left_delta_movement += left_delta_translation;

		if (render_same_fps)
		{
			FTransform left_delta_transform = FTransform(left_delta_rotation, left_delta_translation, FVector(1.0f, 1.0f, 1.0f));
			left_moving_object->AddActorLocalTransform(left_delta_transform);
		}
		else
		{
			if (left_framecount % render_every_n_frames == 0)
			{
				FTransform left_delta_transform = FTransform(left_delta_rotation, left_delta_movement, FVector(1.0f, 1.0f, 1.0f));
				left_delta_movement = FVector(0.0f, 0.0f, 0.0f);
				left_moving_object->AddActorLocalTransform(left_delta_transform);
			}
		}

		left_framecount++;

		if (total_trial_time > right_delay_time)
		{
			float right_z = right_moving_object->GetActorLocation().Z;
			if (right_z >= oscillation_max_z && right_translation_meters_per_second.Z > 0.0f)
			{
				right_translation_meters_per_second.Z = -current_velocity_magnitude;
			}
			else if (right_z <= oscillation_min_z && right_translation_meters_per_second.Z < 0.0f)
			{
				right_translation_meters_per_second.Z = current_velocity_magnitude;
			}

			FVector right_delta_translation = right_translation_meters_per_second * DeltaTime;
			FRotator right_delta_rotation = right_rotation_deg_per_second * DeltaTime;
			FTransform right_delta_transform = FTransform(right_delta_rotation, right_delta_translation, FVector(1.0f, 1.0f, 1.0f));
			right_moving_object->AddActorLocalTransform(right_delta_transform);
		}
	}

	else
	{
		UE_LOG(LogTemp, Log, TEXT("NO ACTOR FOUND"));
	}

	total_trial_time += DeltaTime;

	if (total_trial_time >= trial_length_time)
	{
		experiment_state = EXP1_ALT_EXPERIMENT_STATE::WAITING_FOR_INPUT;
		current_trial_index++;
		UE_LOG(LogTemp, Log, TEXT("Trial complete. Press spacebar for next trial. (%d remaining)"), trials.Num() - current_trial_index);
	}
}

void AExperiment1AltManager::set_actor_to_mobile(AActor* actor)
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


void AExperiment1AltManager::initialize_trials()
{
	trials.Empty();

	for (int s = 0; s < static_cast<int>(EXP1_ALT_STIMULI::COUNT); s++)
	{
		for (int lr = 0; lr < static_cast<int>(EXP1_ALT_LEFTRIGHT::COUNT); lr++)
		{
			for (int e = 0; e < 3; e++)
			{
				for (int f = 0; f < 3; f++)
				{
					for (int rc = 0; rc < 3; rc++)
					{
						Exp1AltTrial t;
						t.stimuli = static_cast<EXP1_ALT_STIMULI>(s);
						t.leftright = static_cast<EXP1_ALT_LEFTRIGHT>(lr);
						t.render_every_n_fps = render_every_n_fps_list[rc];
						t.eccentricity = eccentricities[e];
						t.frequency = frequencies[f];
						t.velocity = choose_initial_velocity_for_stimuli(target_framerate, render_every_n_fps_list[rc], frequencies[f]);
						trials.Add(t);				
					}
				}
			}
		}
	}

	// Shuffle so trials order is randomized
	for (int i = trials.Num() - 1; i > 0; i--)
	{
		int j = FMath::RandRange(0, i);
		trials.Swap(i, j);
	}

}


FVector AExperiment1AltManager::eccentricity_to_world_pos(float eccentricity_deg, EXP1_ALT_LEFTRIGHT side, float z_cm)
{
	APlayerCameraManager* camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	check(camera);

	float actual_fov = camera->GetFOVAngle();
	float half_fov_rad = FMath::DegreesToRadians(actual_fov / 2.0f);

	float x_screen_cm = distance_from_screen_cm * FMath::Tan(FMath::DegreesToRadians(eccentricity_deg));
	float ndc_x = x_screen_cm / (screen_width_cm / 2.0f);
	float lateral_cm = ndc_x * z_cm * FMath::Tan(half_fov_rad);

	UE_LOG(LogTemp, Log, TEXT("ecc=%.1f | fov=%.1f | x_screen=%.2f | screen_half=%.1f | ndc=%.3f | z=%.1f | lateral=%.2f"), eccentricity_deg, actual_fov, x_screen_cm, screen_width_cm / 2.0f, ndc_x, z_cm, lateral_cm);

	float sign = (side == EXP1_ALT_LEFTRIGHT::LEFT) ? -1.0f : 1.0f;
	FRotationMatrix rot(camera->GetCameraRotation());
	FVector forward = rot.GetUnitAxis(EAxis::X);
	FVector right = rot.GetUnitAxis(EAxis::Y);

	return camera->GetCameraLocation() + forward * z_cm + right * (sign * lateral_cm);
}

float AExperiment1AltManager::choose_initial_velocity_for_stimuli(int tgt_framerate, int every_n_fps, float freq)
{
	float velocity = 10.0f;

	// Based on table of observations from grass stimulus
	// Always choose something where it's perceptible first (whether mildly or quite so)
	if (tgt_framerate == 60)
	{
		if (freq >= 0.5f) // frequency_scale is 1
		{
			if (every_n_fps == 1) // intervention is 60fps
			{
				float randnum = FMath::RandRange(200.0f, 800.0f);
				velocity = FMath::RoundToFloat(randnum * 2.0f) / 2.0f;
			}

			else if (every_n_fps == 2) // intervention is 30fps
			{
				float randnum = FMath::RandRange(400.0f, 800.0f);
				velocity = FMath::RoundToFloat(randnum * 2.0f) / 2.0f;
			}

			else if (every_n_fps == 3) // intervention is 20fps
			{
				float randnum = FMath::RandRange(300.0f, 500.0f);
				velocity = FMath::RoundToFloat(randnum * 2.0f) / 2.0f;
			}
		}
	}

	// TODO: For 50, 40, and 30hz baseline


	return velocity;
}