package org.akkord.lib;

public interface GDPRConsentPolicyObserver {
    public void onGDPRConsentGathered();
    public void setPrivacyOptionsRequired();
}