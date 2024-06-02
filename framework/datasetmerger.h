#ifndef __AKK0RD_SDK_DATASETMERGER_H__
#define __AKK0RD_SDK_DATASETMERGER_H__

#include <unordered_set>

namespace AkkordSDK {
    namespace Utils {
        template <typename T, typename U>
        auto mergeDatasets(T it1Begin, T it1End, U it2Begin, U it2End) {
            std::unordered_set<typename std::remove_const<typename std::remove_reference<decltype(*std::declval<T>())>::type>::type> result;

            for (; it1Begin != it1End; ++it1Begin) {
                result.emplace(*it1Begin);
            }

            for (; it2Begin != it2End; ++it2Begin) {
                result.emplace(*it2Begin);
            }


            return result;
        }
    }
}

#endif // __AKK0RD_SDK_DATASETMERGER_H__