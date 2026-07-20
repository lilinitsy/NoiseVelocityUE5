#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/Actor.h"

#include "GaborEnhanceWithRerenderTestChar.h"

#include "Experiment2Manager.generated.h"

enum class EXP2_METHOD
{
	NO_FOVEATION,
	GAUSSIAN_BLUR,
	GABOR_NOISE,
	COUNT
};

enum class EXP2_FOVEATION_LEVEL
{
	LOW,
	HIGH,
	COUNT
};

enum class EXP2_EXPERIMENT_STATE
{
	WAITING_FOR_INPUT,
	BLACK_SCREEN,
	TRIAL_RUNNING,
	RATING_NOISE_VISIBILITY,
	RATING_RECORDED
};

struct Exp2Trial
{
	int phase;
	int fps;
	float max_eccentricity_deg;
	FVector2f fixation_uv;
	EXP2_FOVEATION_LEVEL foveation_level;
	EXP2_METHOD method;
	float initial_velocity;
	int saved_velocity_index;
};

UCLASS()
class NOISEVELOCITY_API AExperiment2Manager : public AActor
{
	GENERATED_BODY()

public:
	AExperiment2Manager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "User Settings")
	AGaborEnhanceWithRerenderTestChar* user;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	int target_framerate = 60;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	int num_repetitions = 1;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	FVector2f fixation_uv = FVector2f(0.05f, 0.5f);

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float screen_width_cm = 60.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float screen_height_cm = 33.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float distance_from_screen_cm = 60.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float initial_velocity = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float velocity_step = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float min_velocity = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float max_velocity = 1200.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	float max_rating_gaze_error_deg = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	bool use_custom_reset_transform = false;

	UPROPERTY(EditAnywhere, Category = "Foveation Test")
	bool experimentally_determine_foveation_level = false;

	UPROPERTY(EditAnywhere, Category = "Foveation Test")
	float foveation_test_blur_rate = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Foveation Test")
	float foveation_test_blur_step = 0.01f;

	UPROPERTY(EditAnywhere, Category = "Foveation Test")
	float foveation_test_min_blur = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Foveation Test")
	float foveation_test_max_blur = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	FVector reset_location = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Experiment 2")
	FRotator reset_rotation = FRotator(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Foveation Levels")
	float low_frequency_scale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Foveation Levels")
	float high_frequency_scale = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Foveation Levels")
	float low_blur_rate_arcmin_per_degree = 0.17f;

	UPROPERTY(EditAnywhere, Category = "Foveation Levels")
	float high_blur_rate_arcmin_per_degree = 0.25f;

	uint32 current_trial_index = 0;
	float current_velocity_magnitude = 0.0f;
	uint32 current_noise_visibility_rating = 0;
	EXP2_EXPERIMENT_STATE experiment_state = EXP2_EXPERIMENT_STATE::WAITING_FOR_INPUT;
	int current_phase = 1;
	FTransform user_original_transform;
	TArray<Exp2Trial> trials;
	TArray<float> saved_blur_velocities;
	bool noise_phase_initialized = false;
	bool screen_blacked_from_gaze = false;

	void initialize_trials();
	void initialize_calibration_trials();
	void initialize_noise_trials();
	void start_trial();
	void apply_trial(const Exp2Trial& trial);
	void apply_foveation_level(EXP2_FOVEATION_LEVEL foveation_level);
	void on_response_recorded();
	void on_increase_velocity();
	void on_decrease_velocity();
	void set_screen_black(bool black);
	void reset_user_position();
	void write_trial_to_csv(const Exp2Trial& trial);
	void record_noise_visibility_rating();
	bool can_accept_noise_visibility_rating() const;
	void configure_foveation_test_mode();
	void apply_foveation_test_blur();
	void adjust_foveation_test_blur(float delta);

	FVector2f max_eccentricity_to_fixation_uv(float max_eccentricity_deg) const;
	void update_fixation_cross(FVector2f trial_fixation_uv);
	float velocity_decrement() const;
	uint32 fps_to_render_every_n_frames(int fps) const;
	FString method_to_string(EXP2_METHOD method) const;
	FString foveation_level_to_string(EXP2_FOVEATION_LEVEL foveation_level) const;
};
