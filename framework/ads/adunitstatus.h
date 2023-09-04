#ifndef __AKK0RD_SDK_ADS_ADUNIT_STATUS_H__
#define __AKK0RD_SDK_ADS_ADUNIT_STATUS_H__

namespace ads {
    enum struct InterstitialStatus : uint8_t { NotInited, ReadyToLoad, Loading, LoadError, Loaded, Showing };
    enum struct RewardedVideoStatus : uint8_t { NotInited, ReadyToLoad, Loading, LoadError, Loaded, Showing };
};

#endif // __AKK0RD_SDK_ADS_ADUNIT_STATUS_H__