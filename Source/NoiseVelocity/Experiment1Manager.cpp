// Fill out your copyright notice in the Description page of Project Settings.


#include "Experiment1Manager.h"


// Sets default values
AExperiment1Manager::AExperiment1Manager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AExperiment1Manager::BeginPlay()
{
	Super::BeginPlay();

	set_actor_to_mobile(left_moving_object);
	set_actor_to_mobile(right_moving_object);

	initialize_trials();

	APlayerController* pc = GetWorld()->GetFirstPlayerController();
	if (pc)
	{
		UE_LOG(LogTemp, Log, TEXT("plaercontroller hit"));
		EnableInput(pc);
		if (InputComponent)
		{
			UE_LOG(LogTemp, Log, TEXT("input component found"));
			InputComponent->BindAction("RecordResponse", IE_Pressed, this, &AExperiment1Manager::on_response_recorded);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Experiment ready. Press spacebar to start trial 1 of %d."), trials.Num());

}



void AExperiment1Manager::on_response_recorded()
{
	UE_LOG(LogTemp, Log, TEXT("on_response_recorded triggered"));
	if (experiment_state == EXPERIMENT_STATE::WAITING_FOR_INPUT)
	{
		start_trial();
	}
}

void AExperiment1Manager::start_trial()
{
	if (current_trial_index >= static_cast<uint32>(trials.Num()))
	{
		UE_LOG(LogTemp, Log, TEXT("All trials complete."));
		return;
	}

	// Reset all timers and motion state for the new trial
	total_trial_time = 0.0f;
	left_running_oscillation_timer = 0.0f;
	right_running_oscillation_timer = 0.0f;
	left_framecount = 0;
	left_delta_movement = FVector(0.0f, 0.0f, 0.0f);

	experiment_state = EXPERIMENT_STATE::TRIAL_RUNNING;

	const Trial& t = trials[current_trial_index];
	UE_LOG(LogTemp, Log, TEXT("Starting trial %d / %d"), current_trial_index + 1, trials.Num());

}


// Called every frame
void AExperiment1Manager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DeltaTime = 0.0167f;

	if (experiment_state != EXPERIMENT_STATE::TRIAL_RUNNING)
	{
		return;
	}

	if (left_moving_object && right_moving_object)
	{
		FVector left_delta_translation = left_translation_meters_per_second * DeltaTime;
		FRotator left_delta_rotation = left_rotation_deg_per_second * DeltaTime;
		FTransform left_delta_transform = FTransform(left_delta_rotation, left_delta_translation, FVector(1.0f, 1.0f, 1.0f));
		left_delta_movement += left_delta_translation;

		// rotate around z axis and translate in given direction
		if (left_moving_object)
		{
			// Check if we should simulate low fps for the purpose of testing blurred vs blurred
			if (render_same_fps)
			{
				left_moving_object->AddActorLocalTransform(left_delta_transform);
			}

			else
			{
				if (left_framecount % render_every_n_frames == 0)
				{
					left_delta_transform = FTransform(left_delta_rotation, left_delta_movement, FVector(1.0f, 1.0f, 1.0f));

					left_delta_movement = FVector(0.0f, 0.0f, 0.0f);
					left_moving_object->AddActorLocalTransform(left_delta_transform);

				}
			}
		}

		FVector right_delta_translation = right_translation_meters_per_second * DeltaTime;
		FRotator right_delta_rotation = right_rotation_deg_per_second * DeltaTime;
		FTransform right_delta_transform = FTransform(right_delta_rotation, right_delta_translation, FVector(1.0f, 1.0f, 1.0f));

		if (right_moving_object && total_trial_time > right_delay_time)
		{
			right_moving_object->AddActorLocalTransform(right_delta_transform);
		}

		left_framecount++;
	}

	else
	{
		UE_LOG(LogTemp, Log, TEXT("NO ACTOR FOUND"));

	}

	left_running_oscillation_timer += DeltaTime;
	if (left_running_oscillation_timer >= oscillation_time)
	{
		left_running_oscillation_timer = 0.0f;
		left_translation_meters_per_second *= -1.0f;
	}

	if (total_trial_time >= right_delay_time)
	{
		right_running_oscillation_timer += DeltaTime;
	}

	if (right_running_oscillation_timer >= oscillation_time)
	{
		right_running_oscillation_timer = 0.0f;
		right_translation_meters_per_second *= -1.0f;
	}


	total_trial_time += DeltaTime; // for the timer delay on start

	if (total_trial_time >= trial_length_time)
	{
		experiment_state = EXPERIMENT_STATE::WAITING_FOR_INPUT;
		current_trial_index++;
		UE_LOG(LogTemp, Log, TEXT("Trial complete. Press spacebar for next trial. (%d remaining)"), trials.Num() - current_trial_index);
	}

}

void AExperiment1Manager::set_actor_to_mobile(AActor* actor)
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


void AExperiment1Manager::initialize_trials()
{
	trials.Empty();

	for (int s = 0; s < static_cast<int>(STIMULI::COUNT); s++)
	{
		for (int lr = 0; lr < static_cast<int>(LEFTRIGHT::COUNT); lr++)
		{
			for (int e = 0; e < 3; e++)
			{
				for (int f = 0; f < 3; f++)
				{
					for (int rep = 0; rep < num_repetitions; rep++)
					{
						Trial t;
						t.stimuli = static_cast<STIMULI>(s);
						t.leftright = static_cast<LEFTRIGHT>(lr);
						t.eccentricity = eccentricities[e];
						t.frequency = frequencies[f];
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
