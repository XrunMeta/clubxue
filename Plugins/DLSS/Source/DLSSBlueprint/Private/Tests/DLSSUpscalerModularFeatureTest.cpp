

#if ENGINE_SUPPORTS_UPSCALER_MODULAR_FEATURE

#include "DLSSLibrary.h"

#include "Features/IModularFeatures.h"
#include "IUpscalerModularFeature.h"
#include "Misc/AutomationTest.h"
#include "StructUtils/PropertyBag.h"

#define TestNotNullExpr(...) TestNotNull(TEXT(#__VA_ARGS__), __VA_ARGS__)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDLSSTemporalUpscalerModularFeatureTest, "Nvidia.DLSS.ModularFeature",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext |
	EAutomationTestFlags::NonNullRHI | EAutomationTestFlags::ProductFilter
)

bool FDLSSTemporalUpscalerModularFeatureTest::RunTest(const FString& Parameters)
{
	using UE::VirtualProduction::IUpscalerModularFeature;

	bool bOriginalDLSSEnabled = UDLSSLibrary::IsDLSSEnabled();

	if (UDLSSLibrary::IsDLSSSupported())
	{
		UDLSSLibrary::EnableDLSS(false);
	}
	ON_SCOPE_EXIT
	{
		if (UDLSSLibrary::IsDLSSSupported())
		{
			UDLSSLibrary::EnableDLSS(bOriginalDLSSEnabled);
		}
	};

	IModularFeatures::FScopedLockModularFeatureList ModularFeatureListLock;
	const TArray<IUpscalerModularFeature*> UpscalerModularFeatures =
		IModularFeatures::Get().GetModularFeatureImplementations<IUpscalerModularFeature>(IUpscalerModularFeature::ModularFeatureName);
	IUpscalerModularFeature* const* DLSSFeaturePtr =
		UpscalerModularFeatures.FindByPredicate([](const IUpscalerModularFeature* UpscalerModularFeature)
	{
		return (UpscalerModularFeature != nullptr) &&
			UpscalerModularFeature->IsFeatureEnabled() &&
			UpscalerModularFeature->GetName() == FName("DLSS");
	});

	if (!UDLSSLibrary::IsDLSSSupported())
	{

		TestFalse("Found 'DLSS' modular feature when DLSS not supported", DLSSFeaturePtr && *DLSSFeaturePtr);
		return true;
	}

	if (TestTrue("Found 'DLSS' modular feature", DLSSFeaturePtr && *DLSSFeaturePtr))
	{
		IUpscalerModularFeature* DLSSFeature = *DLSSFeaturePtr;

		FInstancedPropertyBag PropBag;
		TestTrueExpr(DLSSFeature->GetSettings(PropBag));
		const FPropertyBagPropertyDesc* QualityPropDesc = PropBag.FindPropertyDescByName(FName("Quality"));
		if (TestNotNullExpr(QualityPropDesc))
		{
			TestEqual(TEXT("'Quality' property is enum type"), QualityPropDesc->ValueType, EPropertyBagPropertyType::Enum);
		}
	}

	UEnum* DLSSModeEnum = StaticEnum<UDLSSMode>();
	UTEST_NOT_NULL_EXPR(DLSSModeEnum);
	TArray<UDLSSMode> SupportedModes = UDLSSLibrary::GetSupportedDLSSModes();
	for (UDLSSMode SupportedMode : SupportedModes)
	{
		if (SupportedMode == UDLSSMode::Off)
		{

			continue;
		}
		FString ModeStr = DLSSModeEnum->GetNameStringByValue(static_cast<int64>(SupportedMode));
		FString ExpectedQualityModeStr = FString(TEXT("EDLSSUpscalerModularFeatureQuality::")) + ModeStr;
		UEnum* FoundEnum = nullptr;
		UEnum::LookupEnumName(TEXT("/Script/DLSS"), *ExpectedQualityModeStr,
			EFindFirstObjectOptions::None, &FoundEnum);
		TestNotNull(*(FString(TEXT("enum value ")) + ExpectedQualityModeStr), FoundEnum);
	}

	return true;
}

#endif	

