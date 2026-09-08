
#include "VRGlobalSettings.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(VRGlobalSettings)

#include "Chaos/ChaosConstraintSettings.h"
#include "Grippables/GrippableSkeletalMeshComponent.h"

UVRGlobalSettings::UVRGlobalSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	bLerpHybridWithSweepGrips(true),
	bOnlyLerpHybridRotation(false),
	bHybridWithSweepUseDistanceBasedLerp(true),
	HybridWithSweepLerpDuration(0.2f),
	bUseGlobalLerpToHand(false),
	bSkipLerpToHandIfHeld(false),
	MinDistanceForLerp(10.0f),
	LerpDuration(0.25f),
	MinSpeedForLerp(100.f),
	MaxSpeedForLerp(500.f),
	LerpInterpolationMode(EVRLerpInterpolationMode::QuatInterp),
	bUseCurve(false),
	OneEuroMinCutoff(0.1f),
	OneEuroCutoffSlope(10.0f),
	OneEuroDeltaCutoff(10.0f),
	CurrentControllerProfileInUse(NAME_None),
	CurrentControllerProfileTransform(FTransform::Identity),
	bUseSeperateHandTransforms(false),
	CurrentControllerProfileTransformRight(FTransform::Identity)
{
		DefaultGrippableCharacterMeshComponentClass = UGrippableSkeletalMeshComponent::StaticClass();

		bUseCollisionModificationForCollisionIgnore = false;
		CollisionIgnoreSubsystemUpdateRate = 1.f;

		bUseChaosTranslationScalers = false;
		bSetEngineChaosScalers = false;
		LinearDriveStiffnessScale = 1.0f;
		LinearDriveDampingScale = 1.0f;
		AngularDriveStiffnessScale = 0.3f; 
		AngularDriveDampingScale = 0.3f; 

		JointStiffness = 1.0f;
		SoftLinearStiffnessScale = 1.5f;
		SoftLinearDampingScale = 1.2f;
		SoftAngularStiffnessScale = 100000.f;
		SoftAngularDampingScale = 1000.f;
		JointLinearBreakScale = 1.0f; 
		JointAngularBreakScale = 1.0f; 

}

TSubclassOf<class UGrippableSkeletalMeshComponent> UVRGlobalSettings::GetDefaultGrippableCharacterMeshComponentClass()
{
	const UVRGlobalSettings* VRSettings = GetDefault<UVRGlobalSettings>();

	if (VRSettings)
	{

		if (VRSettings->DefaultGrippableCharacterMeshComponentClass != nullptr)
		{
			return VRSettings->DefaultGrippableCharacterMeshComponentClass;
		}
	}

	return UGrippableSkeletalMeshComponent::StaticClass();
}

bool UVRGlobalSettings::IsGlobalLerpEnabled()
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();
	return VRSettings.bUseGlobalLerpToHand;
}

FTransform UVRGlobalSettings::AdjustTransformByControllerProfile(FName OptionalControllerProfileName, const FTransform& SocketTransform, bool bIsRightHand)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	if (OptionalControllerProfileName == NAME_None)
	{
		if (VRSettings.CurrentControllerProfileInUse != NAME_None)
		{

			return SocketTransform * (((bIsRightHand && VRSettings.bUseSeperateHandTransforms) ? VRSettings.CurrentControllerProfileTransformRight : VRSettings.CurrentControllerProfileTransform));
		}

		return SocketTransform;
	}

	const FBPVRControllerProfile* FoundProfile = VRSettings.ControllerProfiles.FindByPredicate([OptionalControllerProfileName](const FBPVRControllerProfile& ArrayItem)
		{
			return ArrayItem.ControllerName == OptionalControllerProfileName;
		});

	if (FoundProfile)
	{
		return SocketTransform * (((bIsRightHand && VRSettings.bUseSeperateHandTransforms) ? FoundProfile->SocketOffsetTransformRightHand : FoundProfile->SocketOffsetTransform));
	}

	return SocketTransform;
}

FTransform UVRGlobalSettings::AdjustTransformByGivenControllerProfile(UPARAM(ref) FBPVRControllerProfile& ControllerProfile, const FTransform& SocketTransform, bool bIsRightHand)
{

	return SocketTransform * (((bIsRightHand && ControllerProfile.bUseSeperateHandOffsetTransforms) ? ControllerProfile.SocketOffsetTransformRightHand : ControllerProfile.SocketOffsetTransform));
}

TArray<FBPVRControllerProfile> UVRGlobalSettings::GetControllerProfiles()
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	return VRSettings.ControllerProfiles;
}

void UVRGlobalSettings::OverwriteControllerProfile(UPARAM(ref)FBPVRControllerProfile& OverwritingProfile, bool bSaveOutToConfig)
{
	UVRGlobalSettings& VRSettings = *GetMutableDefault<UVRGlobalSettings>();

	for (int i = 0; i < VRSettings.ControllerProfiles.Num(); ++i)
	{
		if (VRSettings.ControllerProfiles[i].ControllerName == OverwritingProfile.ControllerName)
		{
			VRSettings.ControllerProfiles[i] = OverwritingProfile;
		}
	}

	if (bSaveOutToConfig)
		SaveControllerProfiles();
}

void UVRGlobalSettings::AddControllerProfile(UPARAM(ref)FBPVRControllerProfile& NewProfile, bool bSaveOutToConfig)
{
	UVRGlobalSettings& VRSettings = *GetMutableDefault<UVRGlobalSettings>();

	VRSettings.ControllerProfiles.Add(NewProfile);

	if (bSaveOutToConfig)
		SaveControllerProfiles();
}

void UVRGlobalSettings::DeleteControllerProfile(FName ControllerProfileName, bool bSaveOutToConfig)
{
	UVRGlobalSettings& VRSettings = *GetMutableDefault<UVRGlobalSettings>();

	for (int i = VRSettings.ControllerProfiles.Num() - 1; i >= 0; --i)
	{
		if (VRSettings.ControllerProfiles[i].ControllerName == ControllerProfileName)
		{
			VRSettings.ControllerProfiles.RemoveAt(i);
		}
	}

	if (bSaveOutToConfig)
		SaveControllerProfiles();
}

void UVRGlobalSettings::SaveControllerProfiles()
{
	UVRGlobalSettings& VRSettings = *GetMutableDefault<UVRGlobalSettings>();
	VRSettings.SaveConfig();

}

FName UVRGlobalSettings::GetCurrentProfileName(bool& bHadLoadedProfile)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	bHadLoadedProfile = VRSettings.CurrentControllerProfileInUse != NAME_None;
	return VRSettings.CurrentControllerProfileInUse;
}

FBPVRControllerProfile UVRGlobalSettings::GetCurrentProfile(bool& bHadLoadedProfile)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	FName ControllerProfileName = VRSettings.CurrentControllerProfileInUse;
	const FBPVRControllerProfile* FoundProfile = VRSettings.ControllerProfiles.FindByPredicate([ControllerProfileName](const FBPVRControllerProfile& ArrayItem)
		{
			return ArrayItem.ControllerName == ControllerProfileName;
		});

	bHadLoadedProfile = FoundProfile != nullptr;

	if (bHadLoadedProfile)
	{
		return *FoundProfile;
	}
	else
		return FBPVRControllerProfile();
}

bool UVRGlobalSettings::GetControllerProfile(FName ControllerProfileName, FBPVRControllerProfile& OutProfile)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	const FBPVRControllerProfile* FoundProfile = VRSettings.ControllerProfiles.FindByPredicate([ControllerProfileName](const FBPVRControllerProfile& ArrayItem)
		{
			return ArrayItem.ControllerName == ControllerProfileName;
		});

	if (FoundProfile)
	{
		OutProfile = *FoundProfile;
		return true;
	}

	return false;
}

bool UVRGlobalSettings::LoadControllerProfileByName(FName ControllerProfileName, bool bSetAsCurrentProfile)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	const FBPVRControllerProfile* FoundProfile = VRSettings.ControllerProfiles.FindByPredicate([ControllerProfileName](const FBPVRControllerProfile& ArrayItem)
		{
			return ArrayItem.ControllerName == ControllerProfileName;
		});

	if (FoundProfile)
	{
		return LoadControllerProfile(*FoundProfile, bSetAsCurrentProfile);
	}

	UE_LOGF(LogTemp, Warning, "Could not find controller profile!: %ls", *ControllerProfileName.ToString());
	return false;
}

bool UVRGlobalSettings::LoadControllerProfile(const FBPVRControllerProfile& ControllerProfile, bool bSetAsCurrentProfile)
{

	if (bSetAsCurrentProfile)
	{
		UVRGlobalSettings* VRSettings = GetMutableDefault<UVRGlobalSettings>();
		if (VRSettings)
		{
			VRSettings->CurrentControllerProfileInUse = ControllerProfile.ControllerName;
			VRSettings->CurrentControllerProfileTransform = ControllerProfile.SocketOffsetTransform;
			ensure(!VRSettings->CurrentControllerProfileTransform.ContainsNaN());
			VRSettings->bUseSeperateHandTransforms = ControllerProfile.bUseSeperateHandOffsetTransforms;
			VRSettings->CurrentControllerProfileTransformRight = ControllerProfile.SocketOffsetTransformRightHand;
			ensure(!VRSettings->CurrentControllerProfileTransformRight.ContainsNaN());
			VRSettings->OnControllerProfileChangedEvent.Broadcast();
		}
		else
			return false;
	}

	return true;
}

void UVRGlobalSettings::PostInitProperties()
{
#if WITH_EDITOR

#endif

	SetScalers();

	Super::PostInitProperties();
}

#if WITH_EDITOR

void UVRGlobalSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* PropertyThatChanged = PropertyChangedEvent.Property;

	if (PropertyThatChanged != nullptr)
	{
#if WITH_EDITORONLY_DATA
		if (
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, bUseChaosTranslationScalers) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, bSetEngineChaosScalers) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, LinearDriveStiffnessScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, LinearDriveDampingScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, AngularDriveStiffnessScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, AngularDriveDampingScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, JointStiffness) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, SoftLinearStiffnessScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, SoftLinearDampingScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, SoftAngularStiffnessScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, SoftAngularDampingScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, JointLinearBreakScale) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UVRGlobalSettings, JointAngularBreakScale)
			)
		{
			SetScalers();
		}
#endif
	}
}
#endif

void UVRGlobalSettings::SetScalers()
{
	auto CVarLinearDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinearDriveStiffnessScale"));
	auto CVarLinearDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinaearDriveDampingScale"));
	auto CVarAngularDriveStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveStiffnessScale"));
	auto CVarAngularDriveDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularDriveDampingScale"));

	auto CVarJointStiffness = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.JointStiffness"));
	auto CVarSoftLinearStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.SoftLinearStiffnessScale"));
	auto CVarSoftLinearDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.SoftLinearDampingScale"));
	auto CVarSoftAngularStiffnessScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.SoftAngularStiffnessScale"));
	auto CVarSoftAngularDampingScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.SoftAngularDampingScale"));
	auto CVarJointLinearBreakScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.LinearBreakScale"));
	auto CVarJointAngularBreakScale = IConsoleManager::Get().FindConsoleVariable(TEXT("p.Chaos.JointConstraint.AngularBreakScale"));

	if (bUseChaosTranslationScalers && bSetEngineChaosScalers)
	{
		CVarLinearDriveStiffnessScale->Set(LinearDriveStiffnessScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarLinearDriveDampingScale->Set(LinearDriveDampingScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarAngularDriveStiffnessScale->Set(AngularDriveStiffnessScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarAngularDriveDampingScale->Set(AngularDriveDampingScale, EConsoleVariableFlags::ECVF_SetByCode);

		CVarJointStiffness->Set(JointStiffness, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftLinearStiffnessScale->Set(SoftLinearStiffnessScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftLinearDampingScale->Set(SoftLinearDampingScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftAngularStiffnessScale->Set(SoftAngularStiffnessScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftAngularDampingScale->Set(SoftAngularDampingScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarJointLinearBreakScale->Set(JointLinearBreakScale, EConsoleVariableFlags::ECVF_SetByCode);
		CVarJointAngularBreakScale->Set(JointAngularBreakScale, EConsoleVariableFlags::ECVF_SetByCode);
	}
	else if (!bSetEngineChaosScalers)
	{
		CVarLinearDriveStiffnessScale->Set(1.0f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarLinearDriveDampingScale->Set(1.0f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarAngularDriveStiffnessScale->Set(1.5f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarAngularDriveDampingScale->Set(1.5f, EConsoleVariableFlags::ECVF_SetByCode);

		CVarJointStiffness->Set(1.0f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftLinearStiffnessScale->Set(1.5f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftLinearDampingScale->Set(1.2f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftAngularStiffnessScale->Set(100000.f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarSoftAngularDampingScale->Set(1000.f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarJointLinearBreakScale->Set(1.0f, EConsoleVariableFlags::ECVF_SetByCode);
		CVarJointAngularBreakScale->Set(1.0f, EConsoleVariableFlags::ECVF_SetByCode);
	}
}

void UVRGlobalSettings::GetMeleeSurfaceGlobalSettings(TArray<FBPHitSurfaceProperties>& OutMeleeSurfaceSettings)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();
	OutMeleeSurfaceSettings = VRSettings.MeleeSurfaceSettings;
}

void UVRGlobalSettings::GetVirtualStockGlobalSettings(FBPVirtualStockSettings& OutVirtualStockSettings)
{
	const UVRGlobalSettings& VRSettings = *GetDefault<UVRGlobalSettings>();

	OutVirtualStockSettings.bUseDistanceBasedStockSnapping = VRSettings.VirtualStockSettings.bUseDistanceBasedStockSnapping;
	OutVirtualStockSettings.StockSnapDistance = VRSettings.VirtualStockSettings.StockSnapDistance;
	OutVirtualStockSettings.StockSnapOffset = VRSettings.VirtualStockSettings.StockSnapOffset;
	OutVirtualStockSettings.bSmoothStockHand = VRSettings.VirtualStockSettings.bSmoothStockHand;
	OutVirtualStockSettings.SmoothingValueForStock = VRSettings.VirtualStockSettings.SmoothingValueForStock;
}

void UVRGlobalSettings::SaveVirtualStockGlobalSettings(FBPVirtualStockSettings NewVirtualStockSettings)
{
	UVRGlobalSettings& VRSettings = *GetMutableDefault<UVRGlobalSettings>();
	VRSettings.VirtualStockSettings = NewVirtualStockSettings;

	VRSettings.SaveConfig();
}
