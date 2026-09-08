

#include "StreamlineNGXCommon.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "UObject/Class.h"

#define LOCTEXT_NAMESPACE "FStreamlineNGXCommonModule"
DEFINE_LOG_CATEGORY_STATIC(LogStreamlineNGXCommon, Log, All);

class FStreamlineNGXCommonModule : public IModuleInterface
{
public:

	virtual void StartupModule() override 
	{
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("%s Enter"), ANSI_TO_TCHAR(__FUNCTION__));
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("FApp::CanEverRender                    =%d"), FApp::CanEverRender());
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("FApp::CanEverRenderOrProduceRenderData =%d"), FApp::CanEverRenderOrProduceRenderData());
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("IsRunningCommandlet        =%d"), IsRunningCommandlet());
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("IsRunningCookCommandlet    =%d"), IsRunningCookCommandlet());
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("IsRunningDLCCookCommandlet =%d"), IsRunningDLCCookCommandlet());
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("IsRunningCookOnTheFly      =%d"), IsRunningCookOnTheFly());
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("IsAllowCommandletRendering =%d"), IsAllowCommandletRendering());

#if UE_VERSION_AT_LEAST(5,6,0)
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("GetRunningCommandletClass = '%s' GetCommandletNameFromCmdline() = '%s'"), 
			GetRunningCommandletClass() ? *GetRunningCommandletClass()->GetName() : TEXT("nullptr"), *GetCommandletNameFromCmdline());
#endif
		UE_LOG(LogStreamlineNGXCommon, Verbose, TEXT("%s Leave"), ANSI_TO_TCHAR(__FUNCTION__));
	};
	virtual void ShutdownModule() override 
	{
	};

};

IMPLEMENT_MODULE(FStreamlineNGXCommonModule, StreamlineNGXCommon);