// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "GaborEnhancementWithRerenderingViewExtension.h"


#include "GaborEnhanceWithRerenderTestChar.generated.h"

UCLASS()
class NOISEVELOCITY_API AGaborEnhanceWithRerenderTestChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGaborEnhanceWithRerenderTestChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	TSharedPtr<FGaborEnhancementWithRerenderingViewExtension, ESPMode::ThreadSafe> view_extension;

	
	UPROPERTY(EditAnywhere, Category = "Settings")
	float blur_rate_arcmin_per_degree = 0.34;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	unsigned int use_radially_increasing_blur = 0;

	UPROPERTY(EditAnywhere, Category = "Settings")
	unsigned int render_every_n_frames = 1;

	// Keep to 0 if no rotation
	UPROPERTY(EditAnywhere, Category = "Settings")
	float rotation_speed = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float phase_cycles_per_sec = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Settings")
	float phase_strength = 1.0f;

	// 0 = FULLSCREEN, 1 = LEFT, 2 = RIGHT
	UPROPERTY(EditAnywhere, Category = "Settings")
	unsigned int region_mode = 0;


};
