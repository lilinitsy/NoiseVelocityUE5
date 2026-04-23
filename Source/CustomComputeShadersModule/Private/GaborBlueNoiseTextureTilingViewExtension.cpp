#include "GaborBlueNoiseTextureTilingViewExtension.h"
#include "CustomComputeShaders.h"
#include "GenerateMips.h"
#include "PostProcess/PostProcessing.h"
#include "RenderGraphUtils.h"
#include "RenderGraphBuilder.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"



TArray<FVector2f> FGaborBlueNoiseTextureTilingViewExtension::ExtractBlueNoisePoints(UTexture2D* texture)
{
	TArray<FVector2f> points;

	if (!texture)
	{
		return points;
	}

	const int32 width = texture->GetSizeX();
	const int32 height = texture->GetSizeY();
	const FTexture2DMipMap& mip = texture->GetPlatformData()->Mips[0];
	const FColor* raw = static_cast<const FColor*>(mip.BulkData.LockReadOnly());

	if (raw)
	{
		for (int32 y = 0; y < height; y++)
		{
			for (int32 x = 0; x < width; x++)
			{
				// MAKE SURE the texture is imported as a UI texture so there's no compression
				// Somehow that messes up the texture values for raw.
				float r = raw[y * width + x].R / 255.0f;

				if (r <= 0.5f)
				{
					points.Add(FVector2f(static_cast<float>(x), static_cast<float>(y)));
				}
			}
		}
	}

	mip.BulkData.Unlock();
	return points;
}



FGaborBlueNoiseTextureTilingViewExtension::FGaborBlueNoiseTextureTilingViewExtension(
	const FAutoRegister& auto_register,
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
	UTexture2D *bluenoise_tiling_texture,
	unsigned int num_blue_noise_points)
	: FSceneViewExtensionBase(auto_register), 
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
	seed(seed),
	num_blue_noise_points(num_blue_noise_points)
{
	blue_noise_points = ExtractBlueNoisePoints(bluenoise_tiling_texture);
	grid_size = bluenoise_tiling_texture->GetSizeX();

	UE_LOG(LogTemp, Log, TEXT("Blue noise: extracted %d points from %dx%d texture"), blue_noise_points.Num(), grid_size, grid_size);
}



void FGaborBlueNoiseTextureTilingViewExtension::PrePostProcessPass_RenderThread(
	FRDGBuilder& graph_builder,
	const FSceneView& view,
	const FPostProcessingInputs& inputs)
{
	FRDGTextureRef scene_colour = (*inputs.SceneTextures)->SceneColorTexture;
	FRDGTextureDesc desc = scene_colour->Desc;
	desc.Flags |= TexCreate_UAV;
	desc.NumMips = 5;

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

	const FIntVector group_count(FMath::DivideAndRoundUp(desc.Extent.X, 16),FMath::DivideAndRoundUp(desc.Extent.Y, 16), 1);

	graph_builder.AddPass(
		RDG_EVENT_NAME("GaussianBlurCS"),
		blur_params,
		ERDGPassFlags::Compute,
		[blur_cs, blur_params, group_count](FRHICommandList& rhi_cmd_list)
		{
			FComputeShaderUtils::Dispatch(rhi_cmd_list, blur_cs, *blur_params, group_count); 
		});

	FGenerateMipsParams generate_mips_params = {};
	FGenerateMips::Execute(graph_builder, GMaxRHIFeatureLevel, blur_output, generate_mips_params);

	FRDGTextureRef combined_noise_output = graph_builder.CreateTexture(desc, TEXT("gabor_combined_noise_output"));
	FRDGTextureRef noise_output = graph_builder.CreateTexture(desc, TEXT("gabor_noise_output"));

	TShaderMapRef<FGaborBlueNoiseTextureTilingCS> noise_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FGaborBlueNoiseTextureTilingCS::FParameters* noise_params = graph_builder.AllocParameters<FGaborBlueNoiseTextureTilingCS::FParameters>();

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
	noise_params->frequency_scale = 1.0f;
	noise_params->region_mode = 0;

	noise_params->num_blue_noise_points = static_cast<uint32>(blue_noise_points.Num());
	noise_params->grid_size = grid_size;

	for (int32 i = 0; i < blue_noise_points.Num() && i < (int32) FMath::Min((unsigned int) 64, num_blue_noise_points); i++)
	{
		noise_params->blue_noise_points[i] = FVector4f(blue_noise_points[i].X / float(grid_size), blue_noise_points[i].Y / float(grid_size), 0.0f, 0.0f);
	}

	const FIntVector gabor_group_count(FMath::DivideAndRoundUp(desc.Extent.X, 16), FMath::DivideAndRoundUp(desc.Extent.Y, 16), 1);

	graph_builder.AddPass(
		RDG_EVENT_NAME("GaborNoiseEnhancementCS"), 
		noise_params, 
		ERDGPassFlags::Compute,
		[noise_cs, noise_params, gabor_group_count](FRHICommandList& rhi_cmd_list)
		{ 
			FComputeShaderUtils::Dispatch(rhi_cmd_list, noise_cs, *noise_params, gabor_group_count); 
		});

	AddCopyTexturePass(graph_builder, combined_noise_output, scene_colour);
}