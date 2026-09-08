#pragma once
#include "Modules/ModuleManager.h"
class FClubXEmailModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};