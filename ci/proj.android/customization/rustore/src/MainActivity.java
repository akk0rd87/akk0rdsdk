package org.popapp.citest;

import android.content.Intent;
import android.os.Bundle;
import org.akkord.lib.BillingManager;
import org.popapp.citest.CiTestActivity;

public class MainActivity extends CiTestActivity {
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (savedInstanceState == null) { // for RuStore billing. Does nothing for others.
            BillingManager.onNewIntent(getIntent());
        }
    }

    @Override
    protected void onNewIntent(Intent intent) { // for RuStore billing. Does nothing for others.
        super.onNewIntent(intent);
        BillingManager.onNewIntent(intent);
    }

}