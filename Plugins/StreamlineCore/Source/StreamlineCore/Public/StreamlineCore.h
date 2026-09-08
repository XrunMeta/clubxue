

#pragma once

#include "Modules/ModuleManager.h"

class FStreamlineViewExtension;
class FStreamlineMaxTickRateHandler;
class FStreamlineLatencyMarkers;

enum class EStreamlineSupport : uint8;
class FStreamlineRHI;

namespace Streamline
{
	enum class EStreamlineFeature
	{
		DLSSG,
		Reflex,
		DeepDVC,
		NumValues
	};

	enum class EStreamlineFeatureSupport
	{
		Supported,
		NotSupported,

		NotSupportedIncompatibleHardware,
		NotSupportedHardwareSchedulingDisabled,
		NotSupportedOperatingSystemOutOfDate,
		NotSupportedDriverOutOfDate,

		NotSupportedIncompatibleRHI,

		NumValues
	};
};

class IStreamlineModuleInterface : public IModuleInterface
{
public:

	virtual EStreamlineSupport QueryStreamlineSupport() const = 0;
	virtual Streamline::EStreamlineFeatureSupport QueryDLSSGSupport() const = 0;
	virtual Streamline::EStreamlineFeatureSupport QueryDeepDVCSupport() const = 0;
	virtual Streamline::EStreamlineFeatureSupport QueryReflexSupport() const = 0;
};

class FStreamlineCoreModule final: public IStreamlineModuleInterface
{
public:

	virtual void StartupModule();
	virtual void ShutdownModule();
	virtual EStreamlineSupport QueryStreamlineSupport() const override;
	virtual Streamline::EStreamlineFeatureSupport QueryDLSSGSupport() const override;
	virtual Streamline::EStreamlineFeatureSupport QueryDeepDVCSupport() const override;
	virtual Streamline::EStreamlineFeatureSupport QueryReflexSupport() const override;

	static STREAMLINECORE_API FStreamlineRHI* GetStreamlineRHI();
private:

	TSharedPtr< FStreamlineViewExtension, ESPMode::ThreadSafe> StreamlineViewExtension;
};
