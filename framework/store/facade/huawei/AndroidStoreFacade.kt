package org.akkord.lib

import android.app.Activity
import android.content.ActivityNotFoundException
import android.content.Context
import android.content.Intent
import android.util.Log
import androidx.activity.ComponentActivity
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.core.content.pm.PackageInfoCompat
import com.huawei.hms.jos.AppUpdateClient
import com.huawei.hms.jos.JosApps
import com.huawei.updatesdk.service.appmgr.bean.ApkUpgradeInfo
import com.huawei.updatesdk.service.otaupdate.CheckUpdateCallBack
import com.huawei.updatesdk.service.otaupdate.UpdateKey

/**
 * Huawei AppGallery.
 *
 * Оценка - окно отзыва AppGallery (In-App Comments) по intent, без SDK. Показывать ли его, решает AppGallery:
 * приложение из AppGallery вне материкового Китая, AppGallery 11.3.2.302+, вход в HUAWEI ID, не меньше 10 запусков
 * за 3 месяца, оценка и отзыв к этой версии ещё не оставлены. Иначе окна нет, сразу приходит код результата.
 * После оценки или отзыва (102 / 103) до следующего versionCode больше не просим.
 *
 * Обновление - HMS JOS AppUpdateClient (com.huawei.hms:appservice): диалог AppGallery, если вышла новая версия.
 * App id HMS читает из assets/agconnect-services.json приложения.
 */
class AndroidStoreFacade {
    companion object {
        private const val REVIEW_ACTION = "com.huawei.appmarket.intent.action.guidecomment"
        private const val APPGALLERY_PACKAGE = "com.huawei.appmarket"
        private const val REVIEW_REGISTRY_KEY = "akkord_huawei_app_review"
        private const val REVIEW_REQUEST_CODE = 100501

        // Коды результата окна отзыва AppGallery
        private const val REVIEW_RATED = 102
        private const val REVIEW_COMMENTED = 103

        private const val PREFS = "akkord_huawei_store"
        private const val PREF_REVIEWED_VERSION = "reviewed_version_code"

        @JvmStatic
        fun launchAppReviewIfPossible() {
            val activity = getContext()
            val versionCode = currentVersionCode(activity)
            if (versionCode != null && prefs(activity).getLong(PREF_REVIEWED_VERSION, -1L) == versionCode) {
                Log.d(getTag(), "Huawei: review already left for version $versionCode")
                return
            }
            Utils.logFirebaseEvent("app_review_requested", "store", "huawei")
            val registry = (activity as? ComponentActivity)?.activityResultRegistry
            if (registry == null) {
                // SDL-активити (AkkordActivity) - не ComponentActivity: окно открываем, но код ответа не получаем
                try {
                    @Suppress("DEPRECATION")
                    activity.startActivityForResult(Intent(REVIEW_ACTION).setPackage(APPGALLERY_PACKAGE), REVIEW_REQUEST_CODE)
                } catch (e: ActivityNotFoundException) {
                    Log.w(getTag(), "Huawei: AppGallery review is not available: ${e.message}")
                    Utils.logFirebaseEvent("app_review_result", "code", "no_appgallery")
                } catch (e: Exception) {
                    Utils.handleException(e, "Huawei launchAppReviewIfPossible")
                }
                return
            }

            var launcher: ActivityResultLauncher<Intent>? = null
            launcher = registry.register(REVIEW_REGISTRY_KEY, ActivityResultContracts.StartActivityForResult()) { result ->
                launcher?.unregister()
                onReviewResult(activity, result.resultCode, versionCode)
            }
            try {
                launcher.launch(Intent(REVIEW_ACTION).setPackage(APPGALLERY_PACKAGE))
            } catch (e: ActivityNotFoundException) {
                // AppGallery нет или он старше 11.3.2.302
                launcher.unregister()
                Log.w(getTag(), "Huawei: AppGallery review is not available: ${e.message}")
                Utils.logFirebaseEvent("app_review_result", "code", "no_appgallery")
            } catch (e: Exception) {
                launcher.unregister()
                Utils.handleException(e, "Huawei launchAppReviewIfPossible")
            }
        }

        @JvmStatic
        fun checkAppUpdate() {
            val activity = getContext()
            val client: AppUpdateClient = JosApps.getAppUpdateClient(activity)
            client.checkAppUpdate(activity, object : CheckUpdateCallBack {
                override fun onUpdateInfo(intent: Intent?) {
                    try {
                        @Suppress("DEPRECATION")
                        val info = intent?.getSerializableExtra(UpdateKey.INFO)
                        if (info is ApkUpgradeInfo) {
                            Log.d(getTag(), "Huawei: update to ${info.version_} (${info.versionCode_})")
                            client.showUpdateDialog(activity, info, false)
                        } else {
                            Log.d(getTag(), "Huawei: no update, status ${intent?.getIntExtra(UpdateKey.STATUS, -1)}")
                        }
                    } catch (e: Exception) {
                        Utils.handleException(e, "Huawei checkAppUpdate: onUpdateInfo")
                    } finally {
                        client.releaseCallBack()
                    }
                }

                override fun onMarketInstallInfo(intent: Intent?) {
                }

                override fun onMarketStoreError(responseCode: Int) {
                    Log.e(getTag(), "Huawei: checkAppUpdate market error $responseCode")
                    client.releaseCallBack()
                }

                override fun onUpdateStoreError(responseCode: Int) {
                    Log.e(getTag(), "Huawei: checkAppUpdate update error $responseCode")
                    client.releaseCallBack()
                }
            })
        }

        @JvmStatic
        fun onActivityStop() {
        }

        private fun onReviewResult(context: Context, resultCode: Int, versionCode: Long?) {
            Log.d(getTag(), "Huawei: review result $resultCode")
            Utils.logFirebaseEvent("app_review_result", "code", resultCode.toString())
            if (versionCode != null && (resultCode == REVIEW_RATED || resultCode == REVIEW_COMMENTED)) {
                prefs(context).edit().putLong(PREF_REVIEWED_VERSION, versionCode).apply()
            }
        }

        private fun currentVersionCode(context: Context): Long? = try {
            PackageInfoCompat.getLongVersionCode(context.packageManager.getPackageInfo(context.packageName, 0))
        } catch (e: Exception) {
            Log.e(getTag(), "Huawei: versionCode: ${e.message}")
            null
        }

        private fun prefs(context: Context) = context.getSharedPreferences(PREFS, Context.MODE_PRIVATE)
        private fun getContext(): Activity = org.akkord.lib.Utils.GetContext()
        private fun getTag(): String = org.akkord.lib.Utils.TAG
    }
}
