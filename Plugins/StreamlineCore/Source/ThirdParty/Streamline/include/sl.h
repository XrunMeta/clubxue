

#pragma once

#include <limits.h>

#include "sl_struct.h"
#include "sl_consts.h"
#include "sl_version.h"
#include "sl_result.h"
#include "sl_appidentity.h"
#include "sl_device_wrappers.h"

#include "sl_core_api.h"
#include "sl_core_types.h"

#define SL_FUN_DECL(name) PFun_##name* name{}

#define SL_FEATURE_FUN_IMPORT(feature, func) slGetFeatureFunction(feature, #func, (void*&) ##func)
#define SL_FEATURE_FUN_IMPORT_STATIC(feature, func)                             \
static PFun_##func* s_ ##func{};                                                \
if(!s_ ##func) {                                                                \
    sl::Result res = slGetFeatureFunction(feature, #func, (void*&) s_ ##func);  \
    if(res != sl::Result::eOk) return res;                                      \
}                                                                               \

