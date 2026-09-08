#pragma once

#include <cstdint>

#include "sl_struct.h"

namespace sl
{

enum class EngineType : uint32_t
{
    eCustom,
    eUnreal,
    eUnity,
    eCount
};

}
