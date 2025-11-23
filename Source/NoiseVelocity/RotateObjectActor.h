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

	// Make sure this is added to Actor Tags, NOT Component Tags!
	UPROPERTY(EditAnywhere, Category = "Settings")
	FString tag_to_find;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float rotation_deg_per_second = 90.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	AActor* rotating_object;


	//AActor *rotating_object = nullptr;
};
