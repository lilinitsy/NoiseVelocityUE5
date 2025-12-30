#include "CustomComputeShaders.h"


// Example Shaders
IMPLEMENT_SHADER_TYPE(, FCombineShaderPS, TEXT("/CustomComputeShadersModule/ExampleShader.usf"), TEXT("CombineMainPS"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FUVMaskShaderPS, TEXT("/CustomComputeShadersModule/ExampleShader.usf"), TEXT("UVMaskMainPS"), SF_Pixel);


// Gaussian Blur
IMPLEMENT_SHADER_TYPE(, FGaussianBlurCS, TEXT("/CustomComputeShadersModule/GaussianBlur.usf"), TEXT("gaussian_blur_main_cs"), SF_Compute);


// Noise enhancement (runs after blurring or downsampled rendering)
IMPLEMENT_SHADER_TYPE(, FGaborNoiseEnhancementCS, TEXT("/CustomComputeShadersModule/GaborNoiseEnhancement.usf"), TEXT("gabor_foveated_enhance_cs"), SF_Compute);

// Don't need one for the gabor noise with reprojection since it's two shaders that use the noise enhancement and the noise reprojection
IMPLEMENT_SHADER_TYPE(, FNoiseReprojectionCS, TEXT("/CustomComputeShadersModule/noise_reprojection.usf"), TEXT("main_cs"), SF_Compute);


// Same entry point for rerendering noise enhancement
IMPLEMENT_SHADER_TYPE(, FGaborNoiseEnhancementWithRerenderingCS, TEXT("/CustomComputeShadersModule/GaborNoiseEnhancementSelectiveRerendering.usf"), TEXT("selective_rerender_noise_cs"), SF_Compute);
