// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "foveation_utils.h"
#include "DeSyncObjectMover2D.generated.h"

UCLASS()
class NOISEVELOCITY_API ADeSyncObjectMover2D : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADeSyncObjectMover2D();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void initialize_object_positions(float eccentricity, float dist_from_camera);

	void move_object_horizontally(AActor* object, float start_ecc, float end_ecc, float dist_from_camera, float period);
	void move_object_vertically(AActor* object, float start_ecc, float end_ecc, float dist_from_camera, float period);
	void move_object_diagonally(AActor* object, float start_ecc, float end_ecc, float dist_from_camera, float period);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cameras")
	APawn *camera_pawn;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objects")
	AActor *left_object;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objects")
	AActor *right_object;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float object_dist_from_camera = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float start_eccentricity = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parameters")
	float end_eccentricity = 60.0f;




	unsigned int num_ticks = 0;

};
