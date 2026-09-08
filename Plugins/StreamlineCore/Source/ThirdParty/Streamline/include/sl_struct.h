

#pragma once

#include <stdint.h>
#include <string.h>

namespace sl
{

struct StructType
{
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t  data4[8];

    inline bool operator==(const StructType& rhs) const { return memcmp(this, &rhs, sizeof(*this)) == 0; }
    inline bool operator!=(const StructType& rhs) const { return memcmp(this, &rhs, sizeof(*this)) != 0; }
};

constexpr uint32_t kStructVersion1 = 1;
constexpr uint32_t kStructVersion2 = 2;
constexpr uint32_t kStructVersion3 = 3;
constexpr uint32_t kStructVersion4 = 4;
constexpr uint32_t kStructVersion5 = 5;

struct BaseStructure
{
    BaseStructure() = delete;
    BaseStructure(StructType t, uint32_t v) : structType(t), structVersion(v) {};
    BaseStructure* next{};
    StructType structType{};
    size_t structVersion;
};

#define SL_STRUCT_BEGIN(name, guid, version)                                \
struct name : public sl::BaseStructure                                      \
{                                                                           \
    name() : sl::BaseStructure(guid, version){}                             \
    constexpr static sl::StructType s_structType = guid;

#define SL_STRUCT_END() };

#define SL_STRUCT_PROTECTED_BEGIN(name, guid, version)                      \
struct name : public sl::BaseStructure                                      \
{                                                                           \
protected:                                                                  \
    name() : sl::BaseStructure(guid, version){}                             \
public:                                                                     \
    constexpr static sl::StructType s_structType = guid;                    \

#define SL_STRUCT(name, guid, version)                                      \
SL_STRUCT_BEGIN(name, guid, version)

#define SL_STRUCT_PROTECTED(name, guid, version)                            \
SL_STRUCT_PROTECTED_BEGIN(name, guid, version)

} 
