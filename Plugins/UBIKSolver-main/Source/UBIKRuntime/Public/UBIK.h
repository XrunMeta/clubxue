

#pragma once

#include "Kismet/KismetMathLibrary.h"
#include "UBIK.generated.h"

USTRUCT(BlueprintType)
struct UBIKRUNTIME_API FUBIKSettings
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float DistinctShoulderRotationMultiplier = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float DistinctShoulderRotationLimit = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ClavicleOffset = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowBaseOffsetAngle = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowYDistanceStart = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowYWeight = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowHandsRotSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowRotFromHandRotAlpha = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float HeadHandAngleLimit = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float OkSpanAngle = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    FVector BaseCharOffset = FVector(0.f, 0.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float BodyInterSpeed = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (ToolTip = ""))
    float HeadHandAngleLimitDot = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ArmLength = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float UpperArmLength = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float LowerArmLength = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float UpperArmsDistance = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    FVector LocalHandOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    FRotator LocalHandRotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ShoulderHeadHandAlpha = 0.f;
};

USTRUCT(BlueprintType)
struct UBIKRUNTIME_API FUBIKDefaults
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float DistinctShoulderRotationMultiplier = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float DistinctShoulderRotationLimit = 45.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ClavicleOffset = -32.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowBaseOffsetAngle = 90.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowYDistanceStart = .2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowYWeight = 130.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowHandsRotSpeed = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ElbowRotFromHandRotAlpha = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float HeadHandAngleLimit = 150.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float OkSpanAngle = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    FVector BaseCharOffset = FVector(0.f, 0.f, -55.25f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float BodyInterSpeed = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    FVector LocalHandOffset = FVector(14.f, -2.f, 0.f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    FRotator LocalHandRotationOffset = FRotator(0.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float ShoulderHeadHandAlpha = 0.75f;
};

USTRUCT(BlueprintType)
struct UBIKRUNTIME_API FUBIKCalibrationData
{
    GENERATED_USTRUCT_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
    float Height = 184.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault, ToolTip = ""))
    float UpperArmsDistance = 30.f;
};

UCLASS()
class UBIKRUNTIME_API UUBIK : public UObject
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetUBIKSettings", Keywords = "UBIK Calibrate"), Category = "UBIK")
    static FUBIKSettings Initialize(FUBIKDefaults Defaults, FUBIKCalibrationData Calibration);

    static FTransform AddLocalOffset(const FTransform Transform, const FVector Offset);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "RotatePointAroundPivot", Keywords = "UBIK Rotate Pivot"), Category = "UBIK Utility")
    static FTransform RotatePointAroundPivot(FTransform Point, FTransform Pivot, FRotator Delta);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "SafeguardAngle", Keywords = "UBIK Safeguard Angle"), Category = "UBIK Utility")
    static float SafeguardAngle(float Last, float Current, float Threshold);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "CosineRule", Keywords = "UBIK Cosine"), Category = "UBIK Utility")
    static float CosineRule(float Adjacent1, float Adjacent2, float Opposite);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "FindBetweenNormals", Keywords = "UBIK Cosine"), Category = "UBIK Utility")
    static FRotator FindBetweenNormals(const FVector& A, const FVector& B);
};
