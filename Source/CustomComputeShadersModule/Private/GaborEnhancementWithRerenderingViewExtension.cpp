#include "GaborEnhancementWithRerenderingViewExtension.h"
#include "CustomComputeShaders.h"
#include "GenerateMips.h"
#include "PostProcess/PostProcessing.h"
#include "RenderGraphUtils.h"

FGaborEnhancementWithRerenderingViewExtension::FGaborEnhancementWithRerenderingViewExtension(
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
	unsigned int seed) :
	FSceneViewExtensionBase(auto_register),
	render_every_n_frames(render_every_n_frames),
	foveation_center(foveation_center),
	radius_fovea(radius_fovea),
	radius_periphery(radius_periphery),
	screen_width_cm(screen_width_cm),
	screen_height_cm(screen_height_cm),
	distance_from_screen_cm(distance_from_screen_cm),
	blur_rate_arcmin_per_degree(blur_rate_arcmin_per_degree),
	use_radially_increasing_blur(use_radially_increasing_blur),
	s_k(s_k),
	cells(cells),
	impulses_per_cell(impulses_per_cell),
	static_seed(seed),
	dynamic_seed(seed)
{
}

void FGaborEnhancementWithRerenderingViewExtension::PrePostProcessPass_RenderThread(
	FRDGBuilder& graph_builder,
	const FSceneView& view,
	const FPostProcessingInputs& inputs)
{
	bool should_render_full_frame = (frame_counter % render_every_n_frames == 0);
	frame_counter++;
	dynamic_seed++;

	FRDGTextureRef scene_colour = (*inputs.SceneTextures)->SceneColorTexture;
	FRDGTextureRef motion_vector_texture = (*inputs.SceneTextures)->GBufferVelocityTexture;

	FRDGTextureDesc desc = scene_colour->Desc;
	desc.Flags |= TexCreate_UAV;
	desc.NumMips = 5;

	if (should_render_full_frame)
	{
		// Step 1: Gaussian blur
		FRDGTextureRef blur_output = graph_builder.CreateTexture(desc, TEXT("gaussian_blur_output"));

		TShaderMapRef<FGaussianBlurCS> blur_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		FGaussianBlurCS::FParameters* blur_params = graph_builder.AllocParameters<FGaussianBlurCS::FParameters>();

		blur_params->input_texture = scene_colour;
		blur_params->Input_Sampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		blur_params->output_texture = graph_builder.CreateUAV(blur_output);
		blur_params->foveation_center = foveation_center;
		blur_params->radius_fovea = radius_fovea;
		blur_params->radius_periphery = radius_periphery;
		blur_params->screen_width_cm = screen_width_cm;
		blur_params->screen_height_cm = screen_height_cm;
		blur_params->distance_from_screen = distance_from_screen_cm;
		blur_params->blur_rate_arcmin_per_degree = blur_rate_arcmin_per_degree;
		blur_params->use_radially_increasing_blur = use_radially_increasing_blur;

		const FIntVector blur_group_count(
			FMath::DivideAndRoundUp(desc.Extent.X, 16),
			FMath::DivideAndRoundUp(desc.Extent.Y, 16),
			1);

		graph_builder.AddPass(
			RDG_EVENT_NAME("GaussianBlurCS"),
			blur_params,
			ERDGPassFlags::Compute,
			[blur_cs, blur_params, blur_group_count](FRHICommandList& rhi_cmd_list)
			{
				FComputeShaderUtils::Dispatch(rhi_cmd_list, blur_cs, *blur_params, blur_group_count);
			});

		// Generate mips for Laplacian pyramid
		FGenerateMipsParams generate_mips_params = {};
		FGenerateMips::Execute(graph_builder, GMaxRHIFeatureLevel, blur_output, generate_mips_params);

		// Cache the blurred base image
		graph_builder.QueueTextureExtraction(blur_output, &cached_base_image, ERDGResourceExtractionFlags::None);

		// Step 2: Full Gabor noise generation
		FRDGTextureRef combined_noise_output = graph_builder.CreateTexture(desc, TEXT("gabor_combined_noise_output"));
		FRDGTextureRef noise_output = graph_builder.CreateTexture(desc, TEXT("gabor_noise_output"));

		TShaderMapRef<FGaborNoiseEnhancementCS> noise_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		FGaborNoiseEnhancementCS::FParameters* noise_params = graph_builder.AllocParameters<FGaborNoiseEnhancementCS::FParameters>();

		noise_params->input_foveated = blur_output;
		noise_params->LinearSampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		noise_params->output_texture = graph_builder.CreateUAV(combined_noise_output);
		noise_params->output_noise_texture = graph_builder.CreateUAV(noise_output);
		noise_params->foveation_center = foveation_center;
		noise_params->screen_width_cm = screen_width_cm;
		noise_params->screen_height_cm = screen_height_cm;
		noise_params->distance_from_screen_cm = distance_from_screen_cm;
		noise_params->blur_rate_arcmin_per_degree = blur_rate_arcmin_per_degree;
		noise_params->use_radially_increasing_blur = use_radially_increasing_blur;
		noise_params->s_k = s_k;
		noise_params->cells = cells;
		noise_params->impulses_per_cell = impulses_per_cell;
		noise_params->seed = static_seed;

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

		// Cache noise for next frame
		graph_builder.QueueTextureExtraction(noise_output, &cached_noise_texture, ERDGResourceExtractionFlags::None);

		// Copy final output to scene color
		AddCopyTexturePass(graph_builder, combined_noise_output, scene_colour);
	}


	else if (cached_base_image && cached_noise_texture)
	{
		FRDGTextureRef cached_base = graph_builder.RegisterExternalTexture(cached_base_image, TEXT("cached_base_image"));
		FRDGTextureRef previous_noise = graph_builder.RegisterExternalTexture(cached_noise_texture, TEXT("previous_noise"));

		// Regenerate mips for cached base since extraction loses them
		FGenerateMipsParams generate_mips_params = {};
		FGenerateMips::Execute(graph_builder, GMaxRHIFeatureLevel, cached_base, generate_mips_params);

		FRDGTextureRef combined_output = graph_builder.CreateTexture(desc, TEXT("selective_rerender_combined_output"));
		FRDGTextureRef reprojected_noise = graph_builder.CreateTexture(desc, TEXT("selective_rerender_noise"));

		TShaderMapRef<FGaborNoiseEnhancementWithRerenderingCS> noise_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		FGaborNoiseEnhancementWithRerenderingCS::FParameters* noise_params =
			graph_builder.AllocParameters<FGaborNoiseEnhancementWithRerenderingCS::FParameters>();

		noise_params->input_foveated = cached_base;
		noise_params->previous_noise_texture = previous_noise;
		noise_params->motion_vector_texture = motion_vector_texture;
		noise_params->linear_sampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
		noise_params->point_sampler = TStaticSamplerState<SF_Point>::GetRHI();
		noise_params->reprojected_noise_texture = graph_builder.CreateUAV(reprojected_noise);
		noise_params->output_texture = graph_builder.CreateUAV(combined_output);
		noise_params->foveation_center = foveation_center;
		noise_params->screen_width_cm = screen_width_cm;
		noise_params->screen_height_cm = screen_height_cm;
		noise_params->distance_from_screen_cm = distance_from_screen_cm;
		noise_params->blur_rate_arcmin_per_degree = blur_rate_arcmin_per_degree;
		noise_params->use_radially_increasing_blur = use_radially_increasing_blur;
		noise_params->s_k = s_k;
		noise_params->cells = cells;
		noise_params->impulses_per_cell = impulses_per_cell;
		noise_params->seed = static_seed;
		noise_params->time_seconds = (float) view.Family->Time.GetWorldTimeSeconds();
		noise_params->phase_cycles_per_sec = 4.0f;
		noise_params->phase_strength = 4.0f;

		const FIntVector noise_group_count(
			FMath::DivideAndRoundUp(desc.Extent.X, 16),
			FMath::DivideAndRoundUp(desc.Extent.Y, 16),
			1);

		graph_builder.AddPass(
			RDG_EVENT_NAME("SelectiveRerenderNoiseCS"),
			noise_params,
			ERDGPassFlags::Compute,
			[noise_cs, noise_params, noise_group_count](FRHICommandList& rhi_cmd_list)
			{
				FComputeShaderUtils::Dispatch(rhi_cmd_list, noise_cs, *noise_params, noise_group_count);
			});

		// Cache the reprojected noise for next frame
		graph_builder.QueueTextureExtraction(reprojected_noise, &cached_noise_texture, ERDGResourceExtractionFlags::None);

		// Copy final output back to scene color
		AddCopyTexturePass(graph_builder, combined_output, scene_colour);
	
		// Debug: Display the reprojected noise
		//AddCopyTexturePass(graph_builder, reprojected_noise, scene_colour);

	}

}