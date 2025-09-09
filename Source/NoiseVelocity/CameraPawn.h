// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PostProcessComponent.h"

#include "GameFramework/Pawn.h"
#include "Engine/TextureRenderTarget2D.h"

#include "CameraPawn.generated.h"

UCLASS()
class NOISEVELOCITY_API ACameraPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACameraPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	USceneComponent* origin;
	UCameraComponent *camera;
	USceneCaptureComponent2D *left_scenecapture;
	USceneCaptureComponent2D *right_scenecapture;

	// Post process material
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess Materials")
	UMaterialInterface *composite_material;

};
