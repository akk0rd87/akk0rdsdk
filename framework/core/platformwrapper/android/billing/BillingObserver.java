package org.akkord.lib;

public interface BillingObserver {
    public void BillingSetupFinished(int ResponseCode);
    public void BillingDisconnected();
    public void PurchaseQueried(String PurchaseToken, String ProductSKU, int Type);
    public void PurchaseConsumed(String PurchaseToken, String ProductSKU);
}