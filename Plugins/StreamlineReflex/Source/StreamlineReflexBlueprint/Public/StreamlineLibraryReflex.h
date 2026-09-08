

#pragma once

#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"

#include "Kismet/BlueprintFunctionLibrary.h"

#include "StreamlineLibrary.h"

#include "StreamlineLibraryReflex.generated.h"

#define UE_API STREAMLINEREFLEXBLUEPRINT_API

#ifdef __INTELLISENSE__
#define WITH_STREAMLINE 1
#endif

UENUM(BlueprintType)
enum class EStreamlineReflexMode : uint8
{
	Off = 0 UMETA(DisplayName = "Off"),
	Enabled = 1 UMETA(DisplayName = "Enabled"),
	Boost = 3 UMETA(DisplayName = "Boost")
};

class FStreamlineLibraryImplementationBase
{
	protected:

	static EStreamlineFeatureSupport FeatureSupport;

#if WITH_STREAMLINE
	static bool bIsLibraryInitialized;
	static bool TryInitLibrary();
#endif
};

UCLASS(MinimalAPI)
class UStreamlineLibraryReflex : public UBlueprintFunctionLibrary, 
	public FStreamlineLibraryImplementationBase
{
public:
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Is NVIDIA Reflex Supported"))
	static UE_API bool IsReflexSupported();

	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Query NVIDIA Reflex Support"))
	static UE_API EStreamlineFeatureSupport QueryReflexSupport();

	UFUNCTION(BlueprintCallable, Category = "Streamline|Reflex", meta = (DisplayName = "Set Reflex mode"))
	static UE_API void SetReflexMode(const EStreamlineReflexMode Mode);

	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Get Reflex mode"))
	static UE_API EStreamlineReflexMode GetReflexMode();

	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Is Reflex Mode Supported"))
	static UE_API bool IsReflexModeSupported(EStreamlineReflexMode ReflexMode);

	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Get Supported Reflex Modes"))
	static UE_API TArray<EStreamlineReflexMode> GetSupportedReflexModes();

	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Get default Reflex mode"))
	static UE_API EStreamlineReflexMode GetDefaultReflexMode();

	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Get Reflex Game To Render Latency (ms)"))
	static UE_API float GetGameToRenderLatencyInMs();
	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Get Reflex Game Latency (ms)"))
	static UE_API float GetGameLatencyInMs();
	UFUNCTION(BlueprintPure, Category = "Streamline|Reflex", meta = (DisplayName = "Get Reflex Render Latency (ms)"))
	static UE_API float GetRenderLatencyInMs();

	static void Startup();
	static void Shutdown();
};

class FStreamlineLibraryReflexBlueprintModule final : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
};

#undef UE_API
