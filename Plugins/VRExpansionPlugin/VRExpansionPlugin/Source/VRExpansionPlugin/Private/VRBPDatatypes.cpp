

#include "VRBPDatatypes.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRBPDatatypes)

#include "CoreMinimal.h"
#include "VRGlobalSettings.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/IConsoleManager.h"
#include "Chaos/ChaosEngineInterface.h"

namespace VRDataTypeCVARs
{

	static int32 RepHighPrecisionTransforms = 0;
	FAutoConsoleVariableRef CVarRepHighPrecisionTransforms(
		TEXT("vrexp.RepHighPrecisionTransforms"),
		RepHighPrecisionTransforms,
		TEXT("When on, will rep Quantized transforms at full precision, WARNING use at own risk, if this isn't the same setting client & server then it will crash.\n")
		TEXT("0: Disable, 1: Enable"),
		ECVF_Default);
}

bool FTransform_NetQuantize::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;

	FVector rTranslation;
	FVector rScale3D;

	FRotator rRotation;

	uint16 ShortPitch = 0;
	uint16 ShortYaw = 0;
	uint16 ShortRoll = 0;

	bool bUseHighPrecision = VRDataTypeCVARs::RepHighPrecisionTransforms > 0;

	if (Ar.IsSaving())
	{

		rTranslation = this->GetTranslation();
		rScale3D = this->GetScale3D();
		rRotation = this->Rotator();

		if (bUseHighPrecision)
		{
			Ar << rTranslation;
			Ar << rScale3D;
			Ar << rRotation;
		}
		else
		{

			bOutSuccess &= SerializePackedVector<100, 30>(rTranslation, Ar);

			bOutSuccess &= SerializePackedVector<100, 30>(rScale3D, Ar);

			rRotation.SerializeCompressedShort(Ar);
		}

	}
	else 
	{

		if (bUseHighPrecision)
		{
			Ar << rTranslation;
			Ar << rScale3D;
			Ar << rRotation;
		}
		else
		{
			bOutSuccess &= SerializePackedVector<100, 30>(rTranslation, Ar);
			bOutSuccess &= SerializePackedVector<100, 30>(rScale3D, Ar);
			rRotation.SerializeCompressedShort(Ar);
		}

		this->SetComponents(rRotation.Quaternion(), rTranslation, rScale3D);
		this->NormalizeRotation();
	}

	return bOutSuccess;
}

void FBPEuroLowPassFilter::ResetSmoothingFilter()
{
	RawFilter.bFirstTime = true;
	DeltaFilter.bFirstTime = true;
}

FVector FBPEuroLowPassFilter::RunFilterSmoothing(const FVector& InRawValue, const float& InDeltaTime)
{
	if (InDeltaTime <= 0.0f)
	{

		return InRawValue;
	}

	const FVector Delta = RawFilter.bFirstTime == true ? FVector::ZeroVector : (InRawValue - RawFilter.PreviousRaw) * 1.0 / InDeltaTime;

	const FVector Estimated = DeltaFilter.Filter(Delta, FVector(DeltaFilter.CalculateAlphaTau(DeltaCutoff, InDeltaTime)));

	const FVector Cutoff = DeltaFilter.CalculateCutoff(Estimated, MinCutoff, CutoffSlope);

	return RawFilter.Filter(InRawValue, RawFilter.CalculateAlpha(Cutoff, InDeltaTime));
}

void FBPEuroLowPassFilterQuat::ResetSmoothingFilter()
{
	RawFilter.bFirstTime = true;
	DeltaFilter.bFirstTime = true;
}

FQuat FBPEuroLowPassFilterQuat::RunFilterSmoothing(const FQuat& InRawValue, const float& InDeltaTime)
{
	if (InDeltaTime <= 0.0f)
	{

		return InRawValue;
	}

	FQuat NewInVal = InRawValue;
	if (!RawFilter.bFirstTime)
	{

		FVector4 PrevQuatAsVector(RawFilter.Previous.X, RawFilter.Previous.Y, RawFilter.Previous.Z, RawFilter.Previous.W);
		FVector4 CurrQuatAsVector(InRawValue.X, InRawValue.Y, InRawValue.Z, InRawValue.W);
		if ((PrevQuatAsVector - CurrQuatAsVector).SizeSquared() > 2)
			NewInVal = FQuat(-InRawValue.X, -InRawValue.Y, -InRawValue.Z, -InRawValue.W);
	}

	FQuat Delta = FQuat::Identity;

	if (!RawFilter.bFirstTime)
	{
		Delta = (NewInVal - RawFilter.PreviousRaw) * (1.0 / InDeltaTime);
	}

	double AlphaTau = DeltaFilter.CalculateAlphaTau(DeltaCutoff, InDeltaTime);
	FQuat AlphaTauQ(AlphaTau, AlphaTau, AlphaTau, AlphaTau);
	const FQuat Estimated = DeltaFilter.Filter(Delta, AlphaTauQ);

	const FQuat Cutoff = DeltaFilter.CalculateCutoff(Estimated, MinCutoff, CutoffSlope);

	return RawFilter.Filter(NewInVal, RawFilter.CalculateAlpha(Cutoff, InDeltaTime)).GetNormalized();
}

void FBPEuroLowPassFilterTrans::ResetSmoothingFilter()
{
	RawFilter.bFirstTime = true;
	DeltaFilter.bFirstTime = true;
}

FTransform FBPEuroLowPassFilterTrans::RunFilterSmoothing(const FTransform& InRawValue, const float& InDeltaTime)
{
	if (InDeltaTime <= 0.0f)
	{

		return InRawValue;
	}

	FTransform NewInVal = InRawValue;
	if (!RawFilter.bFirstTime)
	{
		FQuat TransQ = NewInVal.GetRotation();
		FQuat PrevQ = RawFilter.Previous.GetRotation();

		FVector4 PrevQuatAsVector(PrevQ.X, PrevQ.Y, PrevQ.Z, PrevQ.W);
		FVector4 CurrQuatAsVector(TransQ.X, TransQ.Y, TransQ.Z, TransQ.W);
		if ((PrevQuatAsVector - CurrQuatAsVector).SizeSquared() > 2)
			NewInVal.SetRotation(FQuat(-TransQ.X, -TransQ.Y, -TransQ.Z, -TransQ.W));
	}

	FTransform Delta = FTransform::Identity;

	double Frequency = 1.0 / InDeltaTime;
	if (!RawFilter.bFirstTime)
	{
		Delta.SetLocation((NewInVal.GetLocation() - RawFilter.PreviousRaw.GetLocation()) * Frequency);
		Delta.SetRotation((NewInVal.GetRotation() - RawFilter.PreviousRaw.GetRotation()) * Frequency);
		Delta.SetScale3D((NewInVal.GetScale3D() - RawFilter.PreviousRaw.GetScale3D()) * Frequency);
	}

	double AlphaTau = DeltaFilter.CalculateAlphaTau(DeltaCutoff, InDeltaTime);
	FTransform AlphaTauQ(FQuat(AlphaTau, AlphaTau, AlphaTau, AlphaTau), FVector(AlphaTau), FVector(AlphaTau));
	const FTransform Estimated = DeltaFilter.Filter(Delta, AlphaTauQ);

	const FTransform Cutoff = DeltaFilter.CalculateCutoff(Estimated, MinCutoff, CutoffSlope);

	FTransform NewTrans = RawFilter.Filter(NewInVal, RawFilter.CalculateAlpha(Cutoff, InDeltaTime));
	NewTrans.NormalizeRotation();

	return NewTrans;
}

bool FBPAdvancedPhysicsHandleSettings::FillTo(FBPActorPhysicsHandleInformation* HandleInfo, bool bModifyWithScalers) const
{
	if (!HandleInfo)
		return false;

	float DampingMod = 0.0f;
	float StiffnessMod = 0.0f;
	float ADampingMod = 0.0f;
	float AStiffnessMod = 0.0f;
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	if (VRSettings.bUseChaosTranslationScalers)
	{
		StiffnessMod = VRSettings.LinearDriveStiffnessScale;
		DampingMod = VRSettings.LinearDriveDampingScale;
		AStiffnessMod = VRSettings.AngularDriveStiffnessScale;
		ADampingMod = VRSettings.AngularDriveDampingScale;
	}
	else
	{
		auto CVarLinearDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinearDriveStiffnessScale"));
		auto CVarLinearDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinaearDriveDampingScale"));
		auto CVarAngularDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveStiffnessScale"));
		auto CVarAngularDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveDampingScale"));

		StiffnessMod = CVarLinearDriveStiffnessScale->GetFloat();
		DampingMod = CVarLinearDriveDampingScale->GetFloat();
		AStiffnessMod = CVarAngularDriveStiffnessScale->GetFloat();
		ADampingMod = CVarAngularDriveDampingScale->GetFloat();
	}

	XAxisSettings.FillTo(HandleInfo->LinConstraint.XDrive, DampingMod, StiffnessMod);
	YAxisSettings.FillTo(HandleInfo->LinConstraint.YDrive, DampingMod, StiffnessMod);
	ZAxisSettings.FillTo(HandleInfo->LinConstraint.ZDrive, DampingMod, StiffnessMod);

	if ((SlerpSettings.bEnablePositionDrive || SlerpSettings.bEnableVelocityDrive))
	{
		HandleInfo->AngConstraint.AngularDriveMode = EAngularDriveMode::SLERP;
		SlerpSettings.FillTo(HandleInfo->AngConstraint.SlerpDrive, ADampingMod, AStiffnessMod);
	}
	else
	{
		HandleInfo->AngConstraint.AngularDriveMode = EAngularDriveMode::TwistAndSwing;
		TwistSettings.FillTo(HandleInfo->AngConstraint.TwistDrive, ADampingMod, AStiffnessMod);
		SwingSettings.FillTo(HandleInfo->AngConstraint.SwingDrive, ADampingMod, AStiffnessMod);
	}

	return true;
}

AActor* FBPActorGripInformation::GetGrippedActor() const
{
	return Cast<AActor>(GrippedObject);
}

UPrimitiveComponent* FBPActorGripInformation::GetGrippedComponent() const
{
	return Cast<UPrimitiveComponent>(GrippedObject);
}

UPrimitiveComponent* FBPActorGripInformation::GetGripPrimitiveComponent() const
{
	UPrimitiveComponent* RootComp = nullptr;

	if (GripTargetType == EGripTargetType::ActorGrip)
	{
		RootComp = Cast<UPrimitiveComponent>(GetGrippedActor()->GetRootComponent());
	}
	else
		RootComp = GetGrippedComponent();

	return RootComp;
}

bool FBPActorGripInformation::operator==(const UPrimitiveComponent* Other) const
{
	if (Other && GrippedObject && GrippedObject == (const UObject*)Other)
		return true;

	return false;
}