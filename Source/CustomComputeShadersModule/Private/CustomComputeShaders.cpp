#include "CustomComputeShaders.h"

IMPLEMENT_SHADER_TYPE(, FCombineShaderPS, TEXT("/CustomComputeShadersModule/ExampleShader.usf"), TEXT("CombineMainPS"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FUVMaskShaderPS, TEXT("/CustomComputeShadersModule/ExampleShader.usf"), TEXT("UVMaskMainPS"), SF_Pixel);
