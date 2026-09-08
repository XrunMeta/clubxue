

#pragma once

#define SL_VERSION_MAJOR 2
#define SL_VERSION_MINOR 11
#define SL_VERSION_PATCH 1

#include <cstdint>
#include <string>

namespace sl
{
constexpr uint64_t kSDKVersionMagic = 0xfedc;
constexpr uint64_t kSDKVersion = (uint64_t(SL_VERSION_MAJOR) << 48) | (uint64_t(SL_VERSION_MINOR) << 32) | (uint64_t(SL_VERSION_PATCH) << 16) | kSDKVersionMagic;

struct Version
{
    Version() : major(0), minor(0), build(0) {};
    Version(uint32_t v1, uint32_t v2, uint32_t v3) : major(v1), minor(v2), build(v3) {};

    inline operator bool() const { return major != 0 || minor != 0 || build != 0; }

    inline std::string toStr() const
    {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(build);
    }
    inline std::wstring toWStr() const
    {
        return std::to_wstring(major) + L"." + std::to_wstring(minor) + L"." + std::to_wstring(build);
    }
    inline std::wstring toWStrOTAId() const
    {
        return std::to_wstring((major << 16) | (minor << 8) | build);
    }
    inline bool operator==(const Version& rhs) const
    {
        return major == rhs.major && minor == rhs.minor && build == rhs.build;
    }
    inline bool operator>(const Version& rhs) const
    {
        if (major < rhs.major) return false;
        else if (major > rhs.major) return true;

        if (minor < rhs.minor) return false;
        else if (minor > rhs.minor) return true;

        if (build < rhs.build) return false;
        else if (build > rhs.build) return true;

        return false;
    };
    inline bool operator>=(const Version& rhs) const
    {
        return operator>(rhs) || operator==(rhs);
    };
    inline bool operator<(const Version& rhs) const
    {
        if (major > rhs.major) return false;
        else if (major < rhs.major) return true;

        if (minor > rhs.minor) return false;
        else if (minor < rhs.minor) return true;

        if (build > rhs.build) return false;
        else if (build < rhs.build) return true;

        return false;
    };
    inline bool operator<=(const Version& rhs) const
    {
        return operator<(rhs) || operator==(rhs);
    };

    uint32_t major;
    uint32_t minor;
    uint32_t build;
};

}
