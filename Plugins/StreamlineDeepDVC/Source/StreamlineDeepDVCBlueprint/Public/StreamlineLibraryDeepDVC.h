

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

#include "StreamlineLibrary.h"

#include "StreamlineLibraryDeepDVC.generated.h"

#define UE_API STREAMLINEDEEPDVCBLUEPRINT_API

class FDelegateHandle;

#ifdef __INTELLISENSE__
#define WITH_STREAMLINE 1
#endif

UENUM(BlueprintType)
enum class EStreamlineDeepDVCMode : uint8
{
	Off UMETA(DisplayName = "Off"),
	On UMETA(DisplayName = "On"),
};

UCLASS(MinimalAPI)
class  UStreamlineLibraryDeepDVC : public UBlueprintFunctionLibrary
{
	friend class FStreamlineBlueprintModule;
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Is NVIDIA DeepDVC Supported"))
	static UE_API bool IsDeepDVCSupported();

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Query NVIDIA DeepDVC Support"))
	static UE_API EStreamlineFeatureSupport QueryDeepDVCSupport();

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Is DeepDVC Mode Supported"))
	static UE_API bool IsDeepDVCModeSupported(EStreamlineDeepDVCMode DeepDVCMode);

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Get Supported DeepDVC Modes"))
	static UE_API TArray<EStreamlineDeepDVCMode> GetSupportedDeepDVCModes();

	UFUNCTION(BlueprintCallable, Category = "Streamline|DeepDVC", meta = (DisplayName = "Set DeepDVC Mode"))
	static UE_API void SetDeepDVCMode(EStreamlineDeepDVCMode DeepDVCMode);

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Get DeepDVC Mode"))
	static UE_API EStreamlineDeepDVCMode GetDeepDVCMode();

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Get Default DeepDVC Mode"))
	static UE_API EStreamlineDeepDVCMode GetDefaultDeepDVCMode();

	UFUNCTION(BlueprintCallable, Category = "Streamline|DeepDVC", meta = (DisplayName = "Set DeepDVC Intensity"))
	static UE_API void SetDeepDVCIntensity(float Intensity);

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Get DeepDVC Intensity"))
	static UE_API float GetDeepDVCIntensity();

	UFUNCTION(BlueprintCallable, Category = "Streamline|DeepDVC", meta = (DisplayName = "Set DeepDVC  Saturation Boost"))
	static UE_API void SetDeepDVCSaturationBoost(float Intensity);

	UFUNCTION(BlueprintPure, Category = "Streamline|DeepDVC", meta = (DisplayName = "Get DeepDVC Saturation Boost"))
	static UE_API float GetDeepDVCSaturationBoost();

	static void Startup();
	static void Shutdown();
private:
	static EStreamlineFeatureSupport DeepDVCSupport;

#if WITH_STREAMLINE

	static bool bDeepDVCLibraryInitialized;

	static bool TryInitDeepDVCLibrary();

#if !UE_BUILD_SHIPPING
	struct FDLSSErrorState
	{
		bool bIsDeepDVCModeUnsupported = false;
		EStreamlineDeepDVCMode InvalidDeepDVCMode = EStreamlineDeepDVCMode::Off;
	};

	static FDLSSErrorState DLSSErrorState;

	static void GetDeepDVCOnScreenMessages(TMultiMap<FCoreDelegates::EOnScreenMessageSeverity, FText>& OutMessages);
	static FDelegateHandle DeepDVCOnScreenMessagesDelegateHandle;
#endif

#endif

};

class FStreamlineLibraryDeepDVCBlueprintModule final : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
};

#undef UE_API
