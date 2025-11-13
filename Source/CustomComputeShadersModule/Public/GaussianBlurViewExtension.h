#pragma once

#include "SceneViewExtension.h"

class CUSTOMCOMPUTESHADERSMODULE_API FGaussianBlurViewExtension : public FSceneViewExtensionBase
{
public:
	FGaussianBlurViewExtension(const FAutoRegister &auto_register, FVector2f foveation_center, float radius_fovea, float radius_periphery, float screen_width_cm, float screen_height_cm, float distance_from_screen, float blur_rate_arcmin_per_degree, unsigned int use_radially_increasing_blur = 1);

	virtual void SetupViewFamily(FSceneViewFamily &in_view_family) override
	{
	}
	virtual void SetupView(FSceneViewFamily &in_view_family, FSceneView &in_view) override
	{
	}
	virtual void BeginRenderViewFamily(FSceneViewFamily &in_view_family) override
	{
	}

	virtual void PrePostProcessPass_RenderThread(FRDGBuilder &graph_builder, const FSceneView &view, const FPostProcessingInputs &inputs) override;

	FVector2f    foveation_center;
	float        radius_fovea;
	float        radius_periphery;
	float        screen_width_cm;
	float        screen_height_cm;
	float        distance_from_screen;
	float        blur_rate_arcmin_per_degree;
	unsigned int use_radially_increasing_blur = 1;
};
