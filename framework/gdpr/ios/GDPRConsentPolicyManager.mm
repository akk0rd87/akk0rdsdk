#include <atomic>
#include <UserMessagingPlatform/UserMessagingPlatform.h>
#include "../gdpr_consentpolicymanager.h"
#include "core/platformwrapper/ios/ios_rootviewcontroller.h"

namespace GDPRConsentPolicy {
    class iOSGDPRManager : public GDPRConsentPolicy::Manager {
    private:
        std::atomic<bool> callBackSent;

        void sendCallback() {
            if(!callBackSent.exchange(true)) {
                onGDPRConsentGathered();
            }
        }

        virtual void requestConsent() override {
            callBackSent.store(false);

            // Create a UMPRequestParameters object.
            UMPRequestParameters *parameters = [[UMPRequestParameters alloc] init];
            // Set tag for under age of consent. NO means users are not under age of consent.
            parameters.tagForUnderAgeOfConsent = NO;

#if defined(DEBUG)
            // Проверка формы согласия на устройстве не из ЕЭЗ (только в отладочной сборке), аргументами запуска:
            //   -UMPDebugTestDeviceId <id>  — устройство считается находящимся в ЕЭЗ; id печатает в журнал сам UMP
            //                                 («To enable debug mode for this device, set: ...»)
            //   -UMPResetConsent YES        — забыть прежний ответ, чтобы форма показалась снова
            NSUserDefaults *arguments = NSUserDefaults.standardUserDefaults;
            NSString *testDeviceId = [arguments stringForKey:@"UMPDebugTestDeviceId"];
            if (testDeviceId.length > 0) {
                UMPDebugSettings *debugSettings = [[UMPDebugSettings alloc] init];
                debugSettings.testDeviceIdentifiers = @[testDeviceId];
                debugSettings.geography = UMPDebugGeographyEEA;
                parameters.debugSettings = debugSettings;
            }
            if ([arguments boolForKey:@"UMPResetConsent"]) {
                [UMPConsentInformation.sharedInstance reset];
            }
#endif

            // Request an update for the consent information.
            [UMPConsentInformation.sharedInstance
                requestConsentInfoUpdateWithParameters:parameters
                    completionHandler:^(NSError *_Nullable requestConsentError) {
                        if (requestConsentError) {
                            // Consent gathering failed.
                            NSLog(@"Error: %@", requestConsentError.localizedDescription);
                            return;
                        }

                        void (^afterFormHandling)(void) = ^{
                            // Consent has been gathered.
                            if (UMPConsentInformation.sharedInstance.canRequestAds) {
                                sendCallback();
                            }

                            if(isPrivacyOptionsRequired()) {
                                setPrivacyOptionsRequired();
                            }
                        };

                        // Looked up here (not captured when requestConsent() was called) because
                        // this completion fires after a network round trip; on SwiftUI's
                        // scene-based lifecycle, requestConsent() typically runs from
                        // application(_:didFinishLaunchingWithOptions:), before any window
                        // exists, so a value captured up front would still be nil here.
                        UIViewController *rootController = akkordRootViewController();
                        if (!rootController) {
                            // No form to present without a root view controller, but ads can
                            // still be unlocked if one isn't actually required.
                            afterFormHandling();
                            return;
                        }

                        [UMPConsentForm loadAndPresentIfRequiredFromViewController:rootController
                            completionHandler:^(NSError *loadAndPresentError) {
                                if (loadAndPresentError) {
                                    // Consent gathering failed.
                                    NSLog(@"Error: %@", loadAndPresentError.localizedDescription);
                                    return;
                                }

                                afterFormHandling();
                            }];
                    }];

            if (UMPConsentInformation.sharedInstance.canRequestAds) {
                sendCallback();
            }
        }

        virtual void showPrivacyOptionsForm() override {
            auto *rootController = akkordRootViewController();
            [UMPConsentForm presentPrivacyOptionsFormFromViewController:rootController
                completionHandler:^(NSError *_Nullable formError) {
                    if (formError) {
                        // Handle the error.
                        NSLog(@"Error: %@", formError.localizedDescription);
                    }
                }
            ];
        }

        bool isPrivacyOptionsRequired() {
            return UMPPrivacyOptionsRequirementStatusRequired == UMPConsentInformation.sharedInstance.privacyOptionsRequirementStatus;
        }
    };
}

static GDPRConsentPolicy::iOSGDPRManager iosGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return iosGDPRManager;
}