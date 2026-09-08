

#pragma once
#include "Misc/EngineVersionComparison.h"
#include "CoreGlobals.h"
#include "Misc/App.h"

#define UE_VERSION_AT_LEAST(MajorVersion, MinorVersion, PatchVersion) (!UE_VERSION_OLDER_THAN(MajorVersion, MinorVersion, PatchVersion))

inline TPair<bool, const TCHAR*> IsEngineExecutionModeSupported()
{

	if (!FApp::CanEverRender())
	{
		return MakeTuple(false, TEXT("Cannot ever render"));
	}

	if (IsRunningCommandlet())
	{
		return MakeTuple(false,TEXT("A command let is running"));
	}

	return MakeTuple(true, TEXT(""));
}
