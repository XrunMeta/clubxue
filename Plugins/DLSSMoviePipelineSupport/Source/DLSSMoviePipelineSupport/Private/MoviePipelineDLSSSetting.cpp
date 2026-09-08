

#include "MoviePipelineDLSSSetting.h"

#include "DLSS.h"
#include "DLSSLibrary.h"
#include "MovieRenderPipelineDataTypes.h"
#include "SceneView.h"

#define LOCTEXT_NAMESPACE "MoviePipelineDLSSSetting"

namespace MRQHelpers
{
	UDLSSMode EMoviePipelineDLSSQualityToUDLSSMode(EMoviePipelineDLSSQuality InDLSSQuality)
	{

		uint8 OffsetDlssQuality = static_cast<uint8>(InDLSSQuality) + static_cast<uint8>(UDLSSMode::DLAA);
		return static_cast<UDLSSMode>(OffsetDlssQuality);
	}
}

UMoviePipelineDLSSSetting::UMoviePipelineDLSSSetting()
	: DLSSQuality(EMoviePipelineDLSSQuality::EMoviePipelineDLSSQuality_Performance)
{
}

void UMoviePipelineDLSSSetting::SetupViewFamily(FSceneViewFamily& ViewFamily)
{
	bool bIsSupported = false;
	if (ViewFamily.ViewMode == EViewModeIndex::VMI_Lit)
	{
		float OptimalScreenPercentage;
		{

			FVector2D DummyScreenRes{};
			bool bDummyFixed;
			float DummyMin, DummyMax, DummySharpness;
			UDLSSLibrary::GetDLSSModeInformation(MRQHelpers::EMoviePipelineDLSSQualityToUDLSSMode(DLSSQuality), DummyScreenRes,
				bIsSupported,
				OptimalScreenPercentage,
				bDummyFixed, DummyMin, DummyMax, DummySharpness);
		}

		if (bIsSupported)
		{

			static IConsoleVariable* CVarScreenPercentage = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ScreenPercentage"));
			if (CVarScreenPercentage != nullptr)
			{
				EConsoleVariableFlags Priority = static_cast<EConsoleVariableFlags>(CVarScreenPercentage->GetFlags() & ECVF_SetByMask);
				CVarScreenPercentage->Set(OptimalScreenPercentage, Priority);
			}

			IDLSSModuleInterface* DLSSModule = &FModuleManager::LoadModuleChecked<IDLSSModuleInterface>("DLSS");
			TSharedPtr<ISceneViewExtension> DLSSViewExt = DLSSModule->GetDLSSUpscalerViewExtension();
			if (DLSSViewExt.IsValid() && !ViewFamily.ViewExtensions.Contains(DLSSViewExt))
			{
				ViewFamily.ViewExtensions.Add(DLSSViewExt.ToSharedRef());
			}
		}
	}
	UDLSSLibrary::EnableDLSS(bIsSupported);
}

void UMoviePipelineDLSSSetting::GetFormatArguments(FMoviePipelineFormatArgs& InOutFormatArgs) const
{
	Super::GetFormatArguments(InOutFormatArgs);

	bool bIsCurrentModeSupported = UDLSSLibrary::IsDLSSModeSupported(MRQHelpers::EMoviePipelineDLSSQualityToUDLSSMode(DLSSQuality));

	if (bIsCurrentModeSupported)
	{
		InOutFormatArgs.FileMetadata.Add(TEXT("unreal/dlssQuality"), StaticEnum<EMoviePipelineDLSSQuality>()->GetDisplayNameTextByIndex((int64)DLSSQuality).ToString());
		InOutFormatArgs.FilenameArguments.Add(TEXT("dlss_quality"), StaticEnum<EMoviePipelineDLSSQuality>()->GetDisplayNameTextByIndex((int64)DLSSQuality).ToString());
	}
	else
	{
		InOutFormatArgs.FileMetadata.Add(TEXT("unreal/dlssQuality"), "Unsupported");
		InOutFormatArgs.FilenameArguments.Add(TEXT("dlss_quality"), "Unsupported");
	}
}

void UMoviePipelineDLSSSetting::ValidateStateImpl()
{
	Super::ValidateStateImpl();

	if (!UDLSSLibrary::IsDLSSSupported())
	{

		ValidationResults.Add(FText::Format(LOCTEXT("DLSS_Unsupported", "DLSS is not supported due to \"{0}\"."), StaticEnum<UDLSSSupport>()->GetDisplayNameTextByValue(int64(UDLSSLibrary::QueryDLSSSupport()))));
		ValidationState = EMoviePipelineValidationState::Warnings;

	}
	else if (!UDLSSLibrary::IsDLSSModeSupported(MRQHelpers::EMoviePipelineDLSSQualityToUDLSSMode(DLSSQuality)))
	{
		ValidationResults.Add(FText::Format(LOCTEXT("DLSS_Mode_Unsupported", "\"{0}\" Mode is not supported. Please try a lower quality setting."), StaticEnum<EMoviePipelineDLSSQuality>()->GetDisplayNameTextByIndex((int64)DLSSQuality)));
		ValidationState = EMoviePipelineValidationState::Warnings;
	}

}
