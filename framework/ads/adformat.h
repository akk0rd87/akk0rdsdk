#ifndef __AKK0RD_SDK_ADS_FORMAT_H__
#define __AKK0RD_SDK_ADS_FORMAT_H__

#include <cstdint>

namespace ads {
    enum struct Format : uint8_t { Interstitial = 1, RewardedVideo = 2 };

    inline Format operator | (Format a, Format b) {
        return static_cast<Format>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
    }

    inline Format operator & (Format a, Format b) {
        return static_cast<Format>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
    }

    inline bool operator == (Format a, Format b) {
        return static_cast<uint8_t>(a) == static_cast<uint8_t>(b);
    }

    inline bool operator!(Format a) {
        return (static_cast<uint8_t>(a) == 0);
    }
};
#endif