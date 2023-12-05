#include <UserMessagingPlatform/UserMessagingPlatform.h>
#include "../gdpr_consentpolicymanager.h"

namespace GDPRConsentPolicy {
    class iOSGDPRManager : public GDPRConsentPolicy::Manager {
    private:
        void sendCallback() {

        }

        virtual void requestConsent() override {
            // Create a UMPRequestParameters object.
            UMPRequestParameters *parameters = [[UMPRequestParameters alloc] init];
            // Set tag for under age of consent. NO means users are not under age
            // of consent.
            parameters.tagForUnderAgeOfConsent = NO;

            auto *rootController = [[[[UIApplication sharedApplication]delegate] window] rootViewController];

            __weak __typeof__(rootController) weakSelf = rootController;
            // Request an update for the consent information.
            [UMPConsentInformation.sharedInstance
                requestConsentInfoUpdateWithParameters:parameters
                    completionHandler:^(NSError *_Nullable requestConsentError) {
                        if (requestConsentError) {
                            // Consent gathering failed.
                            NSLog(@"Error: %@", requestConsentError.localizedDescription);
                            return;
                        }

                        __strong __typeof__(rootController) strongSelf = weakSelf;
                        if (!strongSelf) {
                            return;
                        }

                        [UMPConsentForm loadAndPresentIfRequiredFromViewController:strongSelf
                            completionHandler:^(NSError *loadAndPresentError) {
                                if (loadAndPresentError) {
                                    // Consent gathering failed.
                                    NSLog(@"Error: %@", loadAndPresentError.localizedDescription);
                                    return;
                                }

                                // Consent has been gathered.

                                if (UMPConsentInformation.sharedInstance.canRequestAds) {
                                    sendCallback();
                                }
                            }];
                    }];

            if (UMPConsentInformation.sharedInstance.canRequestAds) {
                sendCallback();
            }
        }
    };
}

static GDPRConsentPolicy::iOSGDPRManager iosGDPRManager;

GDPRConsentPolicy::Manager& GDPRConsentPolicy::getManagerInstance() {
    return iosGDPRManager;
}