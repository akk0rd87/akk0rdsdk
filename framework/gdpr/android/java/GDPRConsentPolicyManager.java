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

public class GDPRConsentPolicyManager {
    private static String TAG = "SDL";
    private static ConsentInformation consentInformation = null;

    static void Initialize() {
        Log.d(TAG, "GDPRConsentPolicyManager: Initialize");

        ConsentDebugSettings debugSettings = new ConsentDebugSettings.Builder(org.akkord.lib.Utils.GetContext())
            .setDebugGeography(ConsentDebugSettings.DebugGeography.DEBUG_GEOGRAPHY_EEA)
            .addTestDeviceHashedId("42C1FEAB41C4B5C89BEA61FD2014F48B")
            .build();


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
            (OnConsentInfoUpdateSuccessListener) () -> {
                UserMessagingPlatform.loadAndShowConsentFormIfRequired(
                    org.akkord.lib.Utils.GetContext(),
                    (OnConsentFormDismissedListener) loadAndShowError -> {
                        if (loadAndShowError != null) {
                            // Consent gathering failed.
                            Log.w(TAG, String.format("%s: %s",
                                loadAndShowError.getErrorCode(),
                                loadAndShowError.getMessage()));
                        }

                        // Consent has been gathered.
                        Log.d(TAG, "GDPR: Consent has been gathered from callback function");
                    }
                );
            },
            (OnConsentInfoUpdateFailureListener) requestConsentError -> {
            // Consent gathering failed.
            Log.w(TAG, String.format("%s: %s",
                requestConsentError.getErrorCode(),
                requestConsentError.getMessage()));
            });

        if (consentInformation.canRequestAds()) {
            Log.d(TAG, "GDPR: consentInformation.canRequestAds returned true");
        } else {
            Log.d(TAG, "GDPR: consentInformation.canRequestAds returned false");
        }
    }
}