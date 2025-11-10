// Fill out your copyright notice in the Description page of Project Settings.


#include "ExampleShaderTestCharacter.h"

#include "SceneViewExtension.h"


// Sets default values
AExampleShaderTestCharacter::AExampleShaderTestCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AExampleShaderTestCharacter::BeginPlay()
{
	Super::BeginPlay();
	

	view_extension = FSceneViewExtensions::NewExtension<FCustomComputeShadersViewExtension>(FLinearColor::Green);

}

// Called every frame
void AExampleShaderTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AExampleShaderTestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

