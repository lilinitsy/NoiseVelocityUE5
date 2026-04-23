#pragma once

#include "Engine/Texture2D.h"
#include "SceneViewExtension.h"

class CUSTOMCOMPUTESHADERSMODULE_API FGaborBlueNoiseTextureTilingViewExtension : public FSceneViewExtensionBase
{
  public:
	FGaborBlueNoiseTextureTilingViewExtension(
		const FAutoRegister &auto_register,
		FVector2f            foveation_center,
		float                radius_fovea,
		float                radius_periphery,
		float                screen_width_cm,
		float                screen_height_cm,
		float                distance_from_screen_cm,
		float                blur_rate_arcmin_per_degree,
		unsigned int         use_radially_increasing_blur,
		float                s_k,
		unsigned int         cells,
		unsigned int         impulses_per_cell,
		unsigned int         seed,
		UTexture2D          *bluenoise_tiling_texture);

	virtual void PrePostProcessPass_RenderThread(
		FRDGBuilder                 &graph_builder,
		const FSceneView            &view,
		const FPostProcessingInputs &inputs) override;

	virtual void SetupViewFamily(FSceneViewFamily &view_family) override
	{
	}
	virtual void SetupView(FSceneViewFamily &view_family, FSceneView &view) override
	{
	}
	virtual void BeginRenderViewFamily(FSceneViewFamily &view_family) override
	{
	}


	static TArray<FVector2f> ExtractBlueNoisePoints(UTexture2D *texture);

	FVector2f    foveation_center;
	float        radius_fovea;
	float        radius_periphery;
	float        screen_width_cm;
	float        screen_height_cm;
	float        distance_from_screen_cm;
	float        blur_rate_arcmin_per_degree;
	unsigned int use_radially_increasing_blur;
	float        s_k;
	unsigned int cells;
	unsigned int impulses_per_cell;
	unsigned int seed;

	TArray<FVector2f> blue_noise_points;
	uint32            grid_size;
};