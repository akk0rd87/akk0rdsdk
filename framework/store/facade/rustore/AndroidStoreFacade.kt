package org.akkord.lib

import android.app.Activity
import android.util.Log
import ru.rustore.sdk.appupdate.listener.InstallStateUpdateListener
import ru.rustore.sdk.appupdate.manager.RuStoreAppUpdateManager
import ru.rustore.sdk.appupdate.manager.factory.RuStoreAppUpdateManagerFactory
import ru.rustore.sdk.appupdate.model.AppUpdateOptions
import ru.rustore.sdk.appupdate.model.AppUpdateType
import ru.rustore.sdk.appupdate.model.InstallStatus
import ru.rustore.sdk.appupdate.model.UpdateAvailability
import ru.rustore.sdk.review.RuStoreReviewManagerFactory

class AndroidStoreFacade {
    companion object {
        private val appUpdateManager : RuStoreAppUpdateManager by lazy { RuStoreAppUpdateManagerFactory.create(getContext()) }

        private val installUpdateListener : InstallStateUpdateListener by lazy {
            InstallStateUpdateListener { state ->
                when (state.installStatus) {
                    InstallStatus.DOWNLOADED -> {
                        completeDownloading()
                    }

                    InstallStatus.DOWNLOADING -> {
                        Log.d(getTag(), "app update ${state.bytesDownloaded} of ${state.totalBytesToDownload} bytes")
                    }
                }
            }
        }

        @JvmStatic
        fun launchAppReviewIfPossible() {
            val manager = RuStoreReviewManagerFactory.create(getContext())
            manager.requestReviewFlow().addOnSuccessListener { reviewInfo ->
                Log.d(getTag(), "RuStore: launchReviewFlow")
                manager.launchReviewFlow(reviewInfo)
            }
        }

        @JvmStatic
        fun checkAppUpdate() {
            // do not check at the moment
            Log.d(getTag(), "RuStore: checkAppUpdate Not implemented yet")

            appUpdateManager.getAppUpdateInfo()
                .addOnSuccessListener { appUpdateInfo ->
                    if (appUpdateInfo.updateAvailability == UpdateAvailability.UPDATE_AVAILABLE) {
                        appUpdateManager.registerListener(installUpdateListener)
                        appUpdateManager.startUpdateFlow(appUpdateInfo, getAppUpdateOptions())
                    }
                }
        }

        @JvmStatic
        fun onActivityStop() {
            Log.d(getTag(), "RuStore: Not implemented yet")
        }

        private fun completeDownloading() {
            try {
                appUpdateManager.unregisterListener(installUpdateListener)
                appUpdateManager.completeUpdate(getAppUpdateOptions())
                    .addOnCompletionListener { result ->
                        Log.d(getTag(), "appUpdateManager: addOnCompletionListener: $result")
                    }
            }
            catch(e : Exception) {
                Log.e(getTag(), e.message?:"Unknown exception")
            }
        }

        private fun getAppUpdateOptions() = AppUpdateOptions.Builder().appUpdateType(AppUpdateType.FLEXIBLE).build()
        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}