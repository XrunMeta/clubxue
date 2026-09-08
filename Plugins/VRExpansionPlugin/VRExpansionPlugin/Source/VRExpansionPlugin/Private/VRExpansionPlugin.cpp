

#include "VRExpansionPlugin.h"

#include "Grippables/GrippablePhysicsReplication.h"

#include "VRGlobalSettings.h"
#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "Physics/Experimental/PhysScene_Chaos.h"

#define LOCTEXT_NAMESPACE "FVRExpansionPluginModule"

void FVRExpansionPluginModule::StartupModule()
{

	RegisterSettings();

	FPhysScene_Chaos::PhysicsReplicationFactory = MakeShared<IPhysicsReplicationFactoryVR>();
}

void FVRExpansionPluginModule::ShutdownModule()
{

	UnregisterSettings();
}

void FVRExpansionPluginModule::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{

		ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

		SettingsModule->RegisterSettings("Project", "Plugins", "VRExpansionPlugin",
			LOCTEXT("VRExpansionSettingsName", "VRExpansion Settings"),
			LOCTEXT("VRExpansionSettingsDescription", "Configure global settings for the VRExpansionPlugin"),
			GetMutableDefault<UVRGlobalSettings>());
	}
}

void FVRExpansionPluginModule::UnregisterSettings()
{

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "VRExpansionPlugin");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVRExpansionPluginModule, VRExpansionPlugin)