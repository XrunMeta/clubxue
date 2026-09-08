

#pragma once

#define SL_CHECK(f) {auto _r = f; if(_r != sl::Result::eOk) return _r;}
#define SL_FAILED(r, f) sl::Result r = f; r != sl::Result::eOk
#define SL_SUCCEEDED(r, f) sl::Result r = f; r == sl::Result::eOk

namespace sl
{

enum class Result
{
    eOk,
    eErrorIO,
    eErrorDriverOutOfDate,
    eErrorOSOutOfDate,
    eErrorOSDisabledHWS,
    eErrorDeviceNotCreated,
    eErrorNoSupportedAdapterFound,
    eErrorAdapterNotSupported,
    eErrorNoPlugins,
    eErrorVulkanAPI,
    eErrorDXGIAPI,
    eErrorD3DAPI,

    eErrorNRDAPI,
    eErrorNVAPI,
    eErrorReflexAPI,
    eErrorNGXFailed,
    eErrorJSONParsing,
    eErrorMissingProxy,
    eErrorMissingResourceState,
    eErrorInvalidIntegration,
    eErrorMissingInputParameter,
    eErrorNotInitialized,
    eErrorComputeFailed,
    eErrorInitNotCalled,
    eErrorExceptionHandler,
    eErrorInvalidParameter,
    eErrorMissingConstants,
    eErrorDuplicatedConstants,
    eErrorMissingOrInvalidAPI,
    eErrorCommonConstantsMissing,
    eErrorUnsupportedInterface,
    eErrorFeatureMissing,
    eErrorFeatureNotSupported,
    eErrorFeatureMissingHooks,
    eErrorFeatureFailedToLoad,
    eErrorFeatureWrongPriority,
    eErrorFeatureMissingDependency,
    eErrorFeatureManagerInvalidState,
    eErrorInvalidState,
    eWarnOutOfVRAM
};

}
