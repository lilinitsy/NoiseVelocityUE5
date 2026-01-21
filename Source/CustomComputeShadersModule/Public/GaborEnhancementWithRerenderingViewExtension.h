#pragma once
#include "SceneViewExtension.h"

class CUSTOMCOMPUTESHADERSMODULE_API FGaborEnhancementWithRerenderingViewExtension : public FSceneViewExtensionBase
{
public:
	FGaborEnhancementWithRerenderingViewExtension(
		const FAutoRegister& auto_register,
		unsigned int render_every_n_frames,
		FVector2f foveation_center,
		float radius_fovea,
		float radius_periphery,
		float screen_width_cm,
		float screen_height_cm,
		float distance_from_screen_cm,
		float blur_rate_arcmin_per_degree,
		unsigned int use_radially_increasing_blur,
		float s_k,
		unsigned int cells,
		unsigned int impulses_per_cell,
		unsigned int seed,
		float phase_cycles_per_sec,
		float phase_strength,
		unsigned int region_mode);

	virtual void SetupViewFamily(FSceneViewFamily& in_view_family) override {}
	virtual void SetupView(FSceneViewFamily& in_view_family, FSceneView& in_view) override {}
	virtual void BeginRenderViewFamily(FSceneViewFamily& in_view_family) override {}
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& graph_builder, const FSceneView& view, const FPostProcessingInputs& inputs) override;

	TRefCountPtr<IPooledRenderTarget> cached_base_image;
	TRefCountPtr<IPooledRenderTarget> cached_noise_texture;
	uint32 frame_counter = 0;
	float running_time = 0.0f;
	const uint32 render_every_n_frames;

	FVector2f foveation_center;
	const float radius_fovea;
	const float radius_periphery;
	const float screen_width_cm;
	const float screen_height_cm;
	const float distance_from_screen_cm;
	const float blur_rate_arcmin_per_degree;
	const unsigned int use_radially_increasing_blur;
	const float s_k;
	unsigned int cells;
	unsigned int impulses_per_cell;
	unsigned int static_seed; // For generating noise to be deterministic (but not the rerendered)
	unsigned int dynamic_seed;
	float phase_cycles_per_sec = 2.0f;
	float phase_strength = 1.0f;
	unsigned int region_mode = 0; // 0 = FULLSCREEN, 1 = LEFT, 2 = RIGHT

};
