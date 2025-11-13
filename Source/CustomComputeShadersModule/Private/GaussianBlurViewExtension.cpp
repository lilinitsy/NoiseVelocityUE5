#include "GaussianBlurViewExtension.h"
#include "CustomComputeShaders.h"

#include "PostProcess/PostProcessing.h"
#include "RenderGraphUtils.h"

FGaussianBlurViewExtension::FGaussianBlurViewExtension(const FAutoRegister& auto_register, FVector2f foveation_center, float radius_fovea, float radius_periphery, float screen_width_cm, float screen_height_cm, float distance_from_screen, float blur_rate_arcmin_per_degree, unsigned int use_radially_increasing_blur) :
    FSceneViewExtensionBase(auto_register),
    foveation_center(foveation_center),
    radius_fovea(radius_fovea),
    radius_periphery(radius_periphery),
    screen_width_cm(screen_width_cm),
    screen_height_cm(screen_height_cm),
    distance_from_screen(distance_from_screen),
    blur_rate_arcmin_per_degree(blur_rate_arcmin_per_degree),
    use_radially_increasing_blur(use_radially_increasing_blur)
{
}

void FGaussianBlurViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& graph_builder, const FSceneView& view, const FPostProcessingInputs& inputs)
{

    FRDGTextureRef scene_color = (*inputs.SceneTextures)->SceneColorTexture;

    // create output with UAV
    FRDGTextureDesc desc = scene_color->Desc;
    desc.Flags |= TexCreate_UAV;

    FRDGTextureRef blur_output = graph_builder.CreateTexture(desc, TEXT("gaussian_blur_output"));

    // bind compute shader
    TShaderMapRef<FGaussianBlurCS> blur_cs(GetGlobalShaderMap(GMaxRHIFeatureLevel));
    FGaussianBlurCS::FParameters* params = graph_builder.AllocParameters<FGaussianBlurCS::FParameters>();

    params->input_texture = scene_color;
    params->Input_Sampler = TStaticSamplerState<SF_Bilinear>::GetRHI();
    params->output_texture = graph_builder.CreateUAV(blur_output);

    params->foveation_center = foveation_center;
    params->radius_fovea = radius_fovea;
    params->radius_periphery = radius_periphery;
    params->screen_width_cm = screen_width_cm;
    params->screen_height_cm = screen_height_cm;
    params->distance_from_screen = distance_from_screen;
    params->blur_rate_arcmin_per_degree = blur_rate_arcmin_per_degree;
    params->use_radially_increasing_blur = use_radially_increasing_blur;

    const FIntVector group_count(
        FMath::DivideAndRoundUp(desc.Extent.X, 16),
        FMath::DivideAndRoundUp(desc.Extent.Y, 16),
        1);

    graph_builder.AddPass(
        RDG_EVENT_NAME("gaussian_blur_compute"),
        params,
        ERDGPassFlags::Compute,
        [blur_cs, params, group_count](FRHICommandList& rhi_cmd_list)
        {
            FComputeShaderUtils::Dispatch(rhi_cmd_list, blur_cs, *params, group_count);
        });

    // overwrite scene color so later passes see blurred image
    AddCopyTexturePass(graph_builder, blur_output, scene_color);
}
