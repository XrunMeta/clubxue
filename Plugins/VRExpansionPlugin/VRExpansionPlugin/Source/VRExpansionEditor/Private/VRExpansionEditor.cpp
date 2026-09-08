

#include "VRExpansionEditor.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "Grippables/HandSocketComponent.h"
#include "PropertyEditorModule.h"
#include "HandSocketVisualizer.h"
#include "HandSocketComponentDetails.h"
#include "VRGlobalSettingsDetails.h"
#include "VRGlobalSettings.h"

IMPLEMENT_MODULE(FVRExpansionEditorModule, VRExpansionEditor);

void FVRExpansionEditorModule::StartupModule()
{
	RegisterComponentVisualizer(UHandSocketComponent::StaticClass()->GetFName(), MakeShareable(new FHandSocketVisualizer));

	{
		auto& PropertyModule = FModuleManager::LoadModuleChecked< FPropertyEditorModule >("PropertyEditor");

		PropertyModule.RegisterCustomClassLayout(
			UHandSocketComponent::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FHandSocketComponentDetails::MakeInstance)
		);

		PropertyModule.RegisterCustomClassLayout(
			UVRGlobalSettings::StaticClass()->GetFName(),
			FOnGetDetailCustomizationInstance::CreateStatic(&FVRGlobalSettingsDetails::MakeInstance)
		);

		PropertyModule.NotifyCustomizationModuleChanged();
	}

}

void FVRExpansionEditorModule::ShutdownModule()
{
	if (GUnrealEd != NULL)
	{

		for (FName ClassName : RegisteredComponentClassNames)
		{
			GUnrealEd->UnregisterComponentVisualizer(ClassName);
		}
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		auto& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

		PropertyModule.UnregisterCustomClassLayout(UHandSocketComponent::StaticClass()->GetFName());
		PropertyModule.UnregisterCustomClassLayout(UVRGlobalSettings::StaticClass()->GetFName());
	}
}

void FVRExpansionEditorModule::RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd != NULL)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}

	RegisteredComponentClassNames.Add(ComponentClassName);

	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}
