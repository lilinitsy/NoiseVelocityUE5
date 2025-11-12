#include "GaborEnhancementViewExtension.h"
#include "CustomComputeShaders.h"

#include "GenerateMips.h"
#include "PostProcess/PostProcessing.h"
#include "RenderGraphUtils.h"


FGaborEnhancementViewExtension::FGaborEnhancementViewExtension(const FAutoRegister& auto_register, FVector2f foveation_center, float radius_fovea, float radius_periphery, float screen_width_cm, float screen_height_cm, float distance_from_screen_cm, float blur_rate_arcmin_per_degree, float s_k, unsigned int cells, unsigned int impulses_per_cell, unsigned int seed) :
	FSceneViewExtensionBase(auto_register),
	foveation_center(foveation_center),
	radius_fovea(radius_fovea),
	radius_periphery(radius_periphery),
    screen_width_cm(screen_width_cm),
    screen_height_cm(screen_height_cm),
	distance_from_screen_cm(distance_from_screen_cm),
	blur_rate_arcmin_per_degree(blur_rate_arcmin_per_degree),
	s_k(s_k),
	cells(cells),
	impulses_per_cell(impulses_per_cell),
	seed(seed)
{
}


// This runs the gaussian blur AND gabor enhancement
void FGaborEnhancementViewExtension::PrePostProcessPass_RenderThread(
	FRDGBuilder& graph_builder,
	const FSceneView& view,
	const FPostProcessingInputs& inputs)
{

	FRDGTextureRef scene_color = (*inputs.SceneTextures)->SceneColorTexture;
	FRDGTextureDesc desc = scene_color->Desc;
	desc.Flags |= TexCreate_UAV;
	desc.NumMips = 5;

	FRDGTextureRef blur_output = graph_builder.CreateTexture(desc, TEXT("gaussian_blur_output"));

	TShaderMapRef<FGaussianBlurCS> blur_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FGaussianBlurCS::FParameters* blur_params = graph_builder.AllocParameters<FGaussianBlurCS::FParameters>();

	blur_params->input_texture = scene_color;
	blur_params->Input_Sampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
	blur_params->output_texture = graph_builder.CreateUAV(blur_output);

	blur_params->foveation_center = foveation_center;
	blur_params->radius_fovea = radius_fovea;
	blur_params->radius_periphery = radius_periphery;
	blur_params->screen_width_cm = screen_width_cm;
	blur_params->screen_height_cm = screen_height_cm;
	blur_params->distance_from_screen = distance_from_screen_cm; // for gaussian blur, it's just called "distance_from_screen"

	const FIntVector group_count(
		FMath::DivideAndRoundUp(desc.Extent.X, 16),
		FMath::DivideAndRoundUp(desc.Extent.Y, 16),
		1);

	graph_builder.AddPass(
		RDG_EVENT_NAME("GaussianBlurCS"),
		blur_params,
		ERDGPassFlags::Compute,
		[blur_cs, blur_params, group_count](FRHICommandList& rhi_cmd_list)
		{
			FComputeShaderUtils::Dispatch(rhi_cmd_list, blur_cs, *blur_params, group_count);
		});
	
	// Generate mips so that the amplitude estimator works
	FGenerateMipsParams generate_mips_params = {};
	FGenerateMips::Execute(graph_builder, GMaxRHIFeatureLevel, blur_output, generate_mips_params);

	// gabor noise
	FRDGTextureRef noise_output = graph_builder.CreateTexture(desc, TEXT("gabor_noise_output"));

	TShaderMapRef<FGaborNoiseEnhancementCS> noise_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FGaborNoiseEnhancementCS::FParameters* noise_params = graph_builder.AllocParameters<FGaborNoiseEnhancementCS::FParameters>();

	noise_params->input_foveated = blur_output;								// <-- chain
	noise_params->LinearSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
	noise_params->output_texture = graph_builder.CreateUAV(noise_output);

	noise_params->foveation_center = foveation_center;
	noise_params->screen_width_cm = screen_width_cm;
	noise_params->screen_height_cm = screen_height_cm;
	noise_params->distance_from_screen_cm = distance_from_screen_cm;
	noise_params->blur_rate_arcmin_per_degree = blur_rate_arcmin_per_degree;
	noise_params->s_k = s_k;
	noise_params->cells = cells;
	noise_params->impulses_per_cell = impulses_per_cell;
	noise_params->seed = seed;

	const FIntVector gabor_group_count(
		FMath::DivideAndRoundUp(desc.Extent.X, 16),
		FMath::DivideAndRoundUp(desc.Extent.Y, 16),
		1);

	graph_builder.AddPass(
		RDG_EVENT_NAME("GaborNoiseEnhancementCS"),
		noise_params,
		ERDGPassFlags::Compute,
		[noise_cs, noise_params, gabor_group_count](FRHICommandList& rhi_cmd_list)
		{
			FComputeShaderUtils::Dispatch(rhi_cmd_list, noise_cs, *noise_params, gabor_group_count);
		});
	

	// copy final back to scene colour
	AddCopyTexturePass(graph_builder, noise_output, scene_color);
}

