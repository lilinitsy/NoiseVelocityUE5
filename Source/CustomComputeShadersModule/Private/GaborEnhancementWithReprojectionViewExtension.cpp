#include "GaborEnhancementWithReprojectionViewExtension.h"
#include "CustomComputeShaders.h"

#include "GenerateMips.h"
#include "PostProcess/PostProcessing.h"
#include "RenderGraphUtils.h"


FGaborEnhancementWithReprojectionViewExtension::FGaborEnhancementWithReprojectionViewExtension(const FAutoRegister& auto_register, unsigned int render_every_n_frames, FVector2f foveation_center, float radius_fovea, float radius_periphery, float screen_width_cm, float screen_height_cm, float distance_from_screen_cm, float blur_rate_arcmin_per_degree, unsigned int use_radially_increasing_blur, float s_k, unsigned int cells, unsigned int impulses_per_cell, unsigned int seed) :
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
	seed(seed)
{
}


// This runs the gaussian blur AND gabor enhancement, or it runs the noise reprojection
void FGaborEnhancementWithReprojectionViewExtension::PrePostProcessPass_RenderThread(
	FRDGBuilder& graph_builder,
	const FSceneView& view,
	const FPostProcessingInputs& inputs)
{
	bool should_render_full_frame = (frame_counter % render_every_n_frames == 0);
	frame_counter++;

	// Get motion vectors
	FRDGTextureRef motion_vector_texture = (*inputs.SceneTextures)->GBufferVelocityTexture;

	FRDGTextureRef scene_colour = (*inputs.SceneTextures)->SceneColorTexture;
	FRDGTextureDesc desc = scene_colour->Desc;
	desc.Flags |= TexCreate_UAV;
	desc.NumMips = 5;

	FRDGTextureRef final_output = nullptr;

	if(should_render_full_frame)
	{
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
		blur_params->distance_from_screen = distance_from_screen_cm; // for gaussian blur, it's just called "distance_from_screen"
		blur_params->blur_rate_arcmin_per_degree = blur_rate_arcmin_per_degree;
		blur_params->use_radially_increasing_blur = use_radially_increasing_blur;

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

		graph_builder.QueueTextureExtraction(blur_output, &cached_base_rendered_image, ERDGResourceExtractionFlags::None);


		// Generate mips so that the amplitude estimator works
		FGenerateMipsParams generate_mips_params = {};
		FGenerateMips::Execute(graph_builder, GMaxRHIFeatureLevel, blur_output, generate_mips_params);

		// gabor noise
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

		// cache textures, no flags needed since this resource will be held a while
		graph_builder.QueueTextureExtraction(noise_output, &cached_noise_texture, ERDGResourceExtractionFlags::None);

		// copy final back to scene colour
		AddCopyTexturePass(graph_builder, combined_noise_output, scene_colour); 
	}

	else
	{
		// Check to avoid segfault
		if (cached_base_rendered_image && cached_noise_texture)
		{
			FRDGTextureRef cached_base = graph_builder.RegisterExternalTexture(cached_base_rendered_image, TEXT("cached_base_image"));
			FRDGTextureRef cached_noise = graph_builder.RegisterExternalTexture(cached_noise_texture, TEXT("cached_noise_texture"));
			FRDGTextureRef reprojected_noise = graph_builder.CreateTexture(desc, TEXT("reprojected_noise"));
			FRDGTextureRef composite_output = graph_builder.CreateTexture(desc, TEXT("reprojected_noise_combined_output"));

			TShaderMapRef<FNoiseReprojectionCS> reprojection_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
			FNoiseReprojectionCS::FParameters* reproject_params = graph_builder.AllocParameters<FNoiseReprojectionCS::FParameters>();

			reproject_params->input_foveated = cached_base;
			reproject_params->previous_noise_texture = cached_noise;
			reproject_params->motion_vector_texture = motion_vector_texture;
			reproject_params->point_sampler = TStaticSamplerState<SF_Point>::GetRHI();
			reproject_params->linear_sampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
			reproject_params->reprojected_noise_texture = graph_builder.CreateUAV(reprojected_noise);
			reproject_params->output_texture = graph_builder.CreateUAV(composite_output);

			const FIntVector reproject_group_count(
				FMath::DivideAndRoundUp(desc.Extent.X, 16),
				FMath::DivideAndRoundUp(desc.Extent.Y, 16),
				1);

			graph_builder.AddPass(
				RDG_EVENT_NAME("NoiseReprojectionCS"),
				reproject_params,
				ERDGPassFlags::Compute,
				[reprojection_cs, reproject_params, reproject_group_count](FRHICommandList& rhi_cmd_list)
				{
					FComputeShaderUtils::Dispatch(rhi_cmd_list, reprojection_cs, *reproject_params, reproject_group_count);
				});

			graph_builder.QueueTextureExtraction(reprojected_noise, &cached_noise_texture);

			AddCopyTexturePass(graph_builder, composite_output, scene_colour);
		}
	}
}

