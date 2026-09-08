

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogStreamlineRHI, Log, All);

bool slVerifyEmbeddedSignature(const FString& PathToBinary);

bool LoadStreamlineFunctionPointers(const FString& InterposerBinaryPath);
void SetStreamlineAPILoggingEnabled(bool bEnabled);

#if defined(__clang__)
#define SL_DISABLE_DEPRECATED_WARNINGS \
        _Pragma("clang diagnostic push") \
        _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define SL_RESTORE_DEPRECATED_WARNINGS \
        _Pragma("clang diagnostic pop")

#elif defined(_MSC_VER)
#define DLSS_DISABLE_DEPRECATED_WARNINGS \
        __pragma(warning(push)) \
        __pragma(warning(disable: 4996))
#define DLSS_RESTORE_DEPRECATED_WARNINGS \
        __pragma(warning(pop))

#else
#define SL_DISABLE_DEPRECATED_WARNINGS
#define SL_RESTORE_DEPRECATED_WARNINGS
#endif

