package org.akkord.lib

import android.util.Log
import ru.rustore.sdk.appupdate.listener.InstallStateUpdateListener
import ru.rustore.sdk.appupdate.manager.RuStoreAppUpdateManager
import ru.rustore.sdk.appupdate.manager.factory.RuStoreAppUpdateManagerFactory
import ru.rustore.sdk.appupdate.model.AppUpdateOptions
import ru.rustore.sdk.appupdate.model.InstallStatus
import ru.rustore.sdk.appupdate.model.UpdateAvailability
import ru.rustore.sdk.review.RuStoreReviewManagerFactory

class AndroidStoreFacade {
    companion object {
        private val appUpdateManager : RuStoreAppUpdateManager by lazy { RuStoreAppUpdateManagerFactory.create(org.akkord.lib.Utils.GetContext()) }

        private val installUpdateListener : InstallStateUpdateListener by lazy {
            InstallStateUpdateListener { state ->
                when (state.installStatus) {
                    InstallStatus.DOWNLOADED -> {
                        //updateManager.unregisterListener(this@InstallStateUpdateListener)
                        // The update is ready to be installed
                        completeDownloading()
                    }
                }
            }
        }

        @JvmStatic
        fun launchAppReviewIfPossible() {
            val manager = RuStoreReviewManagerFactory.create(org.akkord.lib.Utils.GetContext())
            manager.requestReviewFlow().addOnSuccessListener { reviewInfo ->
                Log.d(org.akkord.lib.Utils.TAG, "RuStore: launchReviewFlow")
                manager.launchReviewFlow(reviewInfo)
            }
        }

        @JvmStatic
        fun checkAppUpdate() {
            // do not check at the moment
            Log.d(org.akkord.lib.Utils.TAG, "RuStore: checkAppUpdate Not implemented yet")
            /*
            appUpdateManager.getAppUpdateInfo()
                .addOnSuccessListener { appUpdateInfo ->
                    if (appUpdateInfo.updateAvailability == UpdateAvailability.UPDATE_AVAILABLE) {
                        appUpdateManager.registerListener(installUpdateListener)
                        appUpdateManager.startUpdateFlow(appUpdateInfo, AppUpdateOptions.Builder().build())
                    }
                }
             */
        }

        @JvmStatic
        fun onActivityStop() {
            Log.d(org.akkord.lib.Utils.TAG, "RuStore: Not implemented yet")
        }

        private fun completeDownloading() {
            try {
                appUpdateManager.unregisterListener(installUpdateListener)
                appUpdateManager.completeUpdate()
            }
            catch(e : Exception) {
                Log.e(org.akkord.lib.Utils.TAG, e.message?:"Unknown exception")
            }
        }
    }
}