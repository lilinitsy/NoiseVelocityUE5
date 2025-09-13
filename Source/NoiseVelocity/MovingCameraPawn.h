// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PostProcessComponent.h"

#include "GameFramework/Pawn.h"
#include "Engine/TextureRenderTarget2D.h"

#include "MovingCameraPawn.generated.h"

UCLASS()
class NOISEVELOCITY_API AMovingCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMovingCameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void move_scene_capture_component2d_forward(USceneCaptureComponent2D *scenecapture, float dt);

	USceneComponent* origin;
	UCameraComponent *camera;
	USceneCaptureComponent2D *left_scenecapture;
	USceneCaptureComponent2D *right_scenecapture;

	// Post process material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess Materials")
	UMaterialInterface *composite_material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float movement_speed = 100.0f; // 1m/sec


	unsigned int num_ticks = 0;
};
