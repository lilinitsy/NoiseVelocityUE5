#include "GlobalShader.h"
#include "ScreenPass.h"


// Declare a class and shader structs for each shader declared in the cpp version
BEGIN_SHADER_PARAMETER_STRUCT(FUVMaskShaderParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColor)
	SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D<uint2>, InputStencilTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, ViewParams)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
class FUVMaskShaderPS : public FGlobalShader
{
	public:
		DECLARE_EXPORTED_SHADER_TYPE(FUVMaskShaderPS, Global, );
		using FParameters = FUVMaskShaderParameters;
		SHADER_USE_PARAMETER_STRUCT(FUVMaskShaderPS, FGlobalShader);
};

BEGIN_SHADER_PARAMETER_STRUCT(FCombineShaderParameters, )
	SHADER_PARAMETER(FLinearColor, Color)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SceneColor)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
	SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
	SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, ViewParams)
	RENDER_TARGET_BINDING_SLOTS()
END_SHADER_PARAMETER_STRUCT()
class FCombineShaderPS : public FGlobalShader 
{
	public:
		DECLARE_EXPORTED_SHADER_TYPE(FCombineShaderPS, Global, );
		using FParameters = FCombineShaderParameters;
		SHADER_USE_PARAMETER_STRUCT(FCombineShaderPS, FGlobalShader);
};



BEGIN_SHADER_PARAMETER_STRUCT(FGaussianBlurShaderParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, input_texture)
	SHADER_PARAMETER_SAMPLER(SamplerState, Input_Sampler)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_texture)

	SHADER_PARAMETER(FVector2f, foveation_center)
	SHADER_PARAMETER(float, radius_fovea)
	SHADER_PARAMETER(float, radius_periphery)
	
	SHADER_PARAMETER(float, screen_width_cm)
	SHADER_PARAMETER(float, screen_height_cm)
	SHADER_PARAMETER(float, distance_from_screen)

	SHADER_PARAMETER(float, blur_rate_arcmin_per_degree)
	SHADER_PARAMETER(unsigned int, use_radially_increasing_blur)
END_SHADER_PARAMETER_STRUCT()
class FGaussianBlurCS : public FGlobalShader
{
	public:
		DECLARE_EXPORTED_SHADER_TYPE(FGaussianBlurCS, Global, );
		using FParameters = FGaussianBlurShaderParameters;
		SHADER_USE_PARAMETER_STRUCT(FGaussianBlurCS, FGlobalShader);
};


BEGIN_SHADER_PARAMETER_STRUCT(FGaborNoiseEnhancementParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, input_foveated)
	SHADER_PARAMETER_SAMPLER(SamplerState, LinearSampler)

	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, motion_vector_texture)
	SHADER_PARAMETER_SAMPLER(SamplerState, motion_vector_sampler)

	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_texture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_noise_texture)

	SHADER_PARAMETER(FVector2f, foveation_center)
	SHADER_PARAMETER(float, screen_width_cm)
	SHADER_PARAMETER(float, screen_height_cm)
	SHADER_PARAMETER(float, distance_from_screen_cm)

	SHADER_PARAMETER(float, blur_rate_arcmin_per_degree)
	SHADER_PARAMETER(unsigned int, use_radially_increasing_blur)
	SHADER_PARAMETER(float, s_k)
	SHADER_PARAMETER(unsigned int, cells)
	SHADER_PARAMETER(unsigned int, impulses_per_cell)
	SHADER_PARAMETER(unsigned int, seed)
END_SHADER_PARAMETER_STRUCT()
class FGaborNoiseEnhancementCS : public FGlobalShader
{
	public:
		DECLARE_EXPORTED_SHADER_TYPE(FGaborNoiseEnhancementCS, Global, );
		using FParameters = FGaborNoiseEnhancementParameters;
		SHADER_USE_PARAMETER_STRUCT(FGaborNoiseEnhancementCS, FGlobalShader);
};



// THE NEXT TWO CLASSES ARE FOR GABOR NOISE WITH REPROJECTION
BEGIN_SHADER_PARAMETER_STRUCT(FGaborNoiseEnhancementWithReprojectionParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, input_foveated)
	SHADER_PARAMETER_SAMPLER(SamplerState, LinearSampler)

	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, motion_vector_texture)
	SHADER_PARAMETER_SAMPLER(SamplerState, motion_vector_sampler)

	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_texture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_noise_texture)

	SHADER_PARAMETER(FVector2f, foveation_center)
	SHADER_PARAMETER(float, screen_width_cm)
	SHADER_PARAMETER(float, screen_height_cm)
	SHADER_PARAMETER(float, distance_from_screen_cm)

	SHADER_PARAMETER(float, blur_rate_arcmin_per_degree)
	SHADER_PARAMETER(unsigned int, use_radially_increasing_blur)
	SHADER_PARAMETER(float, s_k)
	SHADER_PARAMETER(unsigned int, cells)
	SHADER_PARAMETER(unsigned int, impulses_per_cell)
	SHADER_PARAMETER(unsigned int, seed)
END_SHADER_PARAMETER_STRUCT()
class FGaborNoiseEnhancementWithReprojectionCS : public FGlobalShader
{
public:
	DECLARE_EXPORTED_SHADER_TYPE(FGaborNoiseEnhancementWithReprojectionCS, Global, );
	using FParameters = FGaborNoiseEnhancementWithReprojectionParameters;
	SHADER_USE_PARAMETER_STRUCT(FGaborNoiseEnhancementWithReprojectionCS, FGlobalShader);
};

// Class for reprojection parameters and reprojection compute shader
// To be used with the FGaborNoiseEnhancementWithReprojectionCS view extension
BEGIN_SHADER_PARAMETER_STRUCT(FNoiseReprojectionParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, input_foveated)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, previous_noise_texture)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, motion_vector_texture)
	SHADER_PARAMETER_SAMPLER(SamplerState, point_sampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, linear_sampler)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, reprojected_noise_texture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_texture)
END_SHADER_PARAMETER_STRUCT()
class FNoiseReprojectionCS : public FGlobalShader
{
	public:
		DECLARE_EXPORTED_SHADER_TYPE(FNoiseReprojectionCS, Global, );
		using FParameters = FNoiseReprojectionParameters;
		SHADER_USE_PARAMETER_STRUCT(FNoiseReprojectionCS, FGlobalShader);
};


// THESE USE GABOR ENHANCEMENT, BUT RERENDER NOISE EVERY FRAME, EVEN IF SHADING IS NOT UPDATED
BEGIN_SHADER_PARAMETER_STRUCT(FGaborNoiseEnhancementWithRerenderingParameters, )
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, input_foveated)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, previous_noise_texture)
	SHADER_PARAMETER_RDG_TEXTURE(Texture2D, motion_vector_texture)

	SHADER_PARAMETER_SAMPLER(SamplerState, linear_sampler)
	SHADER_PARAMETER_SAMPLER(SamplerState, point_sampler)

	//SHADER_PARAMETER_RDG_TEXTURE(Texture2D, depth_texture)
	// SHADER_PARAMETER_RDG_TEXTURE(Texture2D, normal_texture)

	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_texture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, output_noise_texture)
	SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, reprojected_noise_texture)
	
	//SHADER_PARAMETER(FMatrix44f, inv_view_projection_matrix)
	SHADER_PARAMETER(FVector2f, foveation_center)
	SHADER_PARAMETER(float, screen_width_cm)
	SHADER_PARAMETER(float, screen_height_cm)
	SHADER_PARAMETER(float, distance_from_screen_cm)
	SHADER_PARAMETER(float, blur_rate_arcmin_per_degree)
	SHADER_PARAMETER(unsigned int, use_radially_increasing_blur)
	SHADER_PARAMETER(float, s_k)
	SHADER_PARAMETER(unsigned int, cells)
	SHADER_PARAMETER(unsigned int, impulses_per_cell)
	SHADER_PARAMETER(unsigned int, seed)

	SHADER_PARAMETER(float, time_seconds)
	SHADER_PARAMETER(float, phase_cycles_per_sec)
	SHADER_PARAMETER(float, phase_strength)
	SHADER_PARAMETER(unsigned int, region_mode) // 0 = FULLSCREEN, 1 = LEFT, 2 = RIGHT

END_SHADER_PARAMETER_STRUCT()
class FGaborNoiseEnhancementWithRerenderingCS : public FGlobalShader
{
public:
	DECLARE_EXPORTED_SHADER_TYPE(FGaborNoiseEnhancementWithRerenderingCS, Global, );
	using FParameters = FGaborNoiseEnhancementWithRerenderingParameters;
	SHADER_USE_PARAMETER_STRUCT(FGaborNoiseEnhancementWithRerenderingCS, FGlobalShader);
};
