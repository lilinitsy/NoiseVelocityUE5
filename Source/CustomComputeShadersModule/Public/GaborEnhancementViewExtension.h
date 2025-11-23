#pragma once

#include "SceneViewExtension.h"

class CUSTOMCOMPUTESHADERSMODULE_API FGaborEnhancementViewExtension : public FSceneViewExtensionBase
{
public:
    FGaborEnhancementViewExtension(const FAutoRegister& auto_register, FVector2f foveation_center, float radius_fovea, float radius_periphery, float screen_width_cm, float screen_height_cm, float distance_from_screen_cm, float blur_rate_arcmin_per_degree, unsigned int use_radially_increasing_blur, float s_k, unsigned int cells, unsigned int impulses_per_cell, unsigned int seed);

    virtual void SetupViewFamily(FSceneViewFamily& in_view_family) override {}
    virtual void SetupView(FSceneViewFamily& in_view_family, FSceneView& in_view) override {}
    virtual void BeginRenderViewFamily(FSceneViewFamily& in_view_family) override {}

    virtual void PrePostProcessPass_RenderThread(FRDGBuilder& graph_builder, const FSceneView& view, const FPostProcessingInputs& inputs) override;

    FVector2f foveation_center;
	const float        radius_fovea;     // for gaussian blur usage
	const float        radius_periphery; // for gaussian blur
	const float        screen_width_cm;
	const float        screen_height_cm;
	const float        distance_from_screen_cm;
	const float        blur_rate_arcmin_per_degree;
	const unsigned int use_radially_increasing_blur;
	const float        s_k;

    unsigned int cells;
	unsigned int impulses_per_cell;
	unsigned int seed;
};
