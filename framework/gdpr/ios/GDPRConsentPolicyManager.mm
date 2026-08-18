#include <atomic>
#include <UserMessagingPlatform/UserMessagingPlatform.h>
#include "../gdpr_consentpolicymanager.h"

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
                        UIViewController *rootController = [UIApplication sharedApplication].keyWindow.rootViewController;
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
            auto *rootController = [UIApplication sharedApplication].keyWindow.rootViewController;
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