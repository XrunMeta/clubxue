

#pragma once

#include "Modules/ModuleManager.h"

#include "CoreMinimal.h"
#include "RendererInterface.h"
#include "RenderGraphResources.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Misc/EngineVersionComparison.h"
#include "RHIAccess.h"
#include "StreamlineNGXRHI.h"

#define UE_API STREAMLINERHI_API

namespace sl
{
	struct AdapterInfo;
	struct FrameToken;
	struct APIError;
	struct FeatureRequirements;
	using Feature = uint32_t;
	enum class FeatureRequirementFlags : uint32_t;
}

class FSLFrameTokenProvider;

enum class EStreamlineSupport : uint8
{
	Supported,
	NotSupported,
	NotSupportedIncompatibleRHI,
	NumValues
};

enum class EStreamlineResource
{
	Depth,
	MotionVectors,
	HUDLessColor,
	UIColorAndAlpha,
	Backbuffer,
	ScalingOutputColor,

	Last = ScalingOutputColor
};

class FRHIStreamlineResource
{
public:
	FRHITexture* Texture = nullptr;
	FIntRect ViewRect = FIntRect(FIntPoint::ZeroValue, FIntPoint::ZeroValue);
	EStreamlineResource StreamlineTag;
	ERHIAccess ResourceRHIAccess = ERHIAccess::Unknown;

#if !ENGINE_PROVIDES_UE_5_6_ID3D12DYNAMICRHI_METHODS
	FRHITexture* DebugLayerCompatibilityHelperSource = nullptr;
	FRHITexture* DebugLayerCompatibilityHelperDest = nullptr;
#endif 

	static FRHIStreamlineResource FromRDGTextureAccess(FRDGTextureAccess InRDGResource, EStreamlineResource InTag)
	{
		return FromRDGTexture(InRDGResource.GetTexture(), InRDGResource.GetAccess(), InTag);
	}

	static FRHIStreamlineResource FromRDGTextureAccess(FRDGTextureAccess InRDGResource, FIntRect InRect, EStreamlineResource InTag)
	{
		return FromRDGTexture(InRDGResource.GetTexture(), InRDGResource.GetAccess(), InRect, InTag);
	}

	static FRHIStreamlineResource NullResource(EStreamlineResource InTag)
	{
		return { nullptr, FIntRect(FIntPoint::ZeroValue, FIntPoint::ZeroValue), InTag , ERHIAccess::Unknown};
	}

private:
	static FRHIStreamlineResource FromRDGTexture(FRDGTexture* InRDGResource, ERHIAccess InResourceState, EStreamlineResource InTag)
	{
		return { InRDGResource ? InRDGResource->GetRHI() : nullptr,
			FIntRect(FIntPoint::ZeroValue,InRDGResource ? InRDGResource->Desc.Extent :FIntPoint::ZeroValue),
			InTag,
			   InRDGResource ? InResourceState : ERHIAccess::Unknown,
		};
	}

	static FRHIStreamlineResource FromRDGTexture(FRDGTexture* InRDGResource, ERHIAccess InResourceState, FIntRect InRect, EStreamlineResource InTag)
	{
		return { InRDGResource ? InRDGResource->GetRHI() : nullptr,	InRect,	InTag, InRDGResource ? InResourceState : ERHIAccess::Unknown };
	}

};

struct FRHIStreamlineArguments
{

#if ENGINE_MAJOR_VERSION < 5

	using FMatrix44f = FMatrix;
	using FVector2f = FVector2D;
	using FVector3f = FVector;
	using FVector4f = FVector4;
#else
	using FMatrix44f = ::FMatrix44f;
	using FVector3f = ::FVector3f;
	using FVector2f = ::FVector2f;
	using FVector4f = ::FVector4f;
#endif

	uint32 ViewId;

	uint32 FrameId;

	bool bReset;

	bool bIsDepthInverted;

	FVector2f JitterOffset;

	FVector2f MotionVectorScale;

	bool bAreMotionVectorsDilated;

	bool bIsOrthographicProjection;

	FMatrix44f CameraViewToClip;

	FMatrix44f ClipToCameraView;

	FMatrix44f ClipToLenseClip;

	FMatrix44f ClipToPrevClip;

	FMatrix44f PrevClipToClip;

	FVector3f CameraOrigin;

	FVector3f CameraUp;

	FVector3f CameraRight;

	FVector3f CameraForward;

	float CameraNear;

	float CameraFar;

	float CameraFOV;

	float CameraAspectRatio;

	FVector2f CameraPinholeOffset;

};

struct FStreamlineRHICreateArguments
{
	FString PluginBaseDir;
	FDynamicRHI* DynamicRHI = nullptr;
};

class FSLFrameTokenProvider
{
public:
	FSLFrameTokenProvider();

	sl::FrameToken* GetTokenForFrame(uint64 FrameCounter);

private:
	FCriticalSection Section;
	sl::FrameToken* FrameToken;
	uint32_t LastFrameCounter;
};

class  FStreamlineRHIModule;

class FStreamlineRHI
{

	friend class FStreamlineRHIModule;
public:
	UE_API virtual ~FStreamlineRHI();

	UE_API virtual void SetStreamlineData(FRHICommandList& CmdList, const FRHIStreamlineArguments& InArguments);
	UE_API void StreamlineEvaluateDeepDVC(FRHICommandList& CmdList, const FRHIStreamlineResource& InputOutput, sl::FrameToken* FrameToken, uint32 ViewID);

	void TagTextures(FRHICommandList& CmdList, uint32 InViewID, const sl::FrameToken& FrameToken, std::initializer_list< FRHIStreamlineResource> InResources, bool bIsValidUntilEval = false)
	{
		TagTextures(CmdList, InViewID, FrameToken, MakeArrayView(InResources), bIsValidUntilEval);
	}

	void TagTexture(FRHICommandList& CmdList, uint32 InViewID, const sl::FrameToken& FrameToken, const FRHIStreamlineResource& InResource, bool bIsValidUntilEval = false)
	{
		TagTextures(CmdList, InViewID, FrameToken, MakeArrayView<const FRHIStreamlineResource>(&InResource, 1), bIsValidUntilEval);
	}

public: 
	virtual void TagTextures(FRHICommandList& CmdList, uint32 InViewID, const sl::FrameToken& FrameToken, const TArrayView<const FRHIStreamlineResource> InResources, bool bIsValidUntilEval = false) = 0;
	virtual const sl::AdapterInfo* GetAdapterInfo() = 0;
	virtual void APIErrorHandler(const sl::APIError& LastError) const = 0;

	virtual bool IsPluginSideSwapchainProxyEnabled() const = 0;

protected:

	virtual void* GetCommandBuffer(FRHICommandList& CmdList, FRHITexture* Texture) = 0;
	virtual void PostStreamlineFeatureEvaluation(FRHICommandList& CmdList, FRHITexture* Texture) = 0;

	UE_API TTuple<bool, FString> IsSwapChainProviderRequired(const sl::AdapterInfo& AdapterInfo) const;
public:
	virtual bool IsDLSSGSupportedByRHI() const
	{
		return false;
	}

	virtual bool IsDeepDVCSupportedByRHI() const
	{
		return false;
	}

	virtual bool IsReflexSupportedByRHI() const
	{
		return false;
	}

	UE_API bool IsStreamlineAvailable() const;

	static bool IsIncompatibleAPICaptureToolActive()
	{
		return bIsIncompatibleAPICaptureToolActive;
	}

	UE_API sl::FrameToken* GetFrameToken(uint64 FrameCounter);
	UE_API bool IsSwapchainHookingAllowed() const;
	bool IsSwapchainProviderInstalled() const;
	UE_API void ReleaseStreamlineResourcesForAllFeatures(uint32 ViewID);

	void PostPlatformRHICreateInit();

	UE_API void OnSwapchainDestroyed(void* InNativeSwapchain, bool bIsKnownProxy = false) const;
	UE_API void OnSwapchainCreated(void* InNativeSwapchain, bool bIsKnownProxy = false) const;

#if !ENGINE_PROVIDES_UE_5_6_ID3D12DYNAMICRHI_METHODS
	UE_API virtual bool NeedExtraPassesForDebugLayerCompatibility();
#endif

#if WITH_EDITOR
	bool IsUnsupportedPIEActive() const { return bIsUnsupportedPIEActive; }
#endif

protected:

	UE_API FStreamlineRHI(const FStreamlineRHICreateArguments& Arguments);

#if WITH_EDITOR

	void OnBeginPIE(const bool bIsSimulating);
	void OnEndPIE(const bool bIsSimulating);
	bool bIsSupportedPIEActive = false;
	bool bIsUnsupportedPIEActive = false;
	FDelegateHandle BeginPIEHandle;
	FDelegateHandle EndPIEHandle;
#endif

	mutable int32 NumActiveSwapchainProxies = 0;
	virtual bool IsStreamlineSwapchainProxy(void* NativeSwapchain) const = 0;

	int32 GetMaxNumSwapchainProxies() const;
	void ValidateNumSwapchainProxies(const char* CallSite) const;
#if PLATFORM_WINDOWS

	UE_API bool IsDXGIStatus(const HRESULT HR) const ;
#endif

	FDynamicRHI* DynamicRHI = nullptr;
	TUniquePtr<FSLFrameTokenProvider> FrameTokenProvider = nullptr;

	static bool bIsIncompatibleAPICaptureToolActive;

	bool bIsSwapchainProviderInstalled = false;
	static TArray<sl::Feature> FeaturesRequestedAtSLInitTime;

	TArray<sl::Feature> LoadedFeatures;
	TArray<sl::Feature> SupportedFeatures;

};

class IStreamlineRHIModule : public IModuleInterface
{
public:

	virtual TUniquePtr<FStreamlineRHI> CreateStreamlineRHI(const FStreamlineRHICreateArguments& Arguments) = 0;
};

class FStreamlineRHIModule final : public IModuleInterface
{
public:
	UE_API void InitializeStreamline();
	UE_API void ShutdownStreamline();

	virtual void StartupModule();
	virtual void ShutdownModule();

private:
	FString StreamlineBinaryDirectory;
	FString StreamlineBinaryFlavor;
};

STREAMLINERHI_API void PlatformCreateStreamlineRHI();
STREAMLINERHI_API FStreamlineRHI* GetPlatformStreamlineRHI();
STREAMLINERHI_API EStreamlineSupport GetPlatformStreamlineSupport();
STREAMLINERHI_API bool IsStreamlineSupported();
STREAMLINERHI_API bool AreStreamlineFunctionsLoaded();

STREAMLINERHI_API sl::FeatureRequirementFlags PlatformGetAllImplementedStreamlineRHIs();

namespace sl
{

};

STREAMLINERHI_API bool StreamlineFilterRedundantSetOptionsCalls();
STREAMLINERHI_API void LogStreamlineFeatureSupport(sl::Feature Feature, const sl::AdapterInfo& Adapter);
STREAMLINERHI_API void LogStreamlineFeatureRequirements(sl::Feature Feature, const sl::FeatureRequirements& Requirements);
STREAMLINERHI_API FString CurrentThreadName();

STREAMLINERHI_API bool ShouldUseSlSetTag();
STREAMLINERHI_API bool ShouldUseSlateCallbacksForSwapchainTracking();

#undef UE_API
