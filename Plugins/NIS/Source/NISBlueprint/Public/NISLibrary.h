

#pragma once

#include "Modules/ModuleManager.h"

#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Misc/CoreDelegates.h"

#include "NISLibrary.generated.h"

#define UE_API NISBLUEPRINT_API

class FNISUpscaler;
class FDelegateHandle;

UENUM(BlueprintType)
enum class UNISSupport : uint8
{
	Supported UMETA(DisplayName = "Supported"),
	NotSupported UMETA(DisplayName = "Not Supported due to insufficient RHI Feature Level"),
};

UENUM(BlueprintType)
enum class UNISMode : uint8
{
	Off              UMETA(DisplayName = "Off"),
	UltraQuality     UMETA(DisplayName = "Ultra Quality"),
	Quality          UMETA(DisplayName = "Quality"),
	Balanced         UMETA(DisplayName = "Balanced"),
	Performance      UMETA(DisplayName = "Performance"),
	Custom           UMETA(DisplayName = "Custom")
};

UCLASS(MinimalAPI)
class  UNISLibrary : public UBlueprintFunctionLibrary
{
	friend class FNISBlueprintModule;
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintPure, Category = "NIS", meta = (DisplayName = "Is NVIDIA NIS Supported"))
	static UE_API bool IsNISSupported();

	UFUNCTION(BlueprintPure, Category = "NIS", meta = (DisplayName = "Is NIS Mode Supported"))
	static UE_API bool IsNISModeSupported(UNISMode NISMode);

	UFUNCTION(BlueprintPure, Category = "NIS", meta = (DisplayName = "Get Supported NIS Modes"))
	static UE_API TArray<UNISMode> GetSupportedNISModes();

	UFUNCTION(BlueprintPure, Category = "NIS", meta = (DisplayName = "Get NIS Recommended Screen Percentage"))
	static UE_API float GetNISRecommendedScreenPercentage(UNISMode NISMode);

	UFUNCTION(BlueprintPure, Category = "NIS", meta = (DisplayName = "Get NIS Screen Percentage Range"))
	static UE_API void GetNISScreenPercentageRange(float& MinScreenPercentage, float& MaxScreenPercentage);

	UFUNCTION(BlueprintCallable, Category = "NIS", meta = (DisplayName = "Set NIS Mode"))
	static UE_API void SetNISMode(UNISMode NISMode);

	UFUNCTION(BlueprintCallable, Category = "NIS", meta = (DisplayName = "Set NIS Custom Screen Percentage"))
	static UE_API void SetNISCustomScreenPercentage(float CustomScreenPercentage = 100.0f);

	UFUNCTION(BlueprintCallable, Category = "NIS", meta = (DisplayName = "Set NIS Sharpness"))
	static UE_API void SetNISSharpness(float Sharpness);

	UFUNCTION(BlueprintPure, Category = "NIS", meta = (DisplayName = "Get Default NIS Mode"))
	static UE_API UNISMode GetDefaultNISMode();

private:
	static UNISSupport NISSupport;
	static FNISUpscaler* NISUpscaler;
	static float SavedCustomScreenPercentage;
	static bool bIsCustomMode;
};

class FNISBlueprintModule final : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
};

#undef UE_API
