

#pragma once
#include "Modules/ModuleManager.h"

#include "NGXRHI.h"

class FNGXD3D11RHIModule final : public INGXRHIModule
{
public:

	virtual void StartupModule();
	virtual void ShutdownModule();

	virtual TUniquePtr<NGXRHI> CreateNGXRHI(const FNGXRHICreateArguments& Arguments);
};