

#pragma once

#include "CoreMinimal.h"

#include "VRBPDatatypes.h"
#include "Engine/DataAsset.h"
#include "Components/SceneComponent.h"

#include "TimerManager.h"
#include "VRGestureComponent.generated.h"

DECLARE_STATS_GROUP(TEXT("TICKGesture"), STATGROUP_TickGesture, STATCAT_Advanced);

class USplineMeshComponent;
class USplineComponent;
class AVRBaseCharacter;

UENUM(Blueprintable)
enum class EVRGestureState : uint8
{
	GES_None,
	GES_Recording,
	GES_Detecting
};

UENUM(Blueprintable)
enum class EVRGestureMirrorMode : uint8
{
	GES_NoMirror,
	GES_MirrorLeft,
	GES_MirrorRight,
	GES_MirrorBoth
};

UENUM(Blueprintable)
enum class EVRGestureFlattenAxis : uint8
{
	GES_FlattenX,
	GES_FlattenY,
	GES_FlattenZ,
	GES_DontFlatten
};

USTRUCT(BlueprintType, Category = "VRGestures")
struct VREXPANSIONPLUGIN_API FVRGestureSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced")
		int Minimum_Gesture_Length;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced")
		float firstThreshold;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced")
		float FullThreshold;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced")
		EVRGestureMirrorMode MirrorMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced")
		bool bEnabled;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture|Advanced")
		bool bEnableScaling;

	FVRGestureSettings()
	{
		Minimum_Gesture_Length = 1;
		firstThreshold = 20.0f;
		FullThreshold = 20.0f;
		MirrorMode = EVRGestureMirrorMode::GES_NoMirror;
		bEnabled = true;
		bEnableScaling = true;
	}
};

USTRUCT(BlueprintType, Category = "VRGestures")
struct VREXPANSIONPLUGIN_API FVRGesture
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
	FString Name;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
	uint8 GestureType;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "VRGesture")
	TArray<FVector> Samples;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "VRGesture")
	FBox GestureSize;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGesture")
		FVRGestureSettings GestureSettings;

	FVRGesture()
	{
		GestureType = 0;
		GestureSize = FBox();
	}

	void CalculateSizeOfGesture(bool bAllowResizing = false, float TargetExtentSize = 1.f)
	{
		FVector NewSample;
		for (int i = 0; i < Samples.Num(); ++i)
		{
			NewSample = Samples[i];
			GestureSize.Max.X = FMath::Max(NewSample.X, GestureSize.Max.X);
			GestureSize.Max.Y = FMath::Max(NewSample.Y, GestureSize.Max.Y);
			GestureSize.Max.Z = FMath::Max(NewSample.Z, GestureSize.Max.Z);

			GestureSize.Min.X = FMath::Min(NewSample.X, GestureSize.Min.X);
			GestureSize.Min.Y = FMath::Min(NewSample.Y, GestureSize.Min.Y);
			GestureSize.Min.Z = FMath::Min(NewSample.Z, GestureSize.Min.Z);
		}

		if (bAllowResizing)
		{
			FVector BoxSize = GestureSize.GetSize();
			float Scaler = TargetExtentSize / BoxSize.GetMax();

			for (int i = 0; i < Samples.Num(); ++i)
			{
				Samples[i] *= Scaler;
			}

			GestureSize.Min *= Scaler;
			GestureSize.Max *= Scaler;
		}
	}
};

UCLASS(BlueprintType, Category = "VRGestures")
class VREXPANSIONPLUGIN_API UGesturesDatabase : public UDataAsset
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
	TArray <FVRGesture> Gestures;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		float TargetGestureScale;

	UGesturesDatabase()
	{
		TargetGestureScale = 100.0f;
	}

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void RecalculateGestures(bool bScaleToDatabase = true);

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void FillSplineWithGesture(UPARAM(ref)FVRGesture &Gesture, USplineComponent * SplineComponent, bool bCenterPointsOnSpline = true, bool bScaleToBounds = false, float OptionalBounds = 0.0f, bool bUseCurvedPoints = true, bool bFillInSplineMeshComponents = true, UStaticMesh * Mesh = nullptr, UMaterial * MeshMat = nullptr);

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		bool ImportSplineAsGesture(USplineComponent * HostSplineComponent, FString GestureName, bool bKeepSplineCurves = true, float SegmentLen = 10.0f, bool bScaleToDatabase = true);

};

USTRUCT(BlueprintType, Category = "VRGestures")
struct VREXPANSIONPLUGIN_API FVRGestureSplineDraw
{
	GENERATED_BODY()
public:

	UPROPERTY()
		TObjectPtr<USplineComponent> SplineComponent;

	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> SplineMeshes;

	int LastIndexSet;
	int NextIndexCleared;

	void ClearLastPoint();

	void Reset();

	void Clear();

	FVRGestureSplineDraw();

	~FVRGestureSplineDraw();
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FVRGestureDetectedSignature, uint8, GestureType, FString, DetectedGestureName, int, DetectedGestureIndex, UGesturesDatabase *, GestureDataBase, FVector, OriginalUnscaledGestureSize);

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent), ClassGroup = (VRExpansionPlugin))
class VREXPANSIONPLUGIN_API UVRGestureComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UVRGestureComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintImplementableEvent, Category = "BaseVRCharacter")
		void OnGestureDetected(uint8 GestureType, FString &DetectedGestureName, int & DetectedGestureIndex, UGesturesDatabase * GestureDatabase, FVector OriginalUnscaledGestureSize);

	UPROPERTY(BlueprintAssignable, Category = "VRGestures")
		FVRGestureDetectedSignature OnGestureDetected_Bind;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		TObjectPtr<UGesturesDatabase> GesturesDB;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		float SameSampleTolerance;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		EVRGestureMirrorMode MirroringHand;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		TObjectPtr<AVRBaseCharacter> TargetCharacter;

	FVRGestureSplineDraw RecordingGestureDraw;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		bool bDrawSplinesCurved;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		bool bGetGestureInWorldSpace;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		TObjectPtr<UStaticMesh> SplineMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		FVector2D SplineMeshScaler;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
		TObjectPtr<UMaterialInterface> SplineMaterial;

	float RecordingDelta;

	int RecordingBufferSize;

	float RecordingClampingTolerance = 0.0f;
	EVRGestureFlattenAxis RecordingFlattenAxis = EVRGestureFlattenAxis::GES_DontFlatten;
	bool bDrawRecordingGesture = false;
	bool bDrawRecordingGestureAsSpline = false;
	bool bGestureChanged = false;

	FTimerHandle TickGestureTimer_Handle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VRGestures")
	int maxSlope;

	UPROPERTY(BlueprintReadOnly, Category = "VRGestures")
	EVRGestureState CurrentState;

	UPROPERTY(BlueprintReadOnly, Category = "VRGestures")
	FVRGesture GestureLog;

	inline float GetGestureDistance(FVector Seq1, FVector Seq2, bool bMirrorGesture = false)
	{
		if (bMirrorGesture)
		{
			return FVector::DistSquared(Seq1, FVector(Seq2.X, -Seq2.Y, Seq2.Z));
		}

		return FVector::DistSquared(Seq1, Seq2);
	}

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void RecalculateGestureSize(UPARAM(ref) FVRGesture & InputGesture, UGesturesDatabase * GestureDB);

	UFUNCTION(BlueprintCallable, Category = "VRGestures", meta = (WorldContext = "WorldContextObject"))
		void DrawDebugGesture(UObject* WorldContextObject, UPARAM(ref)FTransform& StartTransform, FVRGesture GestureToDraw, FColor const& Color, bool bPersistentLines = false, uint8 DepthPriority = 0, float LifeTime = -1.f, float Thickness = 0.f);

	FVector StartVector;
	FTransform OriginatingTransform;
	FTransform ParentRelativeTransform;

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void BeginRecording(bool bRunDetection, EVRGestureFlattenAxis FlattenAxis = EVRGestureFlattenAxis::GES_FlattenX, bool bDrawGesture = true, bool bDrawAsSpline = false, int SamplingHTZ = 30, int SampleBufferSize = 60, float ClampingTolerance = 0.01f);

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		FVRGesture EndRecording();

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void ClearRecording();

	UFUNCTION(BlueprintCallable, Category = "VRGestures")
		void SaveRecording(UPARAM(ref) FVRGesture &Recording, FString RecordingName, bool bScaleRecordingToDatabase = true);

	void CaptureGestureFrame();

	void TickGesture();

	void RecognizeGesture(FVRGesture inputGesture);

	float dtw(FVRGesture seq1, FVRGesture seq2, bool bMirrorGesture = false, float Scaler = 1.f);

};

