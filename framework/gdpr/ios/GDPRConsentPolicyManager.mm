#include <atomic>
#include <UserMessagingPlatform/UserMessagingPlatform.h>
#import <AppTrackingTransparency/AppTrackingTransparency.h>
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

        // Окно ATT показывается до инициализации рекламы (до сбора данных для трекинга).
        // Запрос, сделанный не в активном состоянии приложения (например, во время запуска или пока
        // закрывается форма UMP), система молча отбрасывает: окна нет, ответ сразу NotDetermined.
        // Поэтому ждём UIApplicationStateActive и повторяем запрос, если окно так и не появилось.
        void requestTrackingAuthorizationIfNeeded(void (^completion)(void), int attemptsLeft = 3) {
            if (!completion) {
                return;
            }

            if (@available(iOS 14, *)) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (ATTrackingManager.trackingAuthorizationStatus != ATTrackingManagerAuthorizationStatusNotDetermined
                        || attemptsLeft <= 0) {
                        completion();
                        return;
                    }

                    if (UIApplication.sharedApplication.applicationState != UIApplicationStateActive) {
                        __block id observer = [NSNotificationCenter.defaultCenter
                            addObserverForName:UIApplicationDidBecomeActiveNotification
                                        object:nil
                                         queue:NSOperationQueue.mainQueue
                                    usingBlock:^(__unused NSNotification *note) {
                                        [NSNotificationCenter.defaultCenter removeObserver:observer];
                                        requestTrackingAuthorizationIfNeeded(completion, attemptsLeft);
                                    }];
                        return;
                    }

                    [ATTrackingManager requestTrackingAuthorizationWithCompletionHandler:^(ATTrackingManagerAuthorizationStatus status) {
                        if (status == ATTrackingManagerAuthorizationStatusNotDetermined) {
                            // окно не показалось — повторим чуть позже
                            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                                requestTrackingAuthorizationIfNeeded(completion, attemptsLeft - 1);
                            });
                            return;
                        }
                        completion();
                    }];
                });
                return;
            }

            completion();
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
                            if (UMPConsentInformation.sharedInstance.canRequestAds) {
                                requestTrackingAuthorizationIfNeeded(^{
                                    sendCallback();
                                });
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

            // Согласие уже получено при прошлом запуске: реклама не ждёт сети (и работает без неё),
            // но всё равно только после ответа на ATT.
            if (UMPConsentInformation.sharedInstance.canRequestAds) {
                requestTrackingAuthorizationIfNeeded(^{
                    sendCallback();
                });
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