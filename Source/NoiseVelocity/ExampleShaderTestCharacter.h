// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "CustomComputeShadersViewExtension.h"

#include "ExampleShaderTestCharacter.generated.h"

UCLASS()
class NOISEVELOCITY_API AExampleShaderTestCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AExampleShaderTestCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	TSharedPtr<class FCustomComputeShadersViewExtension, ESPMode::ThreadSafe> view_extension;
	
};
