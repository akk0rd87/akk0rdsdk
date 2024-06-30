package org.akkord.lib

import android.util.Log
import com.google.android.gms.tasks.OnSuccessListener
import com.google.android.gms.tasks.Task
import com.google.android.play.core.appupdate.AppUpdateInfo
import com.google.android.play.core.appupdate.AppUpdateManager
import com.google.android.play.core.appupdate.AppUpdateManagerFactory
import com.google.android.play.core.install.InstallState
import com.google.android.play.core.install.InstallStateUpdatedListener
import com.google.android.play.core.install.model.AppUpdateType
import com.google.android.play.core.install.model.InstallStatus
import com.google.android.play.core.install.model.UpdateAvailability
import com.google.android.play.core.review.ReviewInfo
import com.google.android.play.core.review.ReviewManagerFactory
import java.util.Objects

class AndroidStoreFacade {
    companion object {
        private val mAppUpdateManager : AppUpdateManager by lazy { AppUpdateManagerFactory.create(org.akkord.lib.Utils.GetContext()) }

        val installStateUpdatedListener = InstallStateUpdatedListener { state: InstallState ->
            if (state.installStatus() == InstallStatus.DOWNLOADED) {
                //CHECK THIS if AppUpdateType.FLEXIBLE, otherwise you can skip
                popupSnackbarForCompleteUpdate()
            } else if (state.installStatus() == InstallStatus.INSTALLED) {
                unRegisterUpdateListener()
            } else {
                Log.i(org.akkord.lib.Utils.TAG, "InstallStateUpdatedListener: state: ${state.installStatus()}")
            }
        }

        @JvmStatic
        fun launchAppReviewIfPossible() {
            val reviewManager = ReviewManagerFactory.create(org.akkord.lib.Utils.GetContext())
            val requestFlow = reviewManager.requestReviewFlow()

            requestFlow.addOnCompleteListener { task: Task<ReviewInfo?> ->
                if (task.isSuccessful) {
                    try {
                        // We can get the ReviewInfo object
                        Log.d(org.akkord.lib.Utils.TAG, "requestReviewFlow isSuccessful")
                        task.result?.let { reviewInfo ->
                            Log.d(org.akkord.lib.Utils.TAG, "launchReviewFlow")
                            reviewManager.launchReviewFlow(org.akkord.lib.Utils.GetContext(), reviewInfo)
                        }

                    } catch (e: Exception) {
                        Log.d(org.akkord.lib.Utils.TAG, e.message?:"unknown exception")
                    }
                } else {
                    Log.d(org.akkord.lib.Utils.TAG, "requestReviewFlow is NOT Successful")
                }
            }
        }

        @JvmStatic
        fun checkAppUpdate() {
            mAppUpdateManager.registerListener(installStateUpdatedListener)
            mAppUpdateManager.appUpdateInfo.
                addOnSuccessListener(OnSuccessListener<AppUpdateInfo> { appUpdateInfo: AppUpdateInfo ->
                    if (appUpdateInfo.updateAvailability() == UpdateAvailability.UPDATE_AVAILABLE) {
                        //&& appUpdateInfo.isUpdateTypeAllowed(AppUpdateType.FLEXIBLE /*AppUpdateType.IMMEDIATE*/))
                        try {
                            mAppUpdateManager.startUpdateFlowForResult(
                                appUpdateInfo,
                                AppUpdateType.FLEXIBLE,  /*AppUpdateType.IMMEDIATE*/
                                org.akkord.lib.Utils.GetContext(),  /*RC_APP_UPDATE*/
                                100500
                            )
                        } catch (e: java.lang.Exception) {
                            Log.e(
                                org.akkord.lib.Utils.TAG,
                                Objects.requireNonNull<String?>(e.message)
                            )
                        }
                    } else if (appUpdateInfo.installStatus() == InstallStatus.DOWNLOADED) {
                        //CHECK THIS if AppUpdateType.FLEXIBLE, otherwise you can skip
                        popupSnackbarForCompleteUpdate()
                    } else {
                        Log.e(org.akkord.lib.Utils.TAG, "checkForAppUpdateAvailability: something else")
                    }
                })
        }

        @JvmStatic
        fun onActivityStop() {
            unRegisterUpdateListener()
        }

        private fun popupSnackbarForCompleteUpdate() {
            try {
                mAppUpdateManager.completeUpdate()
                Log.i(org.akkord.lib.Utils.TAG, "popupSnackbarForCompleteUpdate")
            }
            catch(e: Exception) {
                Log.d(org.akkord.lib.Utils.TAG, e.message?:"unknown exception")
            }
        }

        private fun unRegisterUpdateListener() {
            try {
                mAppUpdateManager.unregisterListener(installStateUpdatedListener)
            }
            catch(e: Exception) {
                Log.d(org.akkord.lib.Utils.TAG, e.message?:"unknown exception")
            }
        }
    }
}