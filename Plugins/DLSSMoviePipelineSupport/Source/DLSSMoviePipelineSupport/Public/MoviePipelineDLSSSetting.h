

#pragma once

#include "MoviePipelineViewFamilySetting.h"
#include "MoviePipelineDLSSSetting.generated.h"

class FSceneViewFamily;

UENUM(BlueprintType)
enum class EMoviePipelineDLSSQuality : uint8
{
	EMoviePipelineDLSSQuality_DLAA					UMETA(DisplayName = "DLAA"),
	EMoviePipelineDLSSQuality_UltraQuality			UMETA(DisplayName = "Ultra Quality"),
	EMoviePipelineDLSSQuality_Quality				UMETA(DisplayName = "Quality"),
	EMoviePipelineDLSSQuality_Balanced				UMETA(DisplayName = "Balanced"),
	EMoviePipelineDLSSQuality_Performance			UMETA(DisplayName = "Performance"),
	EMoviePipelineDLSSQuality_UltraPerformance		UMETA(DisplayName = "Ultra Performance"),
};

UCLASS(MinimalAPI, BlueprintType)
class UMoviePipelineDLSSSetting : public UMoviePipelineViewFamilySetting
{
	GENERATED_BODY()
public:
	UMoviePipelineDLSSSetting();

public:
#if WITH_EDITOR
	virtual FText GetDisplayText() const override { return NSLOCTEXT("MovieRenderPipeline", "DlssSettingDisplayName", "DLSS/DLAA"); }
#endif

	virtual void SetupViewFamily(FSceneViewFamily& ViewFamily) override;

	virtual void GetFormatArguments(FMoviePipelineFormatArgs& InOutFormatArgs) const override;

	virtual void ValidateStateImpl() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DLSS/DLAA settings", DisplayName = "DLSS Quality")
	EMoviePipelineDLSSQuality DLSSQuality;
};