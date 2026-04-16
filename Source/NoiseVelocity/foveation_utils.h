#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"

// Any function implemented in here HAS to be declared
// inline, otherwise the MSVC linker errors

const static int eccentricities[3] = { 18, 24, 30 }; // maybe only do 2?
const static float frequencies[2] = { 1.0f, 0.75f };
const static int render_every_n_fps_list[3] = { 1, 2, 3 };
//const static int render_every_n_fps_list[3] = { 20, 20, 20};
const static int num_repetitions = 2; // deprecate


inline float rad2deg(float rad)
{
	return rad * (180.0f / PI);
}

inline float deg2rad(float deg)
{
	return deg * (PI / 180.0f);
}

inline FVector2D rad2deg(const FVector2D& rad)
{
	return rad * (180.0f / PI);
}

inline FVector2D deg2rad(const FVector2D& deg)
{
	return deg * (PI / 180.0f);
}

inline FVector2D uv2deg(const FVector2D& uv, const FVector2D& fixation_uv, float distance_from_screen, const FVector2D& screen_dims_physical)
{
	FVector2D uv_diff = uv - fixation_uv;
	FVector2D physical_diff = uv_diff * screen_dims_physical;
	FVector2D angle_rad = FVector2D(FMath::Atan(physical_diff.X / distance_from_screen), FMath::Atan(physical_diff.Y / distance_from_screen));
	return rad2deg(angle_rad);
}

inline float eccentricity(const FVector2D& uv, const FVector2D& fixation_uv, float distance_from_screen, const FVector2D& screen_dims_physical)
{
	return uv2deg(uv, fixation_uv, distance_from_screen, screen_dims_physical).Size();
}

inline FVector get_center_position_of_camera_look(UCameraComponent *camera, float dist_from_camera)
{
	FVector camera_forward = camera->GetForwardVector();
	FVector camera_location = camera->GetComponentLocation();
	FVector center_position = camera_location + camera_forward * dist_from_camera;

	return center_position;
}