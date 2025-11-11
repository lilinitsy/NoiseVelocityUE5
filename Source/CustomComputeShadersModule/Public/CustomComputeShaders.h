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

	SHADER_PARAMETER(float, radius_fovea)
	SHADER_PARAMETER(float, radius_periphery)
	
	SHADER_PARAMETER(float, screen_width_cm)
	SHADER_PARAMETER(float, screen_height_cm)
	SHADER_PARAMETER(float, distance_from_screen)
	SHADER_PARAMETER(FVector2f, foveation_center)
END_SHADER_PARAMETER_STRUCT()
class FGaussianBlurCS : public FGlobalShader
{
	public:
		DECLARE_EXPORTED_SHADER_TYPE(FGaussianBlurCS, Global, );
		using FParameters = FGaussianBlurShaderParameters;
		SHADER_USE_PARAMETER_STRUCT(FGaussianBlurCS, FGlobalShader);
};