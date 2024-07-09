package org.akkord.lib

import android.app.Activity
import android.util.Log
import com.google.android.gms.tasks.Task
import com.google.android.play.core.appupdate.AppUpdateInfo
import com.google.android.play.core.appupdate.AppUpdateManager
import com.google.android.play.core.appupdate.AppUpdateManagerFactory
import com.google.android.play.core.appupdate.AppUpdateOptions
import com.google.android.play.core.install.InstallState
import com.google.android.play.core.install.InstallStateUpdatedListener
import com.google.android.play.core.install.model.AppUpdateType
import com.google.android.play.core.install.model.InstallStatus
import com.google.android.play.core.install.model.UpdateAvailability
import com.google.android.play.core.review.ReviewInfo
import com.google.android.play.core.review.ReviewManagerFactory

class AndroidStoreFacade {
    companion object {
        private val mAppUpdateManager : AppUpdateManager by lazy { AppUpdateManagerFactory.create(getContext()) }

        private val installStateUpdatedListener = InstallStateUpdatedListener { state: InstallState ->
            when(state.installStatus()) {
                InstallStatus.DOWNLOADED -> popupSnackbarForCompleteUpdate()
                InstallStatus.INSTALLED -> unRegisterUpdateListener()
            }
        }

        @JvmStatic
        fun launchAppReviewIfPossible() {
            val reviewManager = ReviewManagerFactory.create(getContext())
            val requestFlow = reviewManager.requestReviewFlow()

            requestFlow.addOnCompleteListener { task: Task<ReviewInfo?> ->
                if (task.isSuccessful) {
                    try {
                        // We can get the ReviewInfo object
                        Log.d(getTag(), "requestReviewFlow isSuccessful")
                        task.result?.let { reviewInfo ->
                            Log.d(getTag(), "launchReviewFlow")
                            reviewManager.launchReviewFlow(getContext(), reviewInfo)
                        }

                    } catch (e: Exception) {
                        e.message?.let { Log.e(getTag(), it ) }
                    }
                } else {
                    Log.e(getTag(), "requestReviewFlow is NOT Successful")
                }
            }
        }

        @JvmStatic
        fun checkAppUpdate() {
            mAppUpdateManager.registerListener(installStateUpdatedListener)
            mAppUpdateManager.appUpdateInfo.
                addOnSuccessListener { appUpdateInfo: AppUpdateInfo ->
                    if (appUpdateInfo.updateAvailability() == UpdateAvailability.UPDATE_AVAILABLE) {
                        //&& appUpdateInfo.isUpdateTypeAllowed(AppUpdateType.FLEXIBLE /*AppUpdateType.IMMEDIATE*/))
                        try {
                            mAppUpdateManager.startUpdateFlowForResult(
                                appUpdateInfo,
                                getContext(),
                                AppUpdateOptions.newBuilder(AppUpdateType.FLEXIBLE).build(),
                                100500
                            )
                        } catch (e: Exception) {
                            e.message?.let { Log.e(getTag(), it ) }
                        }
                    } else if (appUpdateInfo.installStatus() == InstallStatus.DOWNLOADED) {
                        //CHECK THIS if AppUpdateType.FLEXIBLE, otherwise you can skip
                        popupSnackbarForCompleteUpdate()
                    } else {
                        Log.e(getTag(), "checkForAppUpdateAvailability: something else")
                    }
                }
        }

        @JvmStatic
        fun onActivityStop() {
            unRegisterUpdateListener()
        }

        private fun popupSnackbarForCompleteUpdate() {
            try {
                mAppUpdateManager.completeUpdate()
                Log.i(getTag(), "popupSnackbarForCompleteUpdate")
            }
            catch(e: Exception) {
                e.message?.let { Log.e(getTag(), it ) }
            }
        }

        private fun unRegisterUpdateListener() {
            try {
                mAppUpdateManager.unregisterListener(installStateUpdatedListener)
            }
            catch(e: Exception) {
                e.message?.let { Log.e(getTag(), it ) }
            }
        }

        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}