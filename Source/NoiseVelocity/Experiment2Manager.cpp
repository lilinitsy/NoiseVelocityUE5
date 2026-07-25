#include "Experiment2Manager.h"

#include "Blueprint/UserWidget.h"
#include "Camera/PlayerCameraManager.h"
#include "HAL/PlatformFileManager.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"


AExperiment2Manager::AExperiment2Manager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AExperiment2Manager::BeginPlay()
{
	Super::BeginPlay();

	if (user)
	{
		user_original_transform = user->GetActorTransform();
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
			InputComponent->BindAction("Rating1", IE_Pressed, this, &AExperiment2Manager::record_noise_visibility_rating);
			InputComponent->BindAction("Rating2", IE_Pressed, this, &AExperiment2Manager::record_noise_visibility_rating);
			InputComponent->BindAction("Rating3", IE_Pressed, this, &AExperiment2Manager::record_noise_visibility_rating);
			InputComponent->BindAction("Rating4", IE_Pressed, this, &AExperiment2Manager::record_noise_visibility_rating);
			InputComponent->BindAction("Rating5", IE_Pressed, this, &AExperiment2Manager::record_noise_visibility_rating);
			InputComponent->BindAction("Rating6", IE_Pressed, this, &AExperiment2Manager::record_noise_visibility_rating);
			InputComponent->BindAction("Rating7", IE_Pressed, this, &AExperiment2Manager::record_noise_visibility_rating);
		}
	}

	if (experimentally_determine_foveation_level)
	{
		configure_foveation_test_mode();
		return;
	}

	initialize_trials();

	if (user)
	{
		user->use_movement = false;
		user->movement_velocity = 0.0f;
		user->fixation_center = fixation_uv;
		user->split_horizontally = false;
		user->region_mode = 0;
		user->update_view_extension();
		update_fixation_cross(fixation_uv);
	}

	UE_LOG(LogTemp, Log, TEXT("Experiment 2 ready. Press spacebar to start trial 1 of %d."), trials.Num());
}

void AExperiment2Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!user || experimentally_determine_foveation_level || experiment_state != EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		return;
	}

	if (user->use_eyetracking && current_trial_index < static_cast<uint32>(trials.Num()))
	{
		const FVector2f gaze = user->gaze_pos;
		const FVector2f fixation = trials[current_trial_index].fixation_uv;
		const float x_diff = gaze.X - fixation.X;
		const float y_diff = gaze.Y - fixation.Y;
		const float x_physical = x_diff * screen_width_cm;
		const float y_physical = y_diff * screen_height_cm;
		const float physical_dist = FMath::Sqrt(x_physical * x_physical + y_physical * y_physical);
		const float gaze_error_deg = FMath::RadiansToDegrees(FMath::Atan(physical_dist / distance_from_screen_cm));

		if (gaze_error_deg > max_rating_gaze_error_deg && !screen_blacked_from_gaze)
		{
			screen_blacked_from_gaze = true;
			set_screen_black(true);
		}
		else if (gaze_error_deg <= max_rating_gaze_error_deg && screen_blacked_from_gaze)
		{
			screen_blacked_from_gaze = false;
			set_screen_black(false);
		}
	}

	user->use_movement = true;
	user->movement_velocity = current_velocity_magnitude;
}

void AExperiment2Manager::initialize_trials()
{
	initialize_calibration_trials();
}

void AExperiment2Manager::initialize_calibration_trials()
{
	trials.Empty();
	saved_blur_velocities.Empty();
	noise_phase_initialized = false;
	current_phase = 1;

	const int fps_options[] = {12, 18, 24};
	const float max_eccentricity_options[] = {28.0f, 35.0f, 42.0f};

	for (int rep = 0; rep < num_repetitions; rep++)
	{
		for (int eccentricity_idx = 0; eccentricity_idx < 2; eccentricity_idx++)
		{
			for (int fps_idx = 0; fps_idx < 2; fps_idx++)
			{
				Exp2Trial no_foveation_trial;
				no_foveation_trial.phase = 1;
				no_foveation_trial.fps = fps_options[fps_idx];
				no_foveation_trial.max_eccentricity_deg = max_eccentricity_options[eccentricity_idx];
				no_foveation_trial.fixation_uv = max_eccentricity_to_fixation_uv(no_foveation_trial.max_eccentricity_deg);
				no_foveation_trial.foveation_level = EXP2_FOVEATION_LEVEL::LOW;
				no_foveation_trial.method = EXP2_METHOD::NO_FOVEATION;
				no_foveation_trial.initial_velocity = initial_velocity;
				no_foveation_trial.saved_velocity_index = -1;
				trials.Add(no_foveation_trial);

				for (int foveation_idx = 0; foveation_idx < static_cast<int>(EXP2_FOVEATION_LEVEL::COUNT); foveation_idx++)
				{
					Exp2Trial trial;
					trial.phase = 1;
					trial.fps = fps_options[fps_idx];
					trial.max_eccentricity_deg = max_eccentricity_options[eccentricity_idx];
					trial.fixation_uv = max_eccentricity_to_fixation_uv(trial.max_eccentricity_deg);
					trial.foveation_level = static_cast<EXP2_FOVEATION_LEVEL>(foveation_idx);
					trial.method = EXP2_METHOD::GAUSSIAN_BLUR;
					trial.initial_velocity = initial_velocity;
					trial.saved_velocity_index = saved_blur_velocities.Num();
					saved_blur_velocities.Add(initial_velocity);
					trials.Add(trial);
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

void AExperiment2Manager::initialize_noise_trials()
{
	TArray<Exp2Trial> phase_one_trials = trials;
	trials.Empty();
	current_trial_index = 0;
	current_phase = 2;
	noise_phase_initialized = true;

	for (int i = 0; i < phase_one_trials.Num(); i++)
	{
		const Exp2Trial& phase_one_trial = phase_one_trials[i];
		if (phase_one_trial.method != EXP2_METHOD::GAUSSIAN_BLUR ||
			phase_one_trial.saved_velocity_index < 0 ||
			phase_one_trial.saved_velocity_index >= saved_blur_velocities.Num())
		{
			continue;
		}

		Exp2Trial noise_trial;
		noise_trial.phase = 2;
		noise_trial.fps = phase_one_trial.fps;
		noise_trial.max_eccentricity_deg = phase_one_trial.max_eccentricity_deg;
		noise_trial.fixation_uv = phase_one_trial.fixation_uv;
		noise_trial.foveation_level = phase_one_trial.foveation_level;
		noise_trial.method = EXP2_METHOD::GABOR_NOISE;
		noise_trial.saved_velocity_index = phase_one_trial.saved_velocity_index;
		noise_trial.initial_velocity = saved_blur_velocities[phase_one_trial.saved_velocity_index];
		trials.Add(noise_trial);
	}

	for (int i = trials.Num() - 1; i > 0; i--)
	{
		int j = FMath::RandRange(0, i);
		trials.Swap(i, j);
	}
}

void AExperiment2Manager::on_response_recorded()
{
	if (experimentally_determine_foveation_level)
	{
		UE_LOG(LogTemp, Log, TEXT("Foveation test active. Up/Down adjusts blur_rate_arcmin_per_degree; current value %.3f."), foveation_test_blur_rate);
		return;
	}

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
			if (current_phase == 1 && !noise_phase_initialized)
			{
				initialize_noise_trials();
				if (trials.Num() > 0)
				{
					update_fixation_cross(trials[current_trial_index].fixation_uv);
					UE_LOG(LogTemp, Log, TEXT("Experiment 2 phase 1 complete. Phase 2 noise trials ready. Press spacebar to start trial 1 of %d."), trials.Num());
					return;
				}
			}

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
		UE_LOG(LogTemp, Log, TEXT("Press 1-5 to record quality rating and end this trial."));
		return;
	}

	if (experiment_state == EXP2_EXPERIMENT_STATE::RATING_NOISE_VISIBILITY)
	{
		UE_LOG(LogTemp, Log, TEXT("Press 1-5 to record quality rating and end this trial."));
		return;
	}

	if (experiment_state == EXP2_EXPERIMENT_STATE::RATING_RECORDED)
	{
		reset_user_position();
		experiment_state = EXP2_EXPERIMENT_STATE::BLACK_SCREEN;
		set_screen_black(true);
		if (current_trial_index >= static_cast<uint32>(trials.Num()))
		{
			if (current_phase == 1 && !noise_phase_initialized)
			{
				initialize_noise_trials();
				if (trials.Num() > 0)
				{
					update_fixation_cross(trials[current_trial_index].fixation_uv);
					UE_LOG(LogTemp, Log, TEXT("Experiment 2 phase 1 complete. Phase 2 noise trials ready. Press spacebar to start trial 1 of %d."), trials.Num());
					return;
				}
			}

			UE_LOG(LogTemp, Log, TEXT("Experiment 2 complete."));
			return;
		}

		update_fixation_cross(trials[current_trial_index].fixation_uv);
		UE_LOG(LogTemp, Log, TEXT("Trial complete. %d remaining."), trials.Num() - current_trial_index);
		return;
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
	current_noise_visibility_rating = 0;
	screen_blacked_from_gaze = false;
	reset_user_position();
	apply_trial(trial);
	experiment_state = EXP2_EXPERIMENT_STATE::TRIAL_RUNNING;
	FString foveation_level_str = trial.method == EXP2_METHOD::NO_FOVEATION ? TEXT("None") : foveation_level_to_string(trial.foveation_level);

	UE_LOG(LogTemp, Log, TEXT("Experiment 2 phase %d trial %d / %d: ecc=%.1f fixation_uv=(%.3f, %.3f) fps=%d level=%s method=%s initial_velocity=%.2f"),
		trial.phase,
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
		user->comparison_mode = static_cast<uint32>(EXPERIMENT2_COMPARISON_MODE::raw_hold);
	}
	else if (trial.method == EXP2_METHOD::GAUSSIAN_BLUR)
	{
		user->comparison_mode = static_cast<uint32>(EXPERIMENT2_COMPARISON_MODE::blur_hold);
	}
	else
	{
		user->comparison_mode = static_cast<uint32>(EXPERIMENT2_COMPARISON_MODE::noise);
	}

	if (user->view_extension)
	{
		user->view_extension->freeze_frame_enabled = false;
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
	if (experimentally_determine_foveation_level)
	{
		adjust_foveation_test_blur(foveation_test_blur_step);
		return;
	}

	if (experiment_state != EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		return;
	}

	current_velocity_magnitude += velocity_step;
	current_velocity_magnitude = FMath::Clamp(current_velocity_magnitude, min_velocity, max_velocity);

	if (user && experiment_state == EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		user->movement_velocity = current_velocity_magnitude;
	}
}

void AExperiment2Manager::on_decrease_velocity()
{
	if (experimentally_determine_foveation_level)
	{
		adjust_foveation_test_blur(-foveation_test_blur_step);
		return;
	}

	if (experiment_state != EXP2_EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		return;
	}

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
		if (black)
		{
			user->view_extension->freeze_frame_enabled = false;
		}

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
	FString level_name = TEXT("UnknownLevel");
	if (GetWorld())
	{
		level_name = GetWorld()->GetMapName();
		level_name.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);
	}
	level_name = FPaths::MakeValidFileName(level_name);
	FString csv_path = FPaths::ProjectDir() + FString::Printf(TEXT("experiment2_alternative_results_%s.csv"), *level_name);
	bool file_exists = FPlatformFileManager::Get().GetPlatformFile().FileExists(*csv_path);
	FString foveation_level_str = trial.method == EXP2_METHOD::NO_FOVEATION ? TEXT("None") : foveation_level_to_string(trial.foveation_level);
	float matched_blur_velocity = 0.0f;
	float velocity_gain = 0.0f;

	if (trial.saved_velocity_index >= 0 && trial.saved_velocity_index < saved_blur_velocities.Num())
	{
		matched_blur_velocity = saved_blur_velocities[trial.saved_velocity_index];
	}

	if (matched_blur_velocity > 0.0f)
	{
		velocity_gain = current_velocity_magnitude / matched_blur_velocity;
	}

	FString row = FString::Printf(TEXT("%d,%d,%.1f,%.6f,%.6f,%d,%d,%s,%s,%.2f,%.2f,%.2f,%.4f,%u\n"),
		trial.phase,
		current_trial_index,
		trial.max_eccentricity_deg,
		trial.fixation_uv.X,
		trial.fixation_uv.Y,
		trial.fps,
		fps_to_render_every_n_frames(trial.fps),
		*foveation_level_str,
		*method_to_string(trial.method),
		trial.initial_velocity,
		current_velocity_magnitude,
		matched_blur_velocity,
		velocity_gain,
		current_noise_visibility_rating);

	if (!file_exists)
	{
		FString header = TEXT("phase,trial_index,max_eccentricity_deg,fixation_uv_x,fixation_uv_y,fps,render_every_n_frames,foveation_level,method,initial_velocity,final_velocity,matched_blur_velocity,velocity_gain,noise_visibility_rating\n");
		FFileHelper::SaveStringToFile(header + row, *csv_path);
	}
	else
	{
		FFileHelper::SaveStringToFile(row, *csv_path, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
	}
}

void AExperiment2Manager::record_noise_visibility_rating()
{
	if (experimentally_determine_foveation_level ||
		(experiment_state != EXP2_EXPERIMENT_STATE::TRIAL_RUNNING &&
		 experiment_state != EXP2_EXPERIMENT_STATE::RATING_NOISE_VISIBILITY))
	{
		return;
	}

	if (!can_accept_noise_visibility_rating())
	{
		return;
	}

	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (!pc)
	{
		return;
	}

	uint32 rating = 0;
	if (pc->WasInputKeyJustPressed(EKeys::One))
	{
		rating = 1;
	}
	else if (pc->WasInputKeyJustPressed(EKeys::Two))
	{
		rating = 2;
	}
	else if (pc->WasInputKeyJustPressed(EKeys::Three))
	{
		rating = 3;
	}
	else if (pc->WasInputKeyJustPressed(EKeys::Four))
	{
		rating = 4;
	}
	else if (pc->WasInputKeyJustPressed(EKeys::Five))
	{
		rating = 5;
	}
	else if (pc->WasInputKeyJustPressed(EKeys::Six))
	{
		rating = 6;
	}
	else if (pc->WasInputKeyJustPressed(EKeys::Seven))
	{
		rating = 7;
	}

	if (rating == 0)
	{
		return;
	}

	if (user)
	{
		if (user->view_extension)
		{
			user->view_extension->freeze_frame_enabled = false;
		}

		user->use_movement = false;
		user->movement_velocity = 0.0f;
	}

	current_noise_visibility_rating = FMath::Clamp(rating, 1u, 7u);
	const Exp2Trial& trial = trials[current_trial_index];
	if (trial.phase == 1 && trial.method == EXP2_METHOD::GAUSSIAN_BLUR && trial.saved_velocity_index >= 0 && trial.saved_velocity_index < saved_blur_velocities.Num())
	{
		saved_blur_velocities[trial.saved_velocity_index] = current_velocity_magnitude;
	}

	write_trial_to_csv(trials[current_trial_index]);
	current_trial_index++;
	reset_user_position();
	screen_blacked_from_gaze = false;
	experiment_state = EXP2_EXPERIMENT_STATE::BLACK_SCREEN;
	set_screen_black(true);

	if (current_trial_index >= static_cast<uint32>(trials.Num()))
	{
		if (current_phase == 1 && !noise_phase_initialized)
		{
			initialize_noise_trials();
			if (trials.Num() > 0)
			{
				update_fixation_cross(trials[current_trial_index].fixation_uv);
				UE_LOG(LogTemp, Log, TEXT("Recorded quality rating %u. Experiment 2 phase 1 complete. Phase 2 noise trials ready. Press spacebar to start trial 1 of %d."), current_noise_visibility_rating, trials.Num());
				return;
			}
		}

		UE_LOG(LogTemp, Log, TEXT("Recorded quality rating %u. Experiment 2 complete."), current_noise_visibility_rating);
		return;
	}

	update_fixation_cross(trials[current_trial_index].fixation_uv);

	UE_LOG(LogTemp, Log, TEXT("Recorded quality rating %u. Trial complete. Press spacebar to start the next trial. %d remaining."), current_noise_visibility_rating, trials.Num() - current_trial_index);
}

bool AExperiment2Manager::can_accept_noise_visibility_rating() const
{
	if (!user || !user->use_eyetracking)
	{
		return true;
	}

	if (current_trial_index >= static_cast<uint32>(trials.Num()))
	{
		return false;
	}

	const FVector2f gaze = user->gaze_pos;
	const FVector2f fixation = trials[current_trial_index].fixation_uv;
	const float x_diff = gaze.X - fixation.X;
	const float y_diff = gaze.Y - fixation.Y;
	const float x_physical = x_diff * screen_width_cm;
	const float y_physical = y_diff * screen_height_cm;
	const float physical_dist = FMath::Sqrt(x_physical * x_physical + y_physical * y_physical);
	const float gaze_error_deg = FMath::RadiansToDegrees(FMath::Atan(physical_dist / distance_from_screen_cm));

	if (gaze_error_deg > max_rating_gaze_error_deg)
	{
		UE_LOG(LogTemp, Log, TEXT("Rating rejected: gaze is %.2f deg from fixation cross; max allowed is %.2f deg."), gaze_error_deg, max_rating_gaze_error_deg);
		return false;
	}

	return true;
}

void AExperiment2Manager::configure_foveation_test_mode()
{
	if (!user)
	{
		UE_LOG(LogTemp, Warning, TEXT("Foveation test mode requires a user assigned."));
		return;
	}

	trials.Empty();
	experiment_state = EXP2_EXPERIMENT_STATE::WAITING_FOR_INPUT;
	current_velocity_magnitude = 0.0f;
	foveation_test_blur_rate = FMath::Clamp(foveation_test_blur_rate, foveation_test_min_blur, foveation_test_max_blur);

	user->use_movement = false;
	user->movement_velocity = 0.0f;
	user->fixation_center = FVector2f(0.5f, 0.5f);
	user->split_horizontally = false;
	user->region_mode = 0;
	user->render_every_n_frames = 1;
	user->comparison_mode = static_cast<uint32>(EXPERIMENT2_COMPARISON_MODE::blur_hold);
	user->use_radially_increasing_blur = 0;

	apply_foveation_test_blur();
	update_fixation_cross(user->fixation_center);

	UE_LOG(LogTemp, Log, TEXT("Foveation test mode active. Up/Down adjusts blur_rate_arcmin_per_degree by %.3f."), foveation_test_blur_step);
}

void AExperiment2Manager::apply_foveation_test_blur()
{
	if (!user)
	{
		return;
	}

	user->blur_rate_arcmin_per_degree = foveation_test_blur_rate;

	if (user->view_extension)
	{
		user->view_extension->reset_cache_requested = true;
	}

	user->update_view_extension();

	UE_LOG(LogTemp, Log, TEXT("Foveation test blur_rate_arcmin_per_degree = %.3f"), foveation_test_blur_rate);
}

void AExperiment2Manager::adjust_foveation_test_blur(float delta)
{
	foveation_test_blur_rate = FMath::Clamp(foveation_test_blur_rate + delta, foveation_test_min_blur, foveation_test_max_blur);
	apply_foveation_test_blur();
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
