

#if WITH_EDITOR

#include "ArmASRSettings.h"
#include "HAL/IConsoleManager.h"

const FString UArmASRSettings::GeneralSettings = TEXT("General Settings");
const FString UArmASRSettings::QualitySettings = TEXT("Quality Settings");
const FString UArmASRSettings::ReactiveMaskSettings = TEXT("Reactive Mask Settings");

FName UArmASRSettings::GetContainerName() const
{
	static const FName ContainerName("Project");
	return ContainerName;
}

FName UArmASRSettings::GetCategoryName() const
{
	static const FName EditorCategoryName("Plugins");
	return EditorCategoryName;
}

FName UArmASRSettings::GetSectionName() const
{
	static const FName EditorSectionName("Arm ASR");
	return EditorSectionName;
}

float UArmASRSettings::GetClampedFloatPropertyValue(FFloatProperty *FloatProp, float OrigCVarValue) const
{
	float ClampedCVarValue = OrigCVarValue;

	if (FloatProp->HasMetaData("ClampMin"))
	{
		float min = FloatProp->GetFloatMetaData("ClampMin");
		ClampedCVarValue = FMath::Max(ClampedCVarValue, min);
	}

	if (FloatProp->HasMetaData("ClampMax"))
	{
		float max = FloatProp->GetFloatMetaData("ClampMax");
		ClampedCVarValue = FMath::Min(ClampedCVarValue, max);
	}

	return ClampedCVarValue;
}

int32 UArmASRSettings::GetClampedEnumPropertyValue(FEnumProperty *EnumProp, int32 OrigCVarValue) const
{
	int32 ClampedCVarValue = OrigCVarValue;

	if (EnumProp->HasMetaData("ClampMin"))
	{
		int32 min = EnumProp->GetIntMetaData("ClampMin");
		ClampedCVarValue = FMath::Max(ClampedCVarValue, min);
	}

	if (EnumProp->HasMetaData("ClampMax"))
	{
		float max = EnumProp->GetIntMetaData("ClampMax");
		ClampedCVarValue = FMath::Min(ClampedCVarValue, max);
	}

	return ClampedCVarValue;
}

void UArmASRSettings::SyncConsoleVariablesWithUI(bool SetConsoleVars, IConsoleVariable* UpdatedCVar)
{

	for (TFieldIterator<FProperty> PropertyIterator(GetClass()); PropertyIterator; ++PropertyIterator)
	{
		FProperty* Property = *PropertyIterator;

		FString CVarName = Property->GetMetaData("ConsoleVariable");

		IConsoleVariable *CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);

		if (CVar)
		{

			if (UpdatedCVar && CVar != UpdatedCVar) continue;

			void* Data = Property->ContainerPtrToValuePtr<void>(this);

			if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
			{
				if (SetConsoleVars)
				{
					CVSetFromUI = CVar;
					CVar->Set(BoolProp->GetPropertyValue(Data), ECVF_SetByConsole);
				}
				else
				{
					int32 CVarValue = CVar->GetInt();
					BoolProp->SetPropertyValue_InContainer(this, CVarValue != 0);
				}
			}

			else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Property))
			{
				if (SetConsoleVars)
				{
					CVSetFromUI = CVar;
					CVar->Set(FloatProp->GetPropertyValue(Data), ECVF_SetByConsole);
				}
				else
				{
					float CVarValue = CVar->GetFloat();
					float ClampedCVarValue = GetClampedFloatPropertyValue(FloatProp, CVarValue);

					FloatProp->SetPropertyValue_InContainer(this, ClampedCVarValue);

					if (ClampedCVarValue != CVarValue)
					{
						CVSetFromUI = CVar;
						CVar->Set(ClampedCVarValue, ECVF_SetByConsole);
					}
				}
			}

			else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
			{
				if (FNumericProperty* UnderlyingProp =
						CastField<FNumericProperty>(EnumProp->GetUnderlyingProperty()))
				{
					if (SetConsoleVars)
					{
						CVSetFromUI = CVar;

						int32 EnumValue = UnderlyingProp->GetSignedIntPropertyValue(Data);
						CVar->Set(EnumValue, ECVF_SetByConsole);
					}
					else
					{
						int32 CVarValue = CVar->GetInt();
						int32 ClampedCVarValue = GetClampedEnumPropertyValue(EnumProp, CVarValue);

						UnderlyingProp->SetIntPropertyValue(Data, static_cast<int64>(ClampedCVarValue));

						if (ClampedCVarValue != CVarValue)
						{
							CVSetFromUI = CVar;
							CVar->Set(ClampedCVarValue, ECVF_SetByConsole);
						}
					}
				}
			}

			CVSetFromUI = nullptr;

			if (UpdatedCVar) break;
		}
	}
}

void UArmASRSettings::OnConsoleVariablesUpdated(IConsoleVariable* CVar)
{
	if (CVar)
	{

		if (CVar != CVSetFromUI)
		{
			SyncConsoleVariablesWithUI(false, CVar);
		}
		SaveConfig();
	}
}

void UArmASRSettings::PostInitProperties()
{
	Super::PostInitProperties();

	SyncConsoleVariablesWithUI(false);

	for (TFieldIterator<FProperty> PropertyIterator(GetClass()); PropertyIterator; ++PropertyIterator)
	{
		FProperty *Property = *PropertyIterator;
		FString CVarName = Property->GetMetaData("ConsoleVariable");

		IConsoleVariable *CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);

		if (CVar)
		{
			CVar->SetOnChangedCallback(
				FConsoleVariableDelegate::CreateUObject(this, &UArmASRSettings::OnConsoleVariablesUpdated));
		}
	}
}

void UArmASRSettings::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SyncConsoleVariablesWithUI(true);
}

#endif
