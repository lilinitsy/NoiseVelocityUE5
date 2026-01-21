// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RotateObjectActor.generated.h"

UCLASS()
class NOISEVELOCITY_API ARotateObjectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARotateObjectActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Settings")
	FRotator rotation_deg_per_second = FRotator(0.0f, 0.0f, 90.0f);

	UPROPERTY(EditAnywhere, Category = "Settings")
	FVector translation_meters_per_second = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Settings")
	float oscillation_time = 1.0f; // how long to let it move before it bounces back and forth

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Actors")
	AActor* left_moving_object;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Moving Actors")
	AActor* right_moving_object;


	// hidden for modulating the oscillation time
	float running_oscillation_timer = 0.0f;


	void set_actor_to_mobile(AActor *actor);

};
