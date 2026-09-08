

#pragma once
#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "PhysicsPublic.h"
#include "Engine/World.h"
#include "Engine/NetDriver.h"

#include "PhysicsEngine/ConstraintTypes.h"
#include "PhysicsEngine/ConstraintDrives.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "VRBPDatatypes.generated.h"

class UGripMotionControllerComponent;
class UVRGripScriptBase;
class UPrimitiveComponent;

namespace IRISNetReplication
{
	FORCEINLINE bool IsIris(UObject* WorldContext)
	{
		if (!WorldContext) return false;

		if (UWorld* World = WorldContext->GetWorld())
		{
			if (UNetDriver* NetDriver = World->GetNetDriver())
			{
				return NetDriver->IsUsingIrisReplication();
			}
		}

		return false;
	}
}

UENUM(BlueprintType)
enum class EVRCustomMovementMode : uint8
{
	VRMOVE_Climbing UMETA(DisplayName = "Climbing"),
	VRMOVE_LowGrav  UMETA(DisplayName = "LowGrav"),
	VRMOVE_Seated UMETA(DisplayName = "Seated"),
	VRMOVE_SplineFollow UMETA(DisplayName = "SplineFollow")

};

UENUM(BlueprintType)
enum class EVRConjoinedMovementModes : uint8
{
	C_MOVE_None	= 0x00	UMETA(DisplayName = "None"),
	C_MOVE_Walking = 0x01	UMETA(DisplayName = "Walking"),
	C_MOVE_NavWalking = 0x02	UMETA(DisplayName = "Navmesh Walking"),
	C_MOVE_Falling = 0x03	UMETA(DisplayName = "Falling"),
	C_MOVE_Swimming = 0x04	UMETA(DisplayName = "Swimming"),
	C_MOVE_Flying = 0x05		UMETA(DisplayName = "Flying"),

	C_MOVE_MAX = 0x07		UMETA(Hidden),
	C_VRMOVE_Climbing = 0x08 UMETA(DisplayName = "Climbing"),
	C_VRMOVE_LowGrav = 0x09 UMETA(DisplayName = "LowGrav"),

	C_VRMOVE_Seated = 0x0A UMETA(DisplayName = "Seated"),
	C_VRMOVE_SplineFollow = 0x0B UMETA(DisplayName = "SplineFollow"), 

	C_VRMOVE_Custom1 = 0x1A UMETA(DisplayName = "Custom1"),
	C_VRMOVE_Custom2 = 0x1B UMETA(DisplayName = "Custom2"),
	C_VRMOVE_Custom3 = 0x1C UMETA(DisplayName = "Custom3"),
	C_VRMOVE_Custom4 = 0x1D UMETA(DisplayName = "Custom4"),
	C_VRMOVE_Custom5 = 0x1E UMETA(DisplayName = "Custom5"),
	C_VRMOVE_Custom6 = 0x1F UMETA(DisplayName = "Custom6"),
	C_VRMOVE_Custom7 = 0x20 UMETA(DisplayName = "Custom7"),
	C_VRMOVE_Custom8 = 0x21 UMETA(DisplayName = "Custom8"),
	C_VRMOVE_Custom9 = 0x22 UMETA(DisplayName = "Custom9"),
	C_VRMOVE_Custom10 = 0x23 UMETA(DisplayName = "Custom10")
};

UENUM()
enum class EBPVRResultSwitch : uint8
{

	OnSucceeded,

	OnFailed
};

UENUM(BlueprintType)
enum class EVRClientAuthConflictResolutionMode : uint8
{

	VRGRIP_CONFLICT_None,

	VRGRIP_CONFLICT_First,

	VRGRIP_CONFLICT_Last,

	VRGRIP_CONFLICT_DropAll
};

UENUM(Blueprintable)
enum class EBPVRWaistTrackingMode : uint8
{

	VRWaist_Tracked_Front,

	VRWaist_Tracked_Rear,

	VRWaist_Tracked_Left,

	VRWaist_Tracked_Right
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPVRWaistTracking_Info
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
		FRotator RestingRotation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
		float WaistRadius;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
	EBPVRWaistTrackingMode TrackingMode;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Settings")
		TObjectPtr<UPrimitiveComponent> TrackedDevice;

	bool IsValid()
	{
		return TrackedDevice != nullptr;
	}

	void Clear()
	{
		TrackedDevice = nullptr;
	}

	FBPVRWaistTracking_Info():
		RestingRotation(FRotator::ZeroRotator),
		WaistRadius(0.0f),
		TrackingMode(EBPVRWaistTrackingMode::VRWaist_Tracked_Rear),
		TrackedDevice(nullptr)
	{}

};

UENUM(BlueprintType)
enum class EVRLerpInterpolationMode : uint8
{

	QuatInterp,

	EulerInterp,

	DualQuatInterp
};

template<class filterType>
class FBasicLowPassFilter
{
public:

	FBasicLowPassFilter(filterType EmptyValueSet)
	{
		EmptyValue = EmptyValueSet;
		Previous = EmptyValue;
		PreviousRaw = EmptyValue;
		bFirstTime = true;
	}

	filterType Filter(const filterType& InValue, const filterType& InAlpha)
	{

		filterType Result = InValue;
		if (!bFirstTime)
		{

			for (int i = 0; i < sizeof(filterType) / sizeof(double); i++)
			{
				((double*)&Result)[i] = ((double*)&InAlpha)[i] * ((double*)&InValue)[i] + (1.0f - ((double*)&InAlpha)[i]) * ((double*)&Previous)[i];
			}
		}

		bFirstTime = false;
		Previous = Result;
		PreviousRaw = InValue;
		return Result;
	}

	filterType EmptyValue;

	filterType Previous;

	filterType PreviousRaw;

	bool bFirstTime;

	const filterType CalculateCutoff(const filterType& InValue, double& MinCutoff, double& CutoffSlope)
	{
		filterType Result;

		for (int i = 0; i < sizeof(filterType) / sizeof(double); i++)
		{
			((double*)&Result)[i] = MinCutoff + CutoffSlope * FMath::Abs(((double*)&InValue)[i]);
		}
		return Result;
	}

	const filterType CalculateAlpha(const filterType& InCutoff, const double InDeltaTime)
	{
		filterType Result;

		for (int i = 0; i < sizeof(filterType) / sizeof(double); i++)
		{
			((double*)&Result)[i] = CalculateAlphaTau(((double*)&InCutoff)[i], InDeltaTime);
		}
		return Result;
	}

	inline const double CalculateAlphaTau(const double InCutoff, const double InDeltaTime)
	{
		const double tau = 1.0 / (2.0 * PI * InCutoff);
		return 1.0 / (1.0 + tau / InDeltaTime);
	}
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPEuroLowPassFilter
{
	GENERATED_BODY()
public:

	FBPEuroLowPassFilter() :
		MinCutoff(0.9),
		DeltaCutoff(1.0),
		CutoffSlope(0.007),
		RawFilter(FVector::ZeroVector),
		DeltaFilter(FVector::ZeroVector)
	{}

	FBPEuroLowPassFilter(const double InMinCutoff, const double InCutoffSlope, const double InDeltaCutoff) :
		MinCutoff(InMinCutoff),
		DeltaCutoff(InDeltaCutoff),
		CutoffSlope(InCutoffSlope),
		RawFilter(FVector::ZeroVector),
		DeltaFilter(FVector::ZeroVector)
	{}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double MinCutoff;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double DeltaCutoff;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double CutoffSlope;

	void ResetSmoothingFilter();

	FVector RunFilterSmoothing(const FVector& InRawValue, const float& InDeltaTime);

private:

	FBasicLowPassFilter<FVector> RawFilter;
	FBasicLowPassFilter<FVector> DeltaFilter;

};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPEuroLowPassFilterQuat
{
	GENERATED_BODY()
public:

	FBPEuroLowPassFilterQuat() :
		MinCutoff(0.9),
		DeltaCutoff(1.0),
		CutoffSlope(0.007),
		RawFilter(FQuat::Identity),
		DeltaFilter(FQuat::Identity)
	{}

	FBPEuroLowPassFilterQuat(const double InMinCutoff, const double InCutoffSlope, const double InDeltaCutoff) :
		MinCutoff(InMinCutoff),
		DeltaCutoff(InDeltaCutoff),
		CutoffSlope(InCutoffSlope),
		RawFilter(FQuat::Identity),
		DeltaFilter(FQuat::Identity)
	{}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double MinCutoff;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double DeltaCutoff;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double CutoffSlope;

	void ResetSmoothingFilter();

	FQuat RunFilterSmoothing(const FQuat& InRawValue, const float& InDeltaTime);

private:

	FBasicLowPassFilter<FQuat> RawFilter;
	FBasicLowPassFilter<FQuat> DeltaFilter;

};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPEuroLowPassFilterTrans
{
	GENERATED_BODY()
public:

	FBPEuroLowPassFilterTrans() :
		MinCutoff(0.1),
		DeltaCutoff(10.0),
		CutoffSlope(10.0),
		RawFilter(FTransform::Identity),
		DeltaFilter(FTransform::Identity)
	{}

	FBPEuroLowPassFilterTrans(const double InMinCutoff, const double InCutoffSlope, const double InDeltaCutoff) :
		MinCutoff(InMinCutoff),
		DeltaCutoff(InDeltaCutoff),
		CutoffSlope(InCutoffSlope),
		RawFilter(FTransform::Identity),
		DeltaFilter(FTransform::Identity)
	{}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double MinCutoff;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double DeltaCutoff;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FilterSettings")
		double CutoffSlope;

	void ResetSmoothingFilter();

	FTransform RunFilterSmoothing(const FTransform& InRawValue, const float& InDeltaTime);

private:

	FBasicLowPassFilter<FTransform> RawFilter;
	FBasicLowPassFilter<FTransform> DeltaFilter;

};

UENUM(BlueprintType)
enum class EVRVelocityType : uint8
{

	VRLOCITY_Default UMETA(DisplayName = "Default"),

	VRLOCITY_RunningAverage  UMETA(DisplayName = "Running Average"),

	VRLOCITY_SamplePeak UMETA(DisplayName = "Sampled Peak")
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPLowPassPeakFilter
{
	GENERATED_BODY()
public:

	FBPLowPassPeakFilter() :
		VelocitySamples(30),
		VelocitySampleLogCounter(0)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Samples")
		int32 VelocitySamples;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Samples")
	TArray<FVector>VelocitySampleLog;

	int32 VelocitySampleLogCounter;

	void Reset()
	{
		VelocitySampleLog.Reset(VelocitySamples);
	}

	void AddSample(FVector NewSample)
	{
		if (VelocitySamples <= 0)
			return;

		if (VelocitySampleLog.Num() != VelocitySamples)
		{
			VelocitySampleLog.Reset(VelocitySamples);
			VelocitySampleLog.AddZeroed(VelocitySamples);
			VelocitySampleLogCounter = 0;
		}

		VelocitySampleLog[VelocitySampleLogCounter] = NewSample;
		++VelocitySampleLogCounter;

		if (VelocitySampleLogCounter >= VelocitySamples)
			VelocitySampleLogCounter = 0;
	}

	FVector GetPeak() const
	{
		FVector MaxValue = FVector::ZeroVector;
		float ValueSizeSq = 0.f;
		float CurSizeSq = 0.f;

		for (int i = 0; i < VelocitySampleLog.Num(); i++)
		{
			CurSizeSq = VelocitySampleLog[i].SizeSquared();
			if (CurSizeSq > ValueSizeSq)
			{
				MaxValue = VelocitySampleLog[i];
				ValueSizeSq = CurSizeSq;
			}
		}

		return MaxValue;
	}
};

namespace TransNetQuant
{
	static const float MinimumQ = -1.0f / 1.414214f;
	static const float MaximumQ = +1.0f / 1.414214f;
	static const float MinMaxQDiff = TransNetQuant::MaximumQ - TransNetQuant::MinimumQ;
}

USTRUCT(BlueprintType, Category = "VRExpansionLibrary|TransformNetQuantize", meta = (HasNativeMake = "/Script/VRExpansionPlugin.VRExpansionFunctionLibrary.MakeTransform_NetQuantize", HasNativeBreak = "/Script/VRExpansionPlugin.VRExpansionFunctionLibrary.BreakTransform_NetQuantize"))
struct FTransform_NetQuantize : public FTransform
{
	GENERATED_USTRUCT_BODY()

		FORCEINLINE FTransform_NetQuantize() : FTransform()
	{}

	FORCEINLINE explicit FTransform_NetQuantize(ENoInit Init) : FTransform(Init)
	{}

	FORCEINLINE explicit FTransform_NetQuantize(const FVector& InTranslation) : FTransform(InTranslation)
	{}

	FORCEINLINE explicit FTransform_NetQuantize(const FQuat& InRotation) : FTransform(InRotation)
	{}

	FORCEINLINE explicit FTransform_NetQuantize(const FRotator& InRotation) : FTransform(InRotation)
	{}

	FORCEINLINE FTransform_NetQuantize(const FQuat& InRotation, const FVector& InTranslation, const FVector& InScale3D = FVector::OneVector)
		: FTransform(InRotation, InTranslation, InScale3D)
	{}

	FORCEINLINE FTransform_NetQuantize(const FRotator& InRotation, const FVector& InTranslation, const FVector& InScale3D = FVector::OneVector)
		: FTransform(InRotation, InTranslation, InScale3D)
	{}

	FORCEINLINE FTransform_NetQuantize(const FTransform& InTransform) : FTransform(InTransform)
	{}

	FORCEINLINE explicit FTransform_NetQuantize(const FMatrix& InMatrix) : FTransform(InMatrix)
	{}

	FORCEINLINE FTransform_NetQuantize(const FVector& InX, const FVector& InY, const FVector& InZ, const FVector& InTranslation)
		: FTransform(InX, InY, InZ, InTranslation)
	{}
public:

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

	template <uint32 bits>
	static bool SerializeQuat_SmallestThree(FArchive& Ar, FQuat &InQuat)
	{
		check(bits > 1 && bits <= 32);

		uint32 IntegerA = 0, IntegerB = 0, IntegerC = 0, LargestIndex = 0;

		const float scale = float((1 << bits) - 1);

		if (Ar.IsSaving())
		{
			InQuat.Normalize();
			const float abs_x = FMath::Abs(InQuat.X);
			const float abs_y = FMath::Abs(InQuat.Y);
			const float abs_z = FMath::Abs(InQuat.Z);
			const float abs_w = FMath::Abs(InQuat.W);

			LargestIndex = 0;
			float largest_value = abs_x;

			if (abs_y > largest_value)
			{
				LargestIndex = 1;
				largest_value = abs_y;
			}

			if (abs_z > largest_value)
			{
				LargestIndex = 2;
				largest_value = abs_z;
			}

			if (abs_w > largest_value)
			{
				LargestIndex = 3;
				largest_value = abs_w;
			}

			float a = 0.f;
			float b = 0.f;
			float c = 0.f;

			switch (LargestIndex)
			{
			case 0:
				if (InQuat.X >= 0)
				{
					a = InQuat.Y;
					b = InQuat.Z;
					c = InQuat.W;
				}
				else
				{
					a = -InQuat.Y;
					b = -InQuat.Z;
					c = -InQuat.W;
				}
				break;

			case 1:
				if (InQuat.Y >= 0)
				{
					a = InQuat.X;
					b = InQuat.Z;
					c = InQuat.W;
				}
				else
				{
					a = -InQuat.X;
					b = -InQuat.Z;
					c = -InQuat.W;
				}
				break;

			case 2:
				if (InQuat.Z >= 0)
				{
					a = InQuat.X;
					b = InQuat.Y;
					c = InQuat.W;
				}
				else
				{
					a = -InQuat.X;
					b = -InQuat.Y;
					c = -InQuat.W;
				}
				break;

			case 3:
				if (InQuat.W >= 0)
				{
					a = InQuat.X;
					b = InQuat.Y;
					c = InQuat.Z;
				}
				else
				{
					a = -InQuat.X;
					b = -InQuat.Y;
					c = -InQuat.Z;
				}
				break;

			default:break;
			}

			const float normal_a = (a - TransNetQuant::MinimumQ) / (TransNetQuant::MinMaxQDiff);
			const float normal_b = (b - TransNetQuant::MinimumQ) / (TransNetQuant::MinMaxQDiff);
			const float normal_c = (c - TransNetQuant::MinimumQ) / (TransNetQuant::MinMaxQDiff);

			IntegerA = FMath::FloorToInt(normal_a * scale + 0.5f);
			IntegerB = FMath::FloorToInt(normal_b * scale + 0.5f);
			IntegerC = FMath::FloorToInt(normal_c * scale + 0.5f);
		}

		Ar.SerializeBits(&LargestIndex, 2);
		Ar.SerializeBits(&IntegerA, bits);
		Ar.SerializeBits(&IntegerB, bits);
		Ar.SerializeBits(&IntegerC, bits);

		if (Ar.IsLoading())
		{
			const float inverse_scale = 1.0f / scale;

			const float a = IntegerA * inverse_scale * (TransNetQuant::MinMaxQDiff) + TransNetQuant::MinimumQ;
			const float b = IntegerB * inverse_scale * (TransNetQuant::MinMaxQDiff) + TransNetQuant::MinimumQ;
			const float c = IntegerC * inverse_scale * (TransNetQuant::MinMaxQDiff) + TransNetQuant::MinimumQ;

			switch (LargestIndex)
			{
			case 0:
			{
				InQuat.X = FMath::Sqrt(1.f - a * a - b * b - c * c);
				InQuat.Y = a;
				InQuat.Z = b;
				InQuat.W = c;
			}
			break;

			case 1:
			{
				InQuat.X = a;
				InQuat.Y = FMath::Sqrt(1.f - a * a - b * b - c * c);
				InQuat.Z = b;
				InQuat.W = c;
			}
			break;

			case 2:
			{
				InQuat.X = a;
				InQuat.Y = b;
				InQuat.Z = FMath::Sqrt(1.f - a * a - b * b - c * c);
				InQuat.W = c;
			}
			break;

			case 3:
			{
				InQuat.X = a;
				InQuat.Y = b;
				InQuat.Z = c;
				InQuat.W = FMath::Sqrt(1.f - a * a - b * b - c * c);
			}
			break;

			default:
			{
				InQuat.X = 0.f;
				InQuat.Y = 0.f;
				InQuat.Z = 0.f;
				InQuat.W = 1.f;
			}
			}

			InQuat.Normalize();
		}

		return true;
	}
};

template<>
struct TStructOpsTypeTraits< FTransform_NetQuantize > : public TStructOpsTypeTraitsBase2<FTransform_NetQuantize>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

UENUM()
enum class EVRVectorQuantization : uint8
{

	RoundOneDecimal = 0,

	RoundTwoDecimals = 1
};

UENUM()
enum class EVRRotationQuantization : uint8
{

	RoundTo10Bits = 0,

	RoundToShort = 1
};

USTRUCT()
struct VREXPANSIONPLUGIN_API FBPVRComponentPosRep
{
	GENERATED_USTRUCT_BODY()
public:

	UPROPERTY(Transient)
		FVector Position;
	UPROPERTY(Transient)
		FRotator Rotation;

	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
		EVRVectorQuantization QuantizationLevel;

	UPROPERTY(EditDefaultsOnly, Category = Replication, AdvancedDisplay)
		EVRRotationQuantization RotationQuantizationLevel;

	FORCEINLINE static uint16 CompressAxisTo10BitShort(float Angle)
	{

		return FMath::RoundToInt(Angle * 1024.f / 360.f) & 0xFFFF;
	}

	FORCEINLINE static float DecompressAxisFrom10BitShort(uint16 Angle)
	{

		return (Angle * 360.f / 1024.f);
	}

	FBPVRComponentPosRep():
		QuantizationLevel(EVRVectorQuantization::RoundTwoDecimals),
		RotationQuantizationLevel(EVRRotationQuantization::RoundToShort)
	{

		Position = FVector::ZeroVector;
		Rotation = FRotator::ZeroRotator;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		Ar.SerializeBits(&QuantizationLevel, 1); 
		Ar.SerializeBits(&RotationQuantizationLevel, 1); 

		uint16 ShortPitch = 0;
		uint16 ShortYaw = 0;
		uint16 ShortRoll = 0;

		if (Ar.IsSaving())
		{		
			switch (QuantizationLevel)
			{
			case EVRVectorQuantization::RoundTwoDecimals: bOutSuccess &= SerializePackedVector<100, 22>(Position, Ar); break;
			case EVRVectorQuantization::RoundOneDecimal: bOutSuccess &= SerializePackedVector<10, 18>(Position, Ar); break;
			}

			switch (RotationQuantizationLevel)
			{
			case EVRRotationQuantization::RoundTo10Bits:
			{
				ShortPitch = CompressAxisTo10BitShort(Rotation.Pitch);
				ShortYaw = CompressAxisTo10BitShort(Rotation.Yaw);
				ShortRoll = CompressAxisTo10BitShort(Rotation.Roll);

				Ar.SerializeBits(&ShortPitch, 10);
				Ar.SerializeBits(&ShortYaw, 10);
				Ar.SerializeBits(&ShortRoll, 10);
			}break;

			case EVRRotationQuantization::RoundToShort:
			{
				ShortPitch = FRotator::CompressAxisToShort(Rotation.Pitch);
				ShortYaw = FRotator::CompressAxisToShort(Rotation.Yaw);
				ShortRoll = FRotator::CompressAxisToShort(Rotation.Roll);

				Ar << ShortPitch;
				Ar << ShortYaw;
				Ar << ShortRoll;
			}break;
			}
		}
		else 
		{

			switch (QuantizationLevel)
			{
			case EVRVectorQuantization::RoundTwoDecimals: bOutSuccess &= SerializePackedVector<100, 22>(Position, Ar); break;
			case EVRVectorQuantization::RoundOneDecimal: bOutSuccess &= SerializePackedVector<10, 18>(Position, Ar); break;
			}

			switch (RotationQuantizationLevel)
			{
			case EVRRotationQuantization::RoundTo10Bits:
			{
				Ar.SerializeBits(&ShortPitch, 10);
				Ar.SerializeBits(&ShortYaw, 10);
				Ar.SerializeBits(&ShortRoll, 10);

				Rotation.Pitch = DecompressAxisFrom10BitShort(ShortPitch);
				Rotation.Yaw = DecompressAxisFrom10BitShort(ShortYaw);
				Rotation.Roll = DecompressAxisFrom10BitShort(ShortRoll);
			}break;

			case EVRRotationQuantization::RoundToShort:
			{
				Ar << ShortPitch;
				Ar << ShortYaw;
				Ar << ShortRoll;

				Rotation.Pitch = FRotator::DecompressAxisFromShort(ShortPitch);
				Rotation.Yaw = FRotator::DecompressAxisFromShort(ShortYaw);
				Rotation.Roll = FRotator::DecompressAxisFromShort(ShortRoll);
			}break;
			}
		}

		return bOutSuccess;
	}

};

template<>
struct TStructOpsTypeTraits< FBPVRComponentPosRep > : public TStructOpsTypeTraitsBase2<FBPVRComponentPosRep>
{
	enum
	{
		WithNetSerializer = true,
		WithNetSharedSerialization = true,
	};
};

UENUM(Blueprintable)
enum class EGripCollisionType : uint8
{

	InteractiveCollisionWithPhysics,

	InteractiveCollisionWithSweep,

	InteractiveHybridCollisionWithPhysics,

	InteractiveHybridCollisionWithSweep,

	SweepWithPhysics,

	PhysicsOnly,

	ManipulationGrip,

	ManipulationGripWithWristTwist,

	AttachmentGrip,

	CustomGrip,

	EventsOnly,

	LockedConstraint

};

UENUM(Blueprintable)
enum class EBPHMDDeviceType : uint8
{
	DT_OculusHMD,
	DT_PSVR,

	DT_ES2GenericStereoMesh,
	DT_SteamVR,
	DT_GearVR,
	DT_GoogleVR,
	DT_AppleARKit,
	DT_GoogleARCore,
	DT_Unknown
};

UENUM(Blueprintable)
enum class EGripLerpState : uint8
{
	StartLerp,
	EndLerp,

	NotLerping
};

UENUM(Blueprintable)
enum class ESecondaryGripType : uint8
{

	SG_None, 

	SG_Free, 

	SG_SlotOnly, 

	SG_Free_Retain, 

	SG_SlotOnly_Retain, 

	SG_FreeWithScaling_Retain, 

	SG_SlotOnlyWithScaling_Retain,

	SG_Custom, 

	SG_ScalingOnly, 
};

UENUM(Blueprintable)
enum class EGripLateUpdateSettings : uint8
{
	LateUpdatesAlwaysOn,
	LateUpdatesAlwaysOff,
	NotWhenColliding,
	NotWhenDoubleGripping,
	NotWhenCollidingOrDoubleGripping
};

UENUM(Blueprintable)
enum class EGripMovementReplicationSettings : uint8
{
	KeepOriginalMovement,
	ForceServerSideMovement,
	ForceClientSideMovement,
	ClientSide_Authoritive,
	ClientSide_Authoritive_NoRep
};

UENUM(Blueprintable)
enum class EGripTargetType : uint8
{
	ActorGrip,
	ComponentGrip

};

UENUM(Blueprintable)
enum class EGripInterfaceTeleportBehavior : uint8
{

	TeleportAllComponents,

	DeltaTeleportation,

	OnlyTeleportRootComponent,

	DropOnTeleport,

	DontTeleport
};

UENUM(Blueprintable)
enum class EPhysicsGripConstraintType : uint8
{
	AccelerationConstraint = 0,
	ForceConstraint = 1
};

UENUM(Blueprintable)
enum class EPhysicsGripCOMType : uint8
{

	COM_Default = 0,

	COM_AtPivot = 1,

	COM_SetAndGripAt = 2,

	COM_GripAt = 3,

	COM_GripAtControllerLoc = 4
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPAdvGripPhysicsSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings")
		bool bUsePhysicsSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"))
		EPhysicsGripConstraintType PhysicsConstraintType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"))
		EPhysicsGripCOMType PhysicsGripLocationSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"))
		bool bTurnOffGravityDuringGrip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"))
		bool bSkipSettingSimulating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"), meta = (ClampMin = "0.00", UIMin = "0.00", ClampMax = "512.00", UIMax = "512.00"))
		float LinearMaxForceCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"), meta = (ClampMin = "0.00", UIMin = "0.00", ClampMax = "512.00", UIMax = "512.00"))
		float AngularMaxForceCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUsePhysicsSettings"))
		bool bUseCustomAngularValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUseCustomAngularValues", ClampMin = "0.000", UIMin = "0.000"))
		float AngularStiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (editcondition = "bUseCustomAngularValues", ClampMin = "0.000", UIMin = "0.000"))
		float AngularDamping;

	FBPAdvGripPhysicsSettings():
		bUsePhysicsSettings(false),
		PhysicsConstraintType(EPhysicsGripConstraintType::AccelerationConstraint),
		PhysicsGripLocationSettings(EPhysicsGripCOMType::COM_Default),
		bTurnOffGravityDuringGrip(false),
		bSkipSettingSimulating(false),
		LinearMaxForceCoefficient(0.f),
		AngularMaxForceCoefficient(0.f),
		bUseCustomAngularValues(false),
		AngularStiffness(0.0f),
		AngularDamping(0.0f)

	{}

	FORCEINLINE bool operator==(const FBPAdvGripPhysicsSettings &Other) const
	{
		return (bUsePhysicsSettings == Other.bUsePhysicsSettings &&
			PhysicsGripLocationSettings == Other.PhysicsGripLocationSettings &&
			bTurnOffGravityDuringGrip == Other.bTurnOffGravityDuringGrip &&
			bSkipSettingSimulating == Other.bSkipSettingSimulating &&
			bUseCustomAngularValues == Other.bUseCustomAngularValues &&
			PhysicsConstraintType == Other.PhysicsConstraintType &&
			FMath::IsNearlyEqual(LinearMaxForceCoefficient, Other.LinearMaxForceCoefficient) &&
			FMath::IsNearlyEqual(AngularMaxForceCoefficient, Other.AngularMaxForceCoefficient) &&
			FMath::IsNearlyEqual(AngularStiffness, Other.AngularStiffness) &&
			FMath::IsNearlyEqual(AngularDamping, Other.AngularDamping) 

			);
	}

	FORCEINLINE bool operator!=(const FBPAdvGripPhysicsSettings &Other) const
	{
		return (bUsePhysicsSettings != Other.bUsePhysicsSettings ||
			PhysicsGripLocationSettings != Other.PhysicsGripLocationSettings ||
			bTurnOffGravityDuringGrip != Other.bTurnOffGravityDuringGrip ||
			bSkipSettingSimulating != Other.bSkipSettingSimulating ||
			bUseCustomAngularValues != Other.bUseCustomAngularValues ||
			PhysicsConstraintType != Other.PhysicsConstraintType ||
			!FMath::IsNearlyEqual(LinearMaxForceCoefficient, Other.LinearMaxForceCoefficient) ||
			!FMath::IsNearlyEqual(AngularMaxForceCoefficient, Other.AngularMaxForceCoefficient) ||
			!FMath::IsNearlyEqual(AngularStiffness, Other.AngularStiffness) ||
			!FMath::IsNearlyEqual(AngularDamping, Other.AngularDamping) 

			);
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{

		Ar.SerializeBits(&bUsePhysicsSettings, 1);

		if (bUsePhysicsSettings)
		{

			Ar.SerializeBits(&PhysicsGripLocationSettings, 3); 

			Ar.SerializeBits(&PhysicsConstraintType, 1); 

			Ar.SerializeBits(&bTurnOffGravityDuringGrip, 1);
			Ar.SerializeBits(&bSkipSettingSimulating, 1);

			if (Ar.IsSaving())
			{
				bOutSuccess &= WriteFixedCompressedFloat<512, 17>(LinearMaxForceCoefficient, Ar);
				bOutSuccess &= WriteFixedCompressedFloat<512, 17>(AngularMaxForceCoefficient, Ar);
			}
			else
			{
				bOutSuccess &= ReadFixedCompressedFloat<512, 17>(LinearMaxForceCoefficient, Ar);
				bOutSuccess &= ReadFixedCompressedFloat<512, 17>(AngularMaxForceCoefficient, Ar);
			}

			Ar.SerializeBits(&bUseCustomAngularValues, 1);

			if (bUseCustomAngularValues)
			{
				Ar << AngularStiffness;
				Ar << AngularDamping;
			}

		}

		bOutSuccess = true;
		return bOutSuccess;
	}
};

template<>
struct TStructOpsTypeTraits< FBPAdvGripPhysicsSettings > : public TStructOpsTypeTraitsBase2<FBPAdvGripPhysicsSettings>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPAdvGripSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings")
		uint8 GripPriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings")
		bool bSetOwnerOnGrip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings")
		bool bDisallowLerping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings")
		bool bDisallowSettingPositionOnClientAuthDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AdvancedGripSettings")
		FBPAdvGripPhysicsSettings PhysicsSettings;

	FBPAdvGripSettings() :
		GripPriority(1),
		bSetOwnerOnGrip(1),
		bDisallowLerping(0),
		bDisallowSettingPositionOnClientAuthDrop(0)
	{}

	FBPAdvGripSettings(int GripPrio) :
		GripPriority(GripPrio),
		bSetOwnerOnGrip(1),
		bDisallowLerping(0),
		bDisallowSettingPositionOnClientAuthDrop(0)
	{}
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPSecondaryGripInfo
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category = "SecondaryGripInfo")
		bool bHasSecondaryAttachment;

	UPROPERTY(BlueprintReadOnly, Category = "SecondaryGripInfo")
		TObjectPtr<USceneComponent> SecondaryAttachment;

	UPROPERTY(BlueprintReadWrite, Category = "SecondaryGripInfo")
		FTransform_NetQuantize SecondaryRelativeTransform;

	UPROPERTY(BlueprintReadWrite, Category = "SecondaryGripInfo")
		bool bIsSlotGrip;

	UPROPERTY(BlueprintReadWrite, Category = "SecondaryGripInfo")
		FName SecondarySlotName;

	UPROPERTY()
		float LerpToRate;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "SecondaryGripInfo")
		float SecondaryGripDistance;

	EGripLerpState GripLerpState;
	float curLerp;

	FVector LastRelativeLocation;

	void ClearNonReppingItems()
	{
		SecondaryGripDistance = 0.0f;
		GripLerpState = EGripLerpState::NotLerping;
		curLerp = 0.0f;
	}

	FBPSecondaryGripInfo():
		bHasSecondaryAttachment(false),
		SecondaryAttachment(nullptr),
		SecondaryRelativeTransform(FTransform::Identity),
		bIsSlotGrip(false),
		SecondarySlotName(NAME_None),
		LerpToRate(0.0f),
		SecondaryGripDistance(0.0f),
		GripLerpState(EGripLerpState::NotLerping),
		curLerp(0.0f),
		LastRelativeLocation(FVector::ZeroVector)
	{}

	FORCEINLINE FBPSecondaryGripInfo& RepCopy(const FBPSecondaryGripInfo& Other)
	{
		this->bHasSecondaryAttachment = Other.bHasSecondaryAttachment;
		this->SecondaryAttachment = Other.SecondaryAttachment;

		if (bHasSecondaryAttachment)
		{
			this->SecondaryRelativeTransform = Other.SecondaryRelativeTransform;
			this->bIsSlotGrip = Other.bIsSlotGrip;
			this->SecondarySlotName = Other.SecondarySlotName;
		}

		this->LerpToRate = Other.LerpToRate;
		return *this;
	}

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		bOutSuccess = true;

		Ar.SerializeBits(&bHasSecondaryAttachment, 1);

		if (bHasSecondaryAttachment)
		{
			Ar << SecondaryAttachment;

			SecondaryRelativeTransform.NetSerialize(Ar, Map, bOutSuccess);

			Ar.SerializeBits(&bIsSlotGrip, 1);

			Ar << SecondarySlotName;
		}

		if (Ar.IsSaving())
			bOutSuccess &= WriteFixedCompressedFloat<16, 12>(LerpToRate, Ar);
		else
			bOutSuccess &= ReadFixedCompressedFloat<16, 12>(LerpToRate, Ar);

		return true;
	}
};

template<>
struct TStructOpsTypeTraits< FBPSecondaryGripInfo > : public TStructOpsTypeTraitsBase2<FBPSecondaryGripInfo>
{
	enum
	{
		WithNetSerializer = true
	};
};

#define INVALID_VRGRIP_ID 0

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPActorGripInformation
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		uint8 GripID;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		EGripTargetType GripTargetType;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		TObjectPtr<UObject> GrippedObject;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		EGripCollisionType GripCollisionType;
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
		EGripLateUpdateSettings GripLateUpdateSetting;
	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "Settings")
		bool bColliding;
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
		FTransform_NetQuantize RelativeTransform;
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
		bool bIsSlotGrip;
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
		FName GrippedBoneName;
	UPROPERTY(BlueprintReadWrite, Category = "Settings")
		FName SlotName;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		EGripMovementReplicationSettings GripMovementReplicationSetting;

	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "Settings")
		bool bIsPaused;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "Settings")
		bool bIsPendingKill;

	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "Settings")
		bool bLockHybridGrip;

	UPROPERTY()
		bool bOriginalReplicatesMovement;
	UPROPERTY()
		bool bOriginalGravity;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		float Damping;
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		float Stiffness;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		FBPAdvGripSettings AdvancedGripSettings;

	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		FBPSecondaryGripInfo SecondaryGripInfo;

	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "Settings")
	FTransform AdditionTransform;

	UPROPERTY(BlueprintReadOnly, NotReplicated, Category = "Settings")
		float GripDistance;

	bool bIsLocked;
	FQuat LastLockedRotation;

	FTransform LastWorldTransform;
	bool bSetLastWorldTransform;

	bool bSkipNextTeleportCheck;

	bool bSkipNextConstraintLengthCheck;

	float CurrentLerpTime;
	float LerpSpeed;
	FTransform OnGripTransform;

	FVector LinVel = FVector::ZeroVector;
	FVector RotVel = FVector::ZeroVector;
	FTransform LastVelWorldTrans; 

	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "Settings")
	bool bIsLerping;

	bool IsLocalAuthGrip()
	{
		return GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive || GripMovementReplicationSetting == EGripMovementReplicationSettings::ClientSide_Authoritive_NoRep;
	}

	bool IsValid() const
	{
		return (!bIsPendingKill && GripID != INVALID_VRGRIP_ID && GrippedObject && IsValidChecked(GrippedObject));
	}

	bool IsActive() const
	{
		return (!bIsPendingKill && GripID != INVALID_VRGRIP_ID && GrippedObject && IsValidChecked(GrippedObject) && !bIsPaused);
	}

	struct FGripValueCache
	{
		bool bWasInitiallyRepped;
		uint8 CachedGripID;

		FGripValueCache() :
			bWasInitiallyRepped(false),
			CachedGripID(INVALID_VRGRIP_ID)
		{}

	}ValueCache;

	void ClearNonReppingItems()
	{
		ValueCache = FGripValueCache();
		bColliding = false;
		bIsLocked = false;
		LastLockedRotation = FQuat::Identity;
		LastWorldTransform.SetIdentity();
		bSetLastWorldTransform = false;
		bSkipNextTeleportCheck = false;
		bSkipNextConstraintLengthCheck = false;
		bIsPaused = false;
		bIsPendingKill = false;
		bLockHybridGrip = false;
		AdditionTransform = FTransform::Identity;
		GripDistance = 0.0f;
		CurrentLerpTime = 0.f;
		LerpSpeed = 0.f;
		OnGripTransform = FTransform::Identity;
		bIsLerping = false;

		SecondaryGripInfo.ClearNonReppingItems();
	}

	FORCEINLINE FBPActorGripInformation& RepCopy(const FBPActorGripInformation& Other)
	{
		this->GripID = Other.GripID;
		this->GripTargetType = Other.GripTargetType;
		this->GrippedObject = Other.GrippedObject;
		this->GripCollisionType = Other.GripCollisionType;
		this->GripLateUpdateSetting = Other.GripLateUpdateSetting;
		this->RelativeTransform = Other.RelativeTransform;
		this->bIsSlotGrip = Other.bIsSlotGrip;
		this->GrippedBoneName = Other.GrippedBoneName;
		this->SlotName = Other.SlotName;
		this->GripMovementReplicationSetting = Other.GripMovementReplicationSetting;

		this->Damping = Other.Damping;
		this->Stiffness = Other.Stiffness;
		this->AdvancedGripSettings = Other.AdvancedGripSettings;		
		this->SecondaryGripInfo.RepCopy(Other.SecondaryGripInfo); 

		return *this;
	}

	AActor* GetGrippedActor() const;

	UPrimitiveComponent* GetGrippedComponent() const;

	UPrimitiveComponent* GetGripPrimitiveComponent() const;

	FORCEINLINE bool operator==(const FBPActorGripInformation &Other) const
	{
		if ((GripID != INVALID_VRGRIP_ID) && (GripID == Other.GripID) )
			return true;

		return false;
	}

	FORCEINLINE bool operator==(const AActor * Other) const
	{
		if (Other && GrippedObject && GrippedObject == (const UObject*)Other)
			return true;

		return false;
	}

	bool operator==(const UPrimitiveComponent* Other) const;

	FORCEINLINE bool operator==(const UObject * Other) const
	{
		if (Other && GrippedObject == Other)
			return true;

		return false;
	}

	FORCEINLINE bool operator==(const uint8& Other) const
	{
		if ((GripID != INVALID_VRGRIP_ID) && (GripID == Other))
			return true;

		return false;
	}

	FBPActorGripInformation() :
		GripID(INVALID_VRGRIP_ID),
		GripTargetType(EGripTargetType::ActorGrip),
		GrippedObject(nullptr),
		GripCollisionType(EGripCollisionType::InteractiveCollisionWithPhysics),
		GripLateUpdateSetting(EGripLateUpdateSettings::NotWhenCollidingOrDoubleGripping),
		bColliding(false),
		RelativeTransform(FTransform::Identity),
		bIsSlotGrip(false),
		GrippedBoneName(NAME_None),
		SlotName(NAME_None),
		GripMovementReplicationSetting(EGripMovementReplicationSettings::ForceClientSideMovement),
		bIsPaused(false),
		bIsPendingKill(false),
		bLockHybridGrip(false),
		bOriginalReplicatesMovement(false),
		bOriginalGravity(false),
		Damping(200.0f),
		Stiffness(1500.0f),
		AdditionTransform(FTransform::Identity),
		GripDistance(0.0f),
		bIsLocked(false),
		LastLockedRotation(FRotator::ZeroRotator),
		LastWorldTransform(FTransform::Identity),
		bSetLastWorldTransform(false),
		bSkipNextTeleportCheck(false),
		bSkipNextConstraintLengthCheck(false),
		CurrentLerpTime(0.f),
		LerpSpeed(0.f),
		OnGripTransform(FTransform::Identity),
		bIsLerping(false)
	{
	}	

};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPGripPair
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripPair")
		TObjectPtr<UGripMotionControllerComponent> HoldingController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GripPair")
	uint8 GripID;

	FBPGripPair() :
		HoldingController(nullptr),
		GripID(INVALID_VRGRIP_ID)
	{}

	FBPGripPair(UGripMotionControllerComponent * Controller, uint8 ID) :
		HoldingController(Controller),
		GripID(ID)
	{}

	void Clear()
	{
		HoldingController = nullptr;
		GripID = INVALID_VRGRIP_ID;
	}

	bool IsValid()
	{
		return HoldingController != nullptr && GripID != INVALID_VRGRIP_ID;
	}

	FORCEINLINE bool operator==(const FBPGripPair & Other) const
	{
		return (Other.HoldingController == HoldingController && ((GripID != INVALID_VRGRIP_ID) && (GripID == Other.GripID)));
	}

	FORCEINLINE bool operator==(const UGripMotionControllerComponent * Other) const
	{
		return (Other == HoldingController);
	}

	FORCEINLINE bool operator==(const uint8 & Other) const
	{
		return GripID == Other;
	}

};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPInterfaceProperties
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		bool bDenyGripping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		bool bAllowMultipleGrips;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripInterfaceTeleportBehavior OnTeleportBehavior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		bool bSimulateOnDrop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripCollisionType SlotDefaultGripType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripCollisionType FreeDefaultGripType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		ESecondaryGripType SecondaryGripType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripMovementReplicationSettings MovementReplicationType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		EGripLateUpdateSettings LateUpdateSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float ConstraintStiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float ConstraintDamping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float ConstraintBreakDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float SecondarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface")
		float PrimarySlotRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VRGripInterface|AdvancedGripSettings")
		FBPAdvGripSettings AdvancedGripSettings;

	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "VRGripInterface")
		bool bIsHeld; 

	bool bWasHeld;;

	UPROPERTY(BlueprintReadWrite, NotReplicated, Category = "VRGripInterface")
		TArray<FBPGripPair> HoldingControllers; 

	FBPInterfaceProperties():
		bDenyGripping(false),
		bAllowMultipleGrips(false),
		OnTeleportBehavior(EGripInterfaceTeleportBehavior::DropOnTeleport),
		bSimulateOnDrop(true),
		SlotDefaultGripType(EGripCollisionType::ManipulationGrip),
		FreeDefaultGripType(EGripCollisionType::ManipulationGrip),
		SecondaryGripType(ESecondaryGripType::SG_None),
		MovementReplicationType(EGripMovementReplicationSettings::ForceClientSideMovement),
		LateUpdateSetting(EGripLateUpdateSettings::LateUpdatesAlwaysOff),
		ConstraintStiffness(1500.0f),
		ConstraintDamping(200.0f),
		ConstraintBreakDistance(0.0f),
		SecondarySlotRange(20.0f),
		PrimarySlotRange(20.0f),
		bIsHeld(false),
		bWasHeld(false)
	{
	}
};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPActorPhysicsHandleInformation
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "Settings")
		TObjectPtr<UObject> HandledObject;
	uint8 GripID;
	bool bIsPaused;

	FPhysicsActorHandle KinActorData2;
	FPhysicsConstraintHandle HandleData2;
	FLinearDriveConstraint LinConstraint;
	FAngularDriveConstraint AngConstraint;

	FTransform LastPhysicsTransform;
	FTransform COMPosition;
	FTransform RootBoneRotation;

	bool bSetCOM;
	bool bSkipResettingCom;
	bool bSkipDeletingKinematicActor;
	bool bInitiallySetup;

	FBPActorPhysicsHandleInformation()
	{	
		HandledObject = nullptr;
		LastPhysicsTransform = FTransform::Identity;
		COMPosition = FTransform::Identity;
		GripID = INVALID_VRGRIP_ID;
		bIsPaused = false;
		RootBoneRotation = FTransform::Identity;
		bSetCOM = false;
		bSkipResettingCom = false;
		bSkipDeletingKinematicActor = false;
		bInitiallySetup = false;
		KinActorData2 = nullptr;
	}

	FORCEINLINE bool operator==(const FBPActorGripInformation & Other) const
	{
		return ((GripID != INVALID_VRGRIP_ID) && (GripID == Other.GripID));
	}

	FORCEINLINE bool operator==(const uint8 & Other) const
	{
		return ((GripID != INVALID_VRGRIP_ID) && (GripID == Other));
	}

};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPAdvancedPhysicsHandleAxisSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Constraint, meta = (ClampMin = "0.0"))
		float Stiffness;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Constraint, meta = (ClampMin = "0.0"))
		float Damping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PhysicsSettings", meta = (ClampMin = "0.00", UIMin = "0.00", ClampMax = "256.00", UIMax = "256.00"))
		float MaxForceCoefficient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Constraint)
		bool bEnablePositionDrive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Constraint)
		bool bEnableVelocityDrive;

	FBPAdvancedPhysicsHandleAxisSettings()
	{
		Stiffness = 0.f;
		Damping = 0.f;
		MaxForceCoefficient = 0.f;
		bEnablePositionDrive = false;
		bEnableVelocityDrive = false;
	}

	void FillFrom(FConstraintDrive& ConstraintDrive)
	{
		Damping = ConstraintDrive.Damping;
		Stiffness = ConstraintDrive.Stiffness;
		MaxForceCoefficient = ConstraintDrive.MaxForce / Stiffness;
		bEnablePositionDrive = ConstraintDrive.bEnablePositionDrive;
		bEnableVelocityDrive = ConstraintDrive.bEnableVelocityDrive;
	}

	void FillTo(FConstraintDrive& ConstraintDrive, float DampingScaler = 1.0f, float StiffnessScaler = 1.0f) const
	{
		ConstraintDrive.Damping = Damping * DampingScaler;
		ConstraintDrive.Stiffness = Stiffness * StiffnessScaler;
		ConstraintDrive.MaxForce = (float)FMath::Clamp<double>((double)ConstraintDrive.Stiffness * (double)MaxForceCoefficient, 0, (double)MAX_FLT);
		ConstraintDrive.bEnablePositionDrive = bEnablePositionDrive;
		ConstraintDrive.bEnableVelocityDrive = bEnableVelocityDrive;
	}

};

USTRUCT(BlueprintType, Category = "VRExpansionLibrary")
struct VREXPANSIONPLUGIN_API FBPAdvancedPhysicsHandleSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linear Constraint Settings")
		FBPAdvancedPhysicsHandleAxisSettings XAxisSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linear Constraint Settings")
		FBPAdvancedPhysicsHandleAxisSettings YAxisSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Linear Constraint Settings")
		FBPAdvancedPhysicsHandleAxisSettings ZAxisSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Angular Constraint Settings")
		FBPAdvancedPhysicsHandleAxisSettings SlerpSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Angular Constraint Settings")
		FBPAdvancedPhysicsHandleAxisSettings TwistSettings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Angular Constraint Settings")
		FBPAdvancedPhysicsHandleAxisSettings SwingSettings;

	bool FillFrom(FBPActorPhysicsHandleInformation* HandleInfo)
	{
		if (!HandleInfo)
			return false;

		XAxisSettings.FillFrom(HandleInfo->LinConstraint.XDrive);
		YAxisSettings.FillFrom(HandleInfo->LinConstraint.YDrive);
		ZAxisSettings.FillFrom(HandleInfo->LinConstraint.ZDrive);

		SlerpSettings.FillFrom(HandleInfo->AngConstraint.SlerpDrive);
		TwistSettings.FillFrom(HandleInfo->AngConstraint.TwistDrive);
		SwingSettings.FillFrom(HandleInfo->AngConstraint.SwingDrive);

		return true;
	}

	bool FillTo(FBPActorPhysicsHandleInformation* HandleInfo, bool bModifyWithScalers = true) const;
};