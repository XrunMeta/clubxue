

#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

#include "StreamlineLibrary.h"

#include "StreamlineLibraryDLSSG.generated.h"

#define UE_API STREAMLINEDLSSGBLUEPRINT_API

class FDelegateHandle;

#ifdef __INTELLISENSE__
#define WITH_STREAMLINE 1
#endif

UENUM(BlueprintType)
enum class EStreamlineDLSSGMode : uint8
{

	Off  = 0   UMETA(DisplayName = "Off"),
	Auto = 251 UMETA(DisplayName = "Auto"),
	OnDynamic = 241 UMETA(DisplayName = "Dynamic"),
	On2X = 17  UMETA(DisplayName = "2X"),
	On3X = 23  UMETA(DisplayName = "3X"),
	On4X = 31  UMETA(DisplayName = "4X"),
	On5X = 37  UMETA(DisplayName = "5X"),
	On6X = 41  UMETA(DisplayName = "6X"),

};

UCLASS(MinimalAPI)
class  UStreamlineLibraryDLSSG : public UBlueprintFunctionLibrary
{
	friend class FStreamlineBlueprintModule;
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Is NVIDIA DLSS-FG Supported"))
	static UE_API bool IsDLSSGSupported();

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Query NVIDIA DLSS-FG Support"))
	static UE_API EStreamlineFeatureSupport QueryDLSSGSupport();

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Is DLSS-FG Mode Supported"))
	static UE_API bool IsDLSSGModeSupported(EStreamlineDLSSGMode DLSSGMode);

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Get Supported DLSS-FG Modes"))
	static UE_API TArray<EStreamlineDLSSGMode> GetSupportedDLSSGModes();

	UFUNCTION(BlueprintCallable, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Set DLSS-FG Mode"))
	static UE_API void SetDLSSGMode(EStreamlineDLSSGMode DLSSGMode);

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Get DLSS-FG Mode"))
	static UE_API EStreamlineDLSSGMode GetDLSSGMode();

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Get Default DLSS-FG Mode"))
	static UE_API EStreamlineDLSSGMode GetDefaultDLSSGMode();

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Get DLSS-FG  frame rate and presented frames"))
	static UE_API void GetDLSSGFrameTiming(float& FrameRateInHertz, int32& FramesPresented);

	UFUNCTION(BlueprintPure, Category = "Streamline|DLSS-FG", meta = (DisplayName = "Get DLSS-FG is vsync available"))
	static UE_API bool GetDLSSGIsVsyncSupportAvailable();

	static void Startup();
	static void Shutdown();
private:
	static EStreamlineFeatureSupport DLSSGSupport;

#if WITH_STREAMLINE

	static bool bDLSSGLibraryInitialized;

	static bool TryInitDLSSGLibrary();

#if !UE_BUILD_SHIPPING
	struct FDLSSErrorState
	{
		bool bIsDLSSGModeUnsupported = false;
		EStreamlineDLSSGMode InvalidDLSSGMode = EStreamlineDLSSGMode::Off;
	};

	static FDLSSErrorState DLSSErrorState;

	static void GetDLSSOnScreenMessages(TMultiMap<FCoreDelegates::EOnScreenMessageSeverity, FText>& OutMessages);
	static FDelegateHandle DLSSOnScreenMessagesDelegateHandle;
#endif

#endif
};

class FStreamlineLibraryDLSSGBlueprintModule final : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
};

#undef UE_API
