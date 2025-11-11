#pragma once

#include "SceneViewExtension.h"

class CUSTOMCOMPUTESHADERSMODULE_API FCustomComputeShadersViewExtension : public FSceneViewExtensionBase
{
public:
	FCustomComputeShadersViewExtension(const FAutoRegister& AutoRegister, FLinearColor CustomColor);

	virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override {};
	virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override {};
	virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;

	FLinearColor HighlightColor;

};
