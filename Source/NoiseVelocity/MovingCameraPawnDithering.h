// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PostProcessComponent.h"

#include "GameFramework/Pawn.h"
#include "Engine/TextureRenderTarget2D.h"

#include "MovingCameraPawnDithering.generated.h"

UCLASS()
class NOISEVELOCITY_API AMovingCameraPawnDithering : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMovingCameraPawnDithering();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void move_scene_capture_component2d_forward(USceneCaptureComponent2D *scenecapture, float movement_speed, float dt);

	void initialize();

	// Input functions
	UFUNCTION()
	void on_up_pressed();

	UFUNCTION()
	void on_down_pressed();


	USceneComponent* origin;
	UCameraComponent *camera;
	USceneCaptureComponent2D *left_scenecapture;
	USceneCaptureComponent2D *right_scenecapture; // REFERENCE side

	// Post process material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess Materials")
	UMaterialInterface *composite_material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float reference_movement_speed = 100.0f; // 1m/sec
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float experiment_condition_movement_speed = 150.0f;


	unsigned int num_ticks = 0;

	bool initialized = false;
};
