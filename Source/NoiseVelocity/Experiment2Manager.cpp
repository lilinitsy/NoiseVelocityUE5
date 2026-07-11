#include "Experiment2Manager.h"

#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/PlatformFileManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

enum class experiment2ComparisonMode : uint32
{
	noise = 1,
	raw_hold = 3,
	blur_hold = 4
};

AExperiment2Manager::AExperiment2Manager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AExperiment2Manager::BeginPlay()
{
	Super::BeginPlay();

	initialize_trials();

	if (user)
	{
		user_original_transform = user->GetActorTransform();
		user->use_movement = false;
		user->movement_velocity = 0.0f;
		user->fixation_center = fixation_uv;
		user->split_horizontally = false;
		user->region_mode = 0;
		user->update_view_extension();
		update_fixation_cross(fixation_uv);
	}

	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc)
	{
		EnableInput(pc);
		if (InputComponent)
		{
			InputComponent->BindAction("RecordResponse", IE_Pressed, this, &AExperiment2Manager::on_response_recorded);
			InputComponent->BindAction("IncreaseVelocity", IE_Pressed, this, &AExperiment2Manager::on_increase_velocity);
			InputComponent->BindAction("DecreaseVelocity", IE_Pressed, this, &AExperiment2Manager::on_decrease_velocity);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Experiment 2 ready. Press spacebar to start trial 1 of %d."), trials.Num());
}

void AExperiment2Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!user || experiment_state != EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		return;
	}

	user->use_movement = true;
	user->movement_velocity = current_velocity_magnitude;
}

void AExperiment2Manager::initialize_trials()
{
	trials.Empty();

	const int fps_options[] = {10, 20, 30, 60};
	const float max_eccentricity_options[] = {28.0f, 35.0f, 42.0f};

	for (int rep = 0; rep < num_repetitions; rep++)
	{
		for (int eccentricity_idx = 0; eccentricity_idx < 3; eccentricity_idx++)
		{
			for (int fps_idx = 0; fps_idx < 4; fps_idx++)
			{
				Exp2Trial no_foveation_trial;
				no_foveation_trial.fps = fps_options[fps_idx];
				no_foveation_trial.max_eccentricity_deg = max_eccentricity_options[eccentricity_idx];
				no_foveation_trial.fixation_uv = max_eccentricity_to_fixation_uv(no_foveation_trial.max_eccentricity_deg);
				no_foveation_trial.foveation_level = EXP2_FOVEATION_LEVEL::LOW;
				no_foveation_trial.method = EXP2_METHOD::NO_FOVEATION;
				no_foveation_trial.initial_velocity = initial_velocity;
				trials.Add(no_foveation_trial);

				for (int foveation_idx = 0; foveation_idx < static_cast<int>(EXP2_FOVEATION_LEVEL::COUNT); foveation_idx++)
				{
					for (int method_idx = static_cast<int>(EXP2_METHOD::GAUSSIAN_BLUR); method_idx < static_cast<int>(EXP2_METHOD::COUNT); method_idx++)
					{
						Exp2Trial trial;
						trial.fps = fps_options[fps_idx];
						trial.max_eccentricity_deg = max_eccentricity_options[eccentricity_idx];
						trial.fixation_uv = max_eccentricity_to_fixation_uv(trial.max_eccentricity_deg);
						trial.foveation_level = static_cast<EXP2_FOVEATION_LEVEL>(foveation_idx);
						trial.method = static_cast<EXP2_METHOD>(method_idx);
						trial.initial_velocity = initial_velocity;
						trials.Add(trial);
					}
				}
			}
		}
	}

	for (int i = trials.Num() - 1; i > 0; i--)
	{
		int j = FMath::RandRange(0, i);
		trials.Swap(i, j);
	}
}

void AExperiment2Manager::on_response_recorded()
{
	if (!user)
	{
		UE_LOG(LogTemp, Warning, TEXT("Experiment 2 has no user assigned."));
		return;
	}

	if (experiment_state == EXP2_EXPERIMENT_STATE::WAITING_FOR_INPUT)
	{
		experiment_state = EXP2_EXPERIMENT_STATE::BLACK_SCREEN;
		set_screen_black(true);
		return;
	}

	if (experiment_state == EXP2_EXPERIMENT_STATE::BLACK_SCREEN)
	{
		if (current_trial_index >= static_cast<uint32>(trials.Num()))
		{
			set_screen_black(true);
			UE_LOG(LogTemp, Log, TEXT("All Experiment 2 trials complete."));
			return;
		}

		set_screen_black(false);
		start_trial();
		return;
	}

	if (experiment_state == EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		write_trial_to_csv(trials[current_trial_index]);
		current_trial_index++;
		user->use_movement = false;
		user->movement_velocity = 0.0f;
		reset_user_position();
		experiment_state = EXP2_EXPERIMENT_STATE::BLACK_SCREEN;
		set_screen_black(true);

		if (current_trial_index >= static_cast<uint32>(trials.Num()))
		{
			UE_LOG(LogTemp, Log, TEXT("Experiment 2 complete."));
			return;
		}

		update_fixation_cross(trials[current_trial_index].fixation_uv);
		UE_LOG(LogTemp, Log, TEXT("Trial complete. %d remaining."), trials.Num() - current_trial_index);
	}
}

void AExperiment2Manager::start_trial()
{
	if (current_trial_index >= static_cast<uint32>(trials.Num()))
	{
		UE_LOG(LogTemp, Log, TEXT("All Experiment 2 trials complete."));
		return;
	}

	const Exp2Trial& trial = trials[current_trial_index];
	current_velocity_magnitude = trial.initial_velocity;
	reset_user_position();
	apply_trial(trial);
	experiment_state = EXP2_EXPERIMENT_STATE::TRIAL_RUNNING;
	FString foveation_level_str = trial.method == EXP2_METHOD::NO_FOVEATION ? TEXT("None") : foveation_level_to_string(trial.foveation_level);

	UE_LOG(LogTemp, Log, TEXT("Experiment 2 trial %d / %d: ecc=%.1f fixation_uv=(%.3f, %.3f) fps=%d level=%s method=%s initial_velocity=%.2f"),
		current_trial_index + 1,
		trials.Num(),
		trial.max_eccentricity_deg,
		trial.fixation_uv.X,
		trial.fixation_uv.Y,
		trial.fps,
		*foveation_level_str,
		*method_to_string(trial.method),
		trial.initial_velocity);
}

void AExperiment2Manager::apply_trial(const Exp2Trial& trial)
{
	user->fixation_center = trial.fixation_uv;
	user->split_horizontally = false;
	user->region_mode = 0;
	user->render_every_n_frames = fps_to_render_every_n_frames(trial.fps);
	user->use_movement = true;
	user->movement_velocity = current_velocity_magnitude;
	update_fixation_cross(trial.fixation_uv);

	apply_foveation_level(trial.foveation_level);

	if (trial.method == EXP2_METHOD::NO_FOVEATION)
	{
		user->comparison_mode = static_cast<uint32>(experiment2ComparisonMode::raw_hold);
	}
	else if (trial.method == EXP2_METHOD::GAUSSIAN_BLUR)
	{
		user->comparison_mode = static_cast<uint32>(experiment2ComparisonMode::blur_hold);
	}
	else
	{
		user->comparison_mode = static_cast<uint32>(experiment2ComparisonMode::noise);
	}

	if (user->view_extension)
	{
		user->view_extension->reset_cache_requested = true;
	}

	user->update_view_extension();
}

void AExperiment2Manager::apply_foveation_level(EXP2_FOVEATION_LEVEL foveation_level)
{
	if (foveation_level == EXP2_FOVEATION_LEVEL::LOW)
	{
		user->frequency_scale = low_frequency_scale;
		user->blur_rate_arcmin_per_degree = low_blur_rate_arcmin_per_degree;
	}
	else
	{
		user->frequency_scale = high_frequency_scale;
		user->blur_rate_arcmin_per_degree = high_blur_rate_arcmin_per_degree;
	}
}

void AExperiment2Manager::on_increase_velocity()
{
	current_velocity_magnitude += velocity_step;
	current_velocity_magnitude = FMath::Clamp(current_velocity_magnitude, min_velocity, max_velocity);

	if (user && experiment_state == EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		user->movement_velocity = current_velocity_magnitude;
	}
}

void AExperiment2Manager::on_decrease_velocity()
{
	current_velocity_magnitude -= velocity_decrement();
	current_velocity_magnitude = FMath::Clamp(current_velocity_magnitude, min_velocity, max_velocity);

	if (user && experiment_state == EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		user->movement_velocity = current_velocity_magnitude;
	}
}

void AExperiment2Manager::set_screen_black(bool black)
{
	if (user && user->view_extension)
	{
		user->view_extension->is_active = !black;
	}

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

void AExperiment2Manager::reset_user_position()
{
	if (!user)
	{
		return;
	}

	if (use_custom_reset_transform)
	{
		user->SetActorLocationAndRotation(reset_location, reset_rotation);
	}
	else
	{
		user->SetActorTransform(user_original_transform);
	}
}

void AExperiment2Manager::write_trial_to_csv(const Exp2Trial& trial)
{
	FString csv_path = FPaths::ProjectDir() + TEXT("experiment2_results.csv");
	bool file_exists = FPlatformFileManager::Get().GetPlatformFile().FileExists(*csv_path);
	FString foveation_level_str = trial.method == EXP2_METHOD::NO_FOVEATION ? TEXT("None") : foveation_level_to_string(trial.foveation_level);

	FString row = FString::Printf(TEXT("%d,%.1f,%.6f,%.6f,%d,%d,%s,%s,%.2f,%.2f\n"),
		current_trial_index,
		trial.max_eccentricity_deg,
		trial.fixation_uv.X,
		trial.fixation_uv.Y,
		trial.fps,
		fps_to_render_every_n_frames(trial.fps),
		*foveation_level_str,
		*method_to_string(trial.method),
		trial.initial_velocity,
		current_velocity_magnitude);

	if (!file_exists)
	{
		FString header = TEXT("trial_index,max_eccentricity_deg,fixation_uv_x,fixation_uv_y,fps,render_every_n_frames,foveation_level,method,initial_velocity,final_velocity\n");
		FFileHelper::SaveStringToFile(header + row, *csv_path);
	}
	else
	{
		FFileHelper::SaveStringToFile(row, *csv_path, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}
}

FVector2f AExperiment2Manager::max_eccentricity_to_fixation_uv(float max_eccentricity_deg) const
{
	float offset_cm = distance_from_screen_cm * FMath::Tan(FMath::DegreesToRadians(max_eccentricity_deg));
	float fixation_x = 1.0f - (offset_cm / screen_width_cm);
	return FVector2f(fixation_x, 0.5f);
}

void AExperiment2Manager::update_fixation_cross(FVector2f trial_fixation_uv)
{
	if (!user || !GEngine || !GEngine->GameViewport)
	{
		return;
	}

	if (!user->fixation_cross_widget)
	{
		if (!user->fixation_cross_widget_class)
		{
			UE_LOG(LogTemp, Warning, TEXT("Experiment 2 requires fixation_cross_widget_class; no fallback fixation cross will be created."));
			return;
		}

		user->fixation_cross_widget = CreateWidget<UUserWidget>(GetWorld(), user->fixation_cross_widget_class);
		if (user->fixation_cross_widget)
		{
			user->fixation_cross_widget->AddToViewport(999);
		}
	}

	if (!user->fixation_cross_widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Experiment 2 failed to create fixation_cross_widget from fixation_cross_widget_class."));
		return;
	}

	FVector2D viewport_size;
	GEngine->GameViewport->GetViewportSize(viewport_size);
	user->fixation_cross_widget->SetVisibility(ESlateVisibility::HitTestInvisible);
	user->fixation_cross_widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	user->fixation_cross_widget->SetDesiredSizeInViewport(FVector2D(32.0f, 32.0f));
	user->fixation_cross_widget->SetPositionInViewport(FVector2D(trial_fixation_uv.X * viewport_size.X, trial_fixation_uv.Y * viewport_size.Y), true);
}

float AExperiment2Manager::velocity_decrement() const
{
	if (current_velocity_magnitude < 30.0f)
	{
		return 5.0f;
	}

	if (current_velocity_magnitude < 50.0f)
	{
		return 10.0f;
	}

	return velocity_step;
}

uint32 AExperiment2Manager::fps_to_render_every_n_frames(int fps) const
{
	if (fps <= 0)
	{
		return 1;
	}

	return FMath::Max(1, FMath::RoundToInt(static_cast<float>(target_framerate) / static_cast<float>(fps)));
}

FString AExperiment2Manager::method_to_string(EXP2_METHOD method) const
{
	if (method == EXP2_METHOD::NO_FOVEATION)
	{
		return TEXT("NoFoveation");
	}

	if (method == EXP2_METHOD::GAUSSIAN_BLUR)
	{
		return TEXT("Blur");
	}

	return TEXT("NoiseAdaptive");
}

FString AExperiment2Manager::foveation_level_to_string(EXP2_FOVEATION_LEVEL foveation_level) const
{
	if (foveation_level == EXP2_FOVEATION_LEVEL::LOW)
	{
		return TEXT("Low");
	}

	return TEXT("High");
}
