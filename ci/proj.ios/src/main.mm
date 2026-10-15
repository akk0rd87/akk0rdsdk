// Пустое приложение для CI: обращается к iOS-частям SDK, чтобы сборка проверяла и компиляцию, и линковку
// (вызовы не дают линковщику выбросить код провайдеров). В CI приложение не запускается.
#import <UIKit/UIKit.h>
#include <memory>
#include "ads/admob/admobfactory.h"
#include "ads/yandex/yandexadsfactory.h"
#include "gdpr/gdpr_consentpolicymanager.h"
#include "firebase/analytics/ios/iosfirebaseanalytics.h"
#include "firebase/analytics/ios/iosadanalytics.h"
#include "configmanager.h"
#include "hpdf.h"

namespace {
    struct CiProviderCallback : public ads::ProviderCallback {
        void eventCallback(ads::Event) const override {}
    };
}

@interface CiAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation CiAppDelegate {
    std::shared_ptr<CiProviderCallback> callback;
    std::shared_ptr<ads::Provider> admob;
    std::shared_ptr<ads::Provider> yandex;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.rootViewController = [[UIViewController alloc] init];
    [self.window makeKeyAndVisible];

    // Firebase без GoogleService-Info.plist не настраивается — в CI-проекте его нет
    if ([NSBundle.mainBundle pathForResource:@"GoogleService-Info" ofType:@"plist"]) {
        Firebase::Analytics::iOS::Init();
    }
    Firebase::Analytics::iOS::LogAdEvent(@"ci_start", @"ci", "");

    callback = std::make_shared<CiProviderCallback>();
    const auto formats = ads::Format::Interstitial | ads::Format::RewardedVideo;
    admob = ads::AdMob::createProvider(callback, formats);
    yandex = ads::Yandex::createProvider(callback, formats);

    GDPRConsentPolicy::Manager& gdpr = GDPRConsentPolicy::getManagerInstance();
    (void)gdpr;

    ConfigManager config;
    (void)config.GetIntValue("ci", 0);

    HPDF_Doc pdf = HPDF_New(nullptr, nullptr);
    HPDF_Free(pdf);
    return YES;
}
@end

int main(int argc, char *argv[]) {
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass(CiAppDelegate.class));
    }
}
