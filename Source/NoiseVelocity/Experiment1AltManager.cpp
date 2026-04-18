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
			
			// Debugging
			InputComponent->BindAction("MoveObjectLeft", IE_Pressed, this, &AExperiment1AltManager::move_object_left);
			InputComponent->BindAction("MoveObjectRight", IE_Pressed, this, &AExperiment1AltManager::move_object_right);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Experiment ready. Press spacebar to start trial 1 of %d."), trials.Num());
}


void AExperiment1AltManager::set_screen_black(bool black)
{
	user->view_extension->is_active = !black;

	APlayerCameraManager* camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if (!camera)
	{
		return;
	}
	if (black)
	{
		camera->StartCameraFade(0.0f, 1.0f, 0.1f, FLinearColor::Black, false, true);
	}

	else
	{
		camera->StartCameraFade(1.0f, 0.0f, 0.1f, FLinearColor::Black, false, false);
	}
}


void AExperiment1AltManager::on_response_recorded()
{
	UE_LOG(LogTemp, Log, TEXT("on_response_recorded triggered"));

	if (experiment_state == EXP1_ALT_EXPERIMENT_STATE::WAITING_FOR_INPUT)
	{
		experiment_state = EXP1_ALT_EXPERIMENT_STATE::BLACK_SCREEN;
		set_screen_black(true);
	}
	else if (experiment_state == EXP1_ALT_EXPERIMENT_STATE::BLACK_SCREEN)
	{
		set_screen_black(false);
		start_trial();
	}
	else if (experiment_state == EXP1_ALT_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		write_trial_to_csv(trials[current_trial_index]);
		current_trial_index++;
		UE_LOG(LogTemp, Log, TEXT("Trial complete. (%d remaining)"), trials.Num() - current_trial_index);
		experiment_state = EXP1_ALT_EXPERIMENT_STATE::BLACK_SCREEN;
		set_screen_black(true);
	}
}

void AExperiment1AltManager::on_increase_velocity()
{
	current_velocity_magnitude += 50.0f;
	current_velocity_magnitude = FMath::Clamp(current_velocity_magnitude, 50.0f, 1200.0f);
	left_translation_meters_per_second.Z = FMath::Sign(left_translation_meters_per_second.Z) == 0 ? current_velocity_magnitude : FMath::Sign(left_translation_meters_per_second.Z) * current_velocity_magnitude;
	right_translation_meters_per_second.Z = FMath::Sign(right_translation_meters_per_second.Z) == 0 ? -current_velocity_magnitude : FMath::Sign(right_translation_meters_per_second.Z) * current_velocity_magnitude;
}

void AExperiment1AltManager::on_decrease_velocity()
{
	current_velocity_magnitude -= 50.0f;
	current_velocity_magnitude = FMath::Clamp(current_velocity_magnitude, 50.0f, 1200.0f);
	left_translation_meters_per_second.Z = FMath::Sign(left_translation_meters_per_second.Z) == 0 ? current_velocity_magnitude : FMath::Sign(left_translation_meters_per_second.Z) * current_velocity_magnitude;
	right_translation_meters_per_second.Z = FMath::Sign(right_translation_meters_per_second.Z) == 0 ? -current_velocity_magnitude : FMath::Sign(right_translation_meters_per_second.Z) * current_velocity_magnitude;
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
	float z_distance = FMath::Abs(user->GetActorLocation().X - left_moving_object->GetActorLocation().X);

	FVector left_location = eccentricity_to_world_pos(trial.eccentricity, trial.leftright, z_distance);
	FVector right_location = left_location;
	right_location.Y *= -1.0f; // flip the y (leftright) axis
	left_moving_object->SetActorLocation(left_location);
	right_moving_object->SetActorLocation(right_location);
	current_velocity_magnitude = trial.velocity;
	left_translation_meters_per_second.Z = current_velocity_magnitude;
	right_translation_meters_per_second.Z = -current_velocity_magnitude; // other way than left
	
	
	// Assign new, and relevant, camera properties, update stimuli
	// gabor noise -> comparison_mode = 1
	// gaussian blur -> comparison_mode = 2
	if (trial.condition == EXP1_ALT_CONDITION::GABOR_NOISE)
	{
		user->comparison_mode = 1;
	}

	else if (trial.condition == EXP1_ALT_CONDITION::GAUSSIAN_BLUR)
	{
		user->comparison_mode = 2;
	}

	render_every_n_frames = trial.render_every_n_fps;
	user->render_every_n_frames = trial.render_every_n_fps;
	user->region_mode = (trial.leftright == EXP1_ALT_LEFTRIGHT::LEFT) ? 1 : 2; // set it to left (1) or right (2)
	user->frequency_scale = trial.frequency;
	user->phase_cycles_per_sec = trial.render_every_n_fps; // this looks more aesthetic and makes sense to cycle noise
	user->update_view_extension();
	apply_material_for_stimuli(trial.stimuli);

	// Update relevant experiment parameters
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
			FVector loc = left_moving_object->GetActorLocation();
			loc.Z = oscillation_max_z;
			left_moving_object->SetActorLocation(loc);
		}

		else if (left_z <= oscillation_min_z && left_translation_meters_per_second.Z < 0.0f)
		{
			left_translation_meters_per_second.Z = current_velocity_magnitude;
			FVector loc = left_moving_object->GetActorLocation();
			loc.Z = oscillation_min_z;
			left_moving_object->SetActorLocation(loc);
		}

		FVector left_delta_translation = left_translation_meters_per_second * DeltaTime;
		FRotator left_delta_rotation = left_rotation_deg_per_second * DeltaTime;
		left_delta_movement += left_delta_translation;
		if (render_same_fps)
		{
			left_moving_object->AddActorLocalRotation(left_delta_rotation);
			left_moving_object->AddActorWorldOffset(left_delta_translation);
		}

		else
		{
			// bullshit block to avoid issues in shader...
			// mod by 1 for gabor noise so that the motion vectors are still generated,
			// but the shader reuse keeps non-noise from visibly moving.
			// Mod by render_every_n_frames for blur so that it doesn't always move smoothly.
			if (trials[current_trial_index].condition == EXP1_ALT_CONDITION::GABOR_NOISE)
			{
				// THIS WAS THE ISSUE CAUSING MOTION VECTORS NOT TO RENDER
				// BECAUSE THE CUBE WAS NOT BEING MOVED
				//if (left_framecount % render_every_n_frames == 0)
				if (left_framecount % 1 == 0)
				{
					left_moving_object->AddActorLocalRotation(left_delta_rotation);
					left_moving_object->AddActorWorldOffset(left_delta_movement);
					left_delta_movement = FVector(0.0f, 0.0f, 0.0f);
				}
			}

			else
			{
				if (left_framecount % render_every_n_frames == 0)
				{
					left_moving_object->AddActorLocalRotation(left_delta_rotation);
					left_moving_object->AddActorWorldOffset(left_delta_movement);
					left_delta_movement = FVector(0.0f, 0.0f, 0.0f);
				}
			}

		}

		left_framecount++;

		float right_z = right_moving_object->GetActorLocation().Z;
		if (right_z >= oscillation_max_z && right_translation_meters_per_second.Z > 0.0f)
		{
			right_translation_meters_per_second.Z = -current_velocity_magnitude;
			FVector loc = right_moving_object->GetActorLocation();
			loc.Z = oscillation_max_z;
			right_moving_object->SetActorLocation(loc);
		}

		else if (right_z <= oscillation_min_z && right_translation_meters_per_second.Z < 0.0f)
		{
			right_translation_meters_per_second.Z = current_velocity_magnitude;
			FVector loc = right_moving_object->GetActorLocation();
			loc.Z = oscillation_min_z;
			right_moving_object->SetActorLocation(loc);
		}

		FVector right_delta_translation = right_translation_meters_per_second * DeltaTime;
		FRotator right_delta_rotation = right_rotation_deg_per_second * DeltaTime;
		FTransform right_delta_transform = FTransform(right_delta_rotation, right_delta_translation, FVector(1.0f, 1.0f, 1.0f));
		right_moving_object->AddActorLocalRotation(right_delta_rotation);
		right_moving_object->AddActorWorldOffset(right_delta_translation);
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("NO ACTOR FOUND"));
	}


	// gaze monitoring
	if (user->use_eyetracking && experiment_state == EXP1_ALT_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		FVector2f gaze = user->gaze_pos;

		// Convert gaze UV to eccentricity degrees using same math as shader
		float x_diff = gaze.X - 0.5f;
		float y_diff = gaze.Y - 0.5f;
		float x_physical = x_diff * screen_width_cm;
		float y_physical = y_diff * screen_height_cm;
		float physical_dist = FMath::Sqrt(x_physical * x_physical + y_physical * y_physical);
		float gaze_eccentricity = FMath::RadiansToDegrees(FMath::Atan(physical_dist / distance_from_screen_cm));

		if (gaze_eccentricity > max_gaze_eccentricity_deg && !screen_blacked_from_gaze)
		{
			screen_blacked_from_gaze = true;
			set_screen_black(true);
		}
		else if (gaze_eccentricity <= max_gaze_eccentricity_deg && screen_blacked_from_gaze)
		{
			screen_blacked_from_gaze = false;
			set_screen_black(false);
		}
	}


	total_trial_time += DeltaTime;
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
				for (int f = 0; f < 2; f++)
				{
					for (int rc = 0; rc < 3; rc++)
					{
						for (int condition = 0; condition < static_cast<int>(EXP1_ALT_CONDITION::COUNT); condition++)
						{
							Exp1AltTrial t;
							t.stimuli = static_cast<EXP1_ALT_STIMULI>(s);
							t.condition = static_cast<EXP1_ALT_CONDITION>(condition);
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
	// THE COMMENTED OUT CODE IS WRONG. USE THE EXPERIMENTAL VALUES BELOW

	/*APlayerCameraManager* camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
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

	return camera->GetCameraLocation() + forward * z_cm + right * (sign * lateral_cm);*/

	APlayerCameraManager* camera = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	check(camera);
	FRotationMatrix rot(camera->GetCameraRotation());
	FVector forward = rot.GetUnitAxis(EAxis::X);
	float sign = (side == EXP1_ALT_LEFTRIGHT::LEFT) ? -1.0f : 1.0f;

	FVector position = camera->GetCameraLocation() + forward * z_cm;;

	// Determine values from testing
	if (eccentricity_deg >= 29.9f) // 30
	{
		position.Y = sign * 142.0f; 
	}

	else if (eccentricity_deg >= 23.9f) // 24
	{
		position.Y = sign * 112.0f;
	}

	else if (eccentricity_deg >= 17.9f) // 18
	{
		position.Y = sign * 82.0f;
	}

	return position;
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
				float randnum = FMath::RandRange(2.0f, 6.0f);
				velocity = FMath::RoundToFloat(randnum * 2.0f) / 2.0f;
				velocity *= 100.0f; // this order keeps it in units of 50's
			}

			else if (every_n_fps == 2) // intervention is 30fps
			{
				float randnum = FMath::RandRange(5.0f, 6.0f);
				velocity = FMath::RoundToFloat(randnum * 2.0f) / 2.0f;
				velocity *= 100.0f;
			}

			else if (every_n_fps == 3) // intervention is 20fps
			{
				float randnum = FMath::RandRange(5.0f, 6.0f);
				velocity = FMath::RoundToFloat(randnum * 2.0f) / 2.0f;
				velocity *= 100.0f;
			}

			else if (every_n_fps == 4) // intervention is 15fps
			{
				float randnum = FMath::RandRange(3.0f, 4.0f);
				velocity = FMath::RoundToFloat(randnum * 2.0f) / 2.0f;
				velocity *= 100.0f;
			}

		}
	}

	// TODO: For 50, 40, and 30hz baseline


	return velocity;
}

void AExperiment1AltManager::apply_material_for_stimuli(EXP1_ALT_STIMULI stimuli)
{
	UMaterialInterface *mat = (stimuli == EXP1_ALT_STIMULI::STIMULI0) ? stimuli0_material : stimuli1_material;
	if (!mat)
	{
		return;
	}

	TArray<UStaticMeshComponent*> left_mesh_components;
	left_moving_object->GetComponents<UStaticMeshComponent>(left_mesh_components);
	for (UStaticMeshComponent* mesh : left_mesh_components)
	{
		for (int32 i = 0; i < mesh->GetNumMaterials(); i++)
		{
			mesh->SetMaterial(i, mat);
		}
	}

	TArray<UStaticMeshComponent*> right_mesh_components;
	right_moving_object->GetComponents<UStaticMeshComponent>(right_mesh_components);
	for (UStaticMeshComponent* mesh : right_mesh_components)
	{
		for (int32 i = 0; i < mesh->GetNumMaterials(); i++)
		{
			mesh->SetMaterial(i, mat);
		}
	}
}


// Debug positions because the screen-space eccentricity is wrong.
void AExperiment1AltManager::move_object_left()
{
	FVector left_object_position = left_moving_object->GetActorLocation();
	left_object_position.Y -= 5.0f;
	left_moving_object->SetActorLocation(left_object_position);
	UE_LOG(LogTemp, Log, TEXT("Left Y position: %f"), left_object_position.Y);

	UStaticMeshComponent* mesh = left_moving_object->FindComponentByClass<UStaticMeshComponent>();
	if (mesh)
	{
		FVector boundsOrigin, boundsExtent;
		float sphereRadius;
		UKismetSystemLibrary::GetComponentBounds(mesh, boundsOrigin, boundsExtent, sphereRadius);

		UE_LOG(LogTemp, Log,
			TEXT("ActorY=%f MeshCompY=%f BoundsOriginY=%f RelY=%f ExtentY=%f"),
			left_moving_object->GetActorLocation().Y,
			mesh->GetComponentLocation().Y,
			boundsOrigin.Y,
			mesh->GetRelativeLocation().Y,
			boundsExtent.Y
		);
	}
}

void AExperiment1AltManager::move_object_right()
{
	FVector left_object_position = left_moving_object->GetActorLocation();
	left_object_position.Y += 5.0f;
	left_moving_object->SetActorLocation(left_object_position);
	UE_LOG(LogTemp, Log, TEXT("Left Y position: %f"), left_object_position.Y);
}


void AExperiment1AltManager::write_trial_to_csv(const Exp1AltTrial& trial)
{
	FString csv_path = FPaths::ProjectDir() + TEXT("trial_results.csv");

	bool file_exists = FPlatformFileManager::Get().GetPlatformFile().FileExists(*csv_path);

	FString condition_str = trial.condition == EXP1_ALT_CONDITION::GABOR_NOISE ? "Noise" : "Blur";

	FString row = FString::Printf(TEXT("%d, %s, %d, %d, %.3f, %d, %d, %.2f, %.2f\n"),
		current_trial_index,
		*condition_str,
		static_cast<int>(trial.stimuli),
		trial.eccentricity,
		trial.frequency,
		static_cast<int>(trial.leftright),
		trial.render_every_n_fps,
		trial.velocity,
		current_velocity_magnitude);

	if (!file_exists)
	{
		FString header = TEXT("index, render_condition, stimuli, eccentricity, frequency, leftright, render_every_n_fps, initial_velocity, final_velocity\n");
		FFileHelper::SaveStringToFile(header + row, *csv_path);
	}
	else
	{
		FFileHelper::SaveStringToFile(row, *csv_path, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}
}
