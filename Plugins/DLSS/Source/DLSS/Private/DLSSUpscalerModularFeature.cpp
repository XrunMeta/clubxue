

#if ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE

#include "DLSSUpscalerModularFeature.h"

#include "DLSS.h"
#include "DLSSUpscaler.h"

#include "CoreGlobals.h"
#include "Features/IModularFeatures.h"
#include "LegacyScreenPercentageDriver.h"
#include "SceneView.h"
#include "SceneViewExtensionContext.h"
#include "UObject/EnumProperty.h"
#include "UObject/UnrealType.h"
#include "UObject/PropertyAccessUtil.h"

namespace DLSS::TemporalUpscalerModularFeature
{
	static auto constexpr DLSSModuleName = TEXT("DLSS");

	static IDLSSModuleInterface& GetAPI()
	{
		static IDLSSModuleInterface& DLSSModuleInterfaceAPI = FModuleManager::GetModuleChecked<IDLSSModuleInterface>(DLSSModuleName);

		return DLSSModuleInterfaceAPI;
	}

	static FDLSSUpscaler* GetUpscaler()
	{

		const EDLSSSupport DLSSSupport = GetAPI().QueryDLSSSRSupport();
		if (DLSSSupport == EDLSSSupport::Supported)
		{
			return GetAPI().GetDLSSUpscaler();
		}

		return nullptr;
	}

	static float GetGlobalScreenPercentage()
	{
		static const TConsoleVariableData<float>* CVarScreenPercentage = IConsoleManager::Get().FindTConsoleVariableDataFloat(TEXT("r.ScreenPercentage"));
		if (CVarScreenPercentage)
		{
			const float GlobalScreenPercentage = CVarScreenPercentage->GetValueOnGameThread() / 100.0f;
			if (GlobalScreenPercentage > 0)
			{
				return GlobalScreenPercentage;
			}
		}

		return 1.0f;
	}

	static const FProperty* FindPropertyByName(const FName InPropertyName, const FInstancedPropertyBag& InPropertyBag)
	{
		const FPropertyBagPropertyDesc* Desc = InPropertyBag.FindPropertyDescByName(InPropertyName);
		return Desc ? Desc->CachedProperty : nullptr;
	}

	static const FProperty* CopyContainerPropertyValue(const FProperty* InContainerProp, const void* InContainerData, FInstancedPropertyBag& DestPropertyBag)
	{
		const FProperty* TargetBagProperty = InContainerProp ? FindPropertyByName(InContainerProp->GetFName(), DestPropertyBag) : nullptr;
		if (!TargetBagProperty)
		{
			return nullptr;
		}

		const void* TargetBagValue = TargetBagProperty->ContainerPtrToValuePtr<void>(DestPropertyBag.GetValue().GetMemory());

		EPropertyAccessResultFlags Result = PropertyAccessUtil::GetPropertyValue_InContainer(
			InContainerProp, InContainerData,
			TargetBagProperty, const_cast<void*>(TargetBagValue),
			INDEX_NONE);

		return (Result == EPropertyAccessResultFlags::Success) ? TargetBagProperty : nullptr;
	}

	void CopyStructToPropertyBag(const UStruct* Struct, const void* StructData, FInstancedPropertyBag& OutPropertyBag)
	{

		TArray<FPropertyBagPropertyDesc> Properties;
		for (TFieldIterator<FProperty> PropIt(Struct); PropIt; ++PropIt)
		{
			if (FProperty* Property = *PropIt)
			{
				Properties.Add(FPropertyBagPropertyDesc(Property->GetFName(), Property));
			}
		}

		const UPropertyBag* NewBagStruct = UPropertyBag::GetOrCreateFromDescs(Properties);
		OutPropertyBag.MigrateToNewBagStruct(NewBagStruct);

		for (TFieldIterator<FProperty> PropIt(Struct); PropIt; ++PropIt)
		{
			if (FProperty* Property = *PropIt)
			{
				CopyContainerPropertyValue(Property, StructData, OutPropertyBag);
			}
		}
	}

	TOptional<bool> GetBoolPropertyByName(const FName& PropertyName, const FInstancedPropertyBag& InSettings)
	{
		TValueOrError<bool, EPropertyBagResult> PropertyResult = InSettings.GetValueBool(PropertyName);
		if (!PropertyResult.IsValid())
		{
			return TOptional<bool>();
		}

		const bool bPropertyValue = PropertyResult.GetValue();

		return bPropertyValue;
	}

	template<typename EnumClass>
	TOptional<EnumClass> GetEnumPropertyByName(const FName& PropertyName, const FInstancedPropertyBag& InSettings)
	{
		TValueOrError<uint8, EPropertyBagResult> EnumBagValue =
			InSettings.GetValueEnum(PropertyName, StaticEnum<EnumClass>());

		if (uint8* EnumValuePtr = EnumBagValue.TryGetValue())
		{
			const EnumClass EnumValue = static_cast<EnumClass>(*EnumValuePtr);

			return EnumValue;
		}

		return TOptional<EnumClass>();
	}
}

const FName& FDLSSTemporalUpscalerModularFeature::GetName() const
{
	static FName FeatureName(TEXT("DLSS"));

	return FeatureName;
}

const FText& FDLSSTemporalUpscalerModularFeature::GetDisplayName() const
{
	static const FText DisplayName = FText::FromString(TEXT("NVIDIA DLSS Super Resolution (DLSS-SR)"));

	return DisplayName;
}

const FText& FDLSSTemporalUpscalerModularFeature::GetTooltipText() const
{
	static const FText TooltipText = FText::FromString(TEXT("NVIDIA DLSS Super Resolution/DLAA"));

	return TooltipText;
}

bool FDLSSTemporalUpscalerModularFeature::IsFeatureEnabled() const
{
	using namespace DLSS::TemporalUpscalerModularFeature;

	return GetUpscaler() != nullptr;
}

bool FDLSSTemporalUpscalerModularFeature::AddSceneViewExtensionIsActiveFunctor(const FSceneViewExtensionIsActiveFunctor& IsActiveFunction)
{

	FSceneViewExtensionIsActiveFunctor& IsActiveFunctionCast = const_cast<FSceneViewExtensionIsActiveFunctor&>(IsActiveFunction);
	const FGuid& IsActiveFunctionGuid = IsActiveFunctionCast.GetGuid();

	const int32 ExistFunctionIndex = IsActiveThisFrameFunctions.IndexOfByPredicate([IsActiveFunctionGuid](const FSceneViewExtensionIsActiveFunctor& IsActiveFunctionIt)
		{

			FSceneViewExtensionIsActiveFunctor& IsActiveFunctionItCast = const_cast<FSceneViewExtensionIsActiveFunctor&>(IsActiveFunctionIt);
			const FGuid& ItemGuid = IsActiveFunctionItCast.GetGuid();

			return ItemGuid == IsActiveFunctionGuid;
		});

	if (ExistFunctionIndex != INDEX_NONE)
	{
		return false;
	}

	IsActiveThisFrameFunctions.Add(IsActiveFunction);

	return true;
}

bool FDLSSTemporalUpscalerModularFeature::RemoveSceneViewExtensionIsActiveFunctor(const FGuid& FunctorGuid)
{
	const int32 ExistFunctionIndex = IsActiveThisFrameFunctions.IndexOfByPredicate([FunctorGuid](const FSceneViewExtensionIsActiveFunctor& IsActiveFunctionIt)
		{

			FSceneViewExtensionIsActiveFunctor& IsActiveFunctionItCast = const_cast<FSceneViewExtensionIsActiveFunctor&>(IsActiveFunctionIt);
			const FGuid& ItemGuid = IsActiveFunctionItCast.GetGuid();

			return ItemGuid == FunctorGuid;
		});

	if (ExistFunctionIndex == INDEX_NONE)
	{

		return false;
	}

	IsActiveThisFrameFunctions.RemoveAt(ExistFunctionIndex);

	return true;
}

TOptional<bool> FDLSSTemporalUpscalerModularFeature::SceneViewExtensionIsActive(const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context) const
{
	TOptional<bool> IsActiveResult;

	for (const FSceneViewExtensionIsActiveFunctor& IsActiveFunctorIt : IsActiveThisFrameFunctions)
	{
		TOptional<bool> IsActiveFunctorItResult = IsActiveFunctorIt(SceneViewExtension, Context);
		if (IsActiveFunctorItResult.IsSet())
		{

			if (!IsActiveResult.IsSet() || *IsActiveResult == false)
			{
				IsActiveResult = *IsActiveFunctorItResult;
			}
		}
	}

	return IsActiveResult;
}

bool FDLSSTemporalUpscalerModularFeature::GetSettings(FInstancedPropertyBag& OutSettings) const
{
	using namespace DLSS::TemporalUpscalerModularFeature;
	static FDLSSUpscalerModularFeatureSettings DefaultUpscalerSettings;

	CopyStructToPropertyBag(
		DefaultUpscalerSettings.StaticStruct(),
		&DefaultUpscalerSettings,
		OutSettings);

	return true;
}

const FInstancedPropertyBag* FDLSSTemporalUpscalerModularFeature::GetCustomSettings(const FSceneView& View) const
{
	using namespace DLSS::TemporalUpscalerModularFeature;

	check(IsInGameThread());

	const uint32 ViewKey = View.GetViewKey();
	if (CustomSettings.Contains(ViewKey))
	{
		return &CustomSettings[ViewKey];
	}

	return nullptr;
}

const FInstancedPropertyBag* FDLSSTemporalUpscalerModularFeature::GetCustomSettings_RenderThread(const FSceneView& View) const
{
	using namespace DLSS::TemporalUpscalerModularFeature;

	check(IsInRenderingThread());

	const uint32 ViewKey = View.GetViewKey();
	if (CustomSettings_RenderThread.Contains(ViewKey))
	{
		return &CustomSettings_RenderThread[ViewKey];
	}

	return nullptr;
}

void FDLSSTemporalUpscalerModularFeature::SetupSceneView(
	const FInstancedPropertyBag& InUpscalerSettings,
	FSceneView& InOutView)
{

	InOutView.AntiAliasingMethod = EAntiAliasingMethod::AAM_TSR;
	InOutView.PrimaryScreenPercentageMethod = EPrimaryScreenPercentageMethod::TemporalUpscale;
}

bool FDLSSTemporalUpscalerModularFeature::PostConfigureViewFamily(
	const FInstancedPropertyBag& InUpscalerSettings,
	const FUpscalerModularFeatureParameters& InUpscalerParam,
	FSceneViewFamilyContext& InOutViewFamily)
{
	using namespace DLSS::TemporalUpscalerModularFeature;

	FDLSSUpscaler* DLSSUpscaler = GetUpscaler();
	if (!DLSSUpscaler)
	{
		return false;
	}

	int32 MaxPixelCount = 0;
	for (const FSceneView* View : InOutViewFamily.Views)
	{

		int32 PixelCount = View->UnscaledViewRect.Area();
		if (PixelCount > MaxPixelCount)
		{
			MaxPixelCount = PixelCount;
		}
	}
	const TOptional<EDLSSQualityMode> QualityMode = GetQualityMode(InUpscalerSettings, MaxPixelCount);
	if (!(QualityMode.IsSet() && DLSSUpscaler->IsQualityModeSupported(*QualityMode)))
	{
		return false;
	}

	const float OptimalResolutionFraction = DLSSUpscaler->GetOptimalResolutionFractionForQuality(*QualityMode);

	const float GlobalScreenPercentage = GetGlobalScreenPercentage();

	const float AdjustedGlobalResolutionFraction = OptimalResolutionFraction / GlobalScreenPercentage;

	InOutViewFamily.SecondaryViewFraction = InUpscalerParam.SecondaryScreenPercentage;

	InOutViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(
		InOutViewFamily, AdjustedGlobalResolutionFraction));

	if (LastFrameCounter != GFrameCounter)
	{
		LastFrameCounter = GFrameCounter;

		if (CustomSettings.Num() > 0)
		{
			CustomSettings.Reset();

			ENQUEUE_RENDER_COMMAND(DLSSTemporalUpscalerModularFeature_ClearData)(
				[WeakThis = AsWeak()](FRHICommandListImmediate& RHICmdList)
				{
					if (TSharedPtr<FDLSSTemporalUpscalerModularFeature> This = WeakThis.Pin())
					{
						This->CustomSettings_RenderThread.Reset();
					}
				});
		}
	}

	for (const FSceneView* ViewIt : InOutViewFamily.Views)
	{
		if (!ViewIt)
		{
			continue;
		}

		const uint32 ViewKey = ViewIt->GetViewKey();

		CustomSettings.FindOrAdd(ViewKey) = InUpscalerSettings;

		ENQUEUE_RENDER_COMMAND(DLSSTemporalUpscalerModularFeature_SetData)(
			[WeakThis = AsWeak(),
			ViewKey = ViewKey,
			UpscalerSettings = InUpscalerSettings](FRHICommandListImmediate& RHICmdList)
			{
				if (TSharedPtr<FDLSSTemporalUpscalerModularFeature> This = WeakThis.Pin())
				{
					This->CustomSettings_RenderThread.FindOrAdd(ViewKey) = UpscalerSettings;
				}
			});
	}

	return true;
}

TOptional<EDLSSQualityMode> FDLSSTemporalUpscalerModularFeature::GetQualityMode(const FInstancedPropertyBag& InSettings, int32 PixelCount)
{
	using namespace DLSS::TemporalUpscalerModularFeature;

	const TOptional<EDLSSUpscalerModularFeatureQuality> Quality =
		GetEnumPropertyByName<EDLSSUpscalerModularFeatureQuality>(FName("Quality"), InSettings);

	if (Quality.IsSet())
	{
		static_assert(int32(EDLSSUpscalerModularFeatureQuality::Count) == 7, "dear DLSS plugin NVIDIA developer, please update this code to handle the new EDLSSUpscalerModularFeatureQuality enum values");
		switch (*Quality)
		{
		case EDLSSUpscalerModularFeatureQuality::Auto:
		{
			FDLSSUpscaler* Upscaler = GetUpscaler();
			if (Upscaler != nullptr)
			{
				return Upscaler->GetAutoQualityModeFromPixels(PixelCount);
			}
		}
		case EDLSSUpscalerModularFeatureQuality::UltraQuality:     return EDLSSQualityMode::UltraQuality;
		case EDLSSUpscalerModularFeatureQuality::Quality:          return EDLSSQualityMode::Quality;
		case EDLSSUpscalerModularFeatureQuality::Balanced:         return EDLSSQualityMode::Balanced;
		case EDLSSUpscalerModularFeatureQuality::Performance:      return EDLSSQualityMode::Performance;
		case EDLSSUpscalerModularFeatureQuality::UltraPerformance: return EDLSSQualityMode::UltraPerformance;
		case EDLSSUpscalerModularFeatureQuality::DLAA:             return EDLSSQualityMode::DLAA;

		default:
			break;
		}
	}

	return NullOpt;
}

TSharedPtr<FDLSSTemporalUpscalerModularFeature, ESPMode::ThreadSafe> FDLSSTemporalUpscalerModularFeature::ModularFeatureSingleton;

void FDLSSTemporalUpscalerModularFeature::RegisterModularFeature()
{
	if (ModularFeatureSingleton.IsValid())
	{
		return;
	}

	ModularFeatureSingleton = MakeShared<FDLSSTemporalUpscalerModularFeature, ESPMode::ThreadSafe>();

	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	ModularFeatures.RegisterModularFeature(FDLSSTemporalUpscalerModularFeature::ModularFeatureName, ModularFeatureSingleton.Get());
}

void FDLSSTemporalUpscalerModularFeature::UnregisterModularFeature()
{
	if(!ModularFeatureSingleton.IsValid())
	{
		return;
	}

	IModularFeatures& ModularFeatures = IModularFeatures::Get();
	ModularFeatures.UnregisterModularFeature(FDLSSTemporalUpscalerModularFeature::ModularFeatureName, ModularFeatureSingleton.Get());

	ModularFeatureSingleton.Reset();
}

#endif 

