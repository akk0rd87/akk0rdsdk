package org.akkord.lib;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import com.google.android.ump.ConsentInformation;
import com.google.android.ump.ConsentInformation.OnConsentInfoUpdateSuccessListener;
import com.google.android.ump.ConsentInformation.OnConsentInfoUpdateFailureListener;
import com.google.android.ump.ConsentRequestParameters;
import com.google.android.ump.FormError;
import com.google.android.ump.ConsentForm.OnConsentFormDismissedListener;
import com.google.android.ump.UserMessagingPlatform;
import com.google.android.ump.ConsentDebugSettings;

import java.util.Objects;
import java.util.concurrent.atomic.AtomicBoolean;

public class GDPRConsentPolicyManager {
    private static final AtomicBoolean isMobileAdsInitializeCalled = new AtomicBoolean(false);
    private static final String TAG = "SDL";
    private static org.akkord.lib.GDPRConsentPolicyObserver gdprConsentPolicyObserver = null;
    private static ConsentInformation consentInformation = null;
    private static native void GDPRCallback(int Code);

    public static final int GDPR_CONSENT_GATHERED = 0;
    public static final int PRIVACY_OPTIONS_REQUIRED = 1;

    private static void private_GDPRConsentReceived() {
        try {
            if (isMobileAdsInitializeCalled.getAndSet(true)) {
                return;
            }

            GDPRCallback(GDPR_CONSENT_GATHERED);
            if(null != gdprConsentPolicyObserver) {
                gdprConsentPolicyObserver.onGDPRConsentGathered();
            }
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    private static void private_SetPrivacyOptionsRequired() {
        try {
            GDPRCallback(PRIVACY_OPTIONS_REQUIRED);
            if(null != gdprConsentPolicyObserver) {
                gdprConsentPolicyObserver.setPrivacyOptionsRequired();
            }
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static void setObserver(org.akkord.lib.GDPRConsentPolicyObserver observer) {
        gdprConsentPolicyObserver = observer;
    }

    public static void Initialize() {
        try {
            private_Initialize();
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    public static void ShowPrivacyOptionsForm (){
        try {
            org.akkord.lib.Utils.GetContext().runOnUiThread(() -> {
                try {
                    UserMessagingPlatform.showPrivacyOptionsForm(
                        org.akkord.lib.Utils.GetContext(),
                        formError -> {
                            if (formError != null) {
                                Log.d(TAG, "showPrivacyOptionsForm: error: " + formError.getMessage());
                            }
                        }
                    );
                }
                catch(Exception e) {
                    Log.e(TAG, Objects.requireNonNull(e.getMessage()));
                }
            });
        }
        catch(Exception e) {
            Log.e(TAG, Objects.requireNonNull(e.getMessage()));
        }
    }

    private static void private_Initialize() {
        Log.d(TAG, "GDPRConsentPolicyManager: Initialize");
        isMobileAdsInitializeCalled.set(false);

        //ConsentDebugSettings debugSettings = new ConsentDebugSettings.Builder(org.akkord.lib.Utils.GetContext())
        //    .setDebugGeography(ConsentDebugSettings.DebugGeography.DEBUG_GEOGRAPHY_EEA)
        //    .addTestDeviceHashedId("42C1FEAB41C4B5C89BEA61FD2014F48B")
        //    .build();


        // Set tag for under age of consent. false means users are not under age
        // of consent.
        ConsentRequestParameters params = new ConsentRequestParameters
            .Builder()
            .setTagForUnderAgeOfConsent(false)
            //.setConsentDebugSettings(debugSettings)
            .build();

        consentInformation = UserMessagingPlatform.getConsentInformation(org.akkord.lib.Utils.GetContext());
        consentInformation.requestConsentInfoUpdate(
            org.akkord.lib.Utils.GetContext(),
            params,
            (OnConsentInfoUpdateSuccessListener) () -> UserMessagingPlatform.loadAndShowConsentFormIfRequired(
                org.akkord.lib.Utils.GetContext(),
                (OnConsentFormDismissedListener) loadAndShowError -> {
                    if (loadAndShowError != null) {
                        // Consent gathering failed.
                        Log.w(TAG, String.format("%s: %s",
                            loadAndShowError.getErrorCode(),
                            loadAndShowError.getMessage()));
                    }

                    // Consent has been gathered.
                    if (consentInformation.canRequestAds()) {
                        Log.d(TAG, "GDPR: Consent has been gathered from callback function");
                        private_GDPRConsentReceived();
                    }

                    if (isPrivacyOptionsRequired()) {
                        private_SetPrivacyOptionsRequired();
                    }
                }
            ),
            (OnConsentInfoUpdateFailureListener) requestConsentError -> {
            // Consent gathering failed.
            Log.w(TAG, String.format("%s: %s",
                requestConsentError.getErrorCode(),
                requestConsentError.getMessage()));
            });

        if (consentInformation.canRequestAds()) {
            private_GDPRConsentReceived();
            Log.d(TAG, "GDPR: consentInformation.canRequestAds returned true");
        } else {
            Log.d(TAG, "GDPR: consentInformation.canRequestAds returned false");
        }
    }

    private static boolean isPrivacyOptionsRequired() {
        return consentInformation.getPrivacyOptionsRequirementStatus() == ConsentInformation.PrivacyOptionsRequirementStatus.REQUIRED;
    }
}