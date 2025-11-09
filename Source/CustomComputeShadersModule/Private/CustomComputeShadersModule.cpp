#include "CustomComputeShadersModule.h"

void FCustomComputeShadersModule::StartupModule() {
	FString BaseDir = FPaths::Combine(FPaths::GameSourceDir(), TEXT("CustomComputeShadersModule"));
	FString ModuleShaderDir = FPaths::Combine(BaseDir, TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/CustomComputeShadersModule"), ModuleShaderDir);
}

IMPLEMENT_MODULE(FCustomComputeShadersModule, CustomComputeShadersModule)