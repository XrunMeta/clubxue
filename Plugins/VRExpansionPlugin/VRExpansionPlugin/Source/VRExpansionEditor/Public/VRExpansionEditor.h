

#pragma once

#include "Runtime/Core/Public/Modules/ModuleInterface.h"

#include "ComponentVisualizer.h"

class FVRExpansionEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void RegisterComponentVisualizer(FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer);

	TArray<FName> RegisteredComponentClassNames;
};