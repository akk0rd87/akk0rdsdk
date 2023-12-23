#ifndef __AKK0RD_SDK_IOS_WRAPPER_UTILS_H__
#define __AKK0RD_SDK_IOS_WRAPPER_UTILS_H__

#import <UIKit/UIKit.h>

namespace iosWrapper {
    UIViewController* getRootViewController();
    void showViewOverRootViewController(UIActivityViewController *activityVC);
}

#endif // __AKK0RD_SDK_IOS_WRAPPER_UTILS_H__