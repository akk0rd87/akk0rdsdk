package org.akkord.lib;

import android.os.Bundle
import android.view.WindowManager
import androidx.activity.ComponentActivity

open class AkkordComposeActivity : ComponentActivity() {
    @Volatile
    private var isFirstResume = true

    override fun onCreate(savedInstanceState: Bundle?) {
        // чтобы приложение не вылетало с ошибкой при пересоздании фрагментов после устройства процесса системой
        savedInstanceState?.remove("android:fragments")
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        org.akkord.lib.Utils.Init(this)
        loadNativeLibs()
    }

    override fun onStop() {
        super.onStop()
        org.akkord.lib.Utils.onActivityStop()
    }

    /** Проверять ли обновление в сторе при первом onResume; наследник выключает, например, в UI-тестах. */
    protected open val isUpdateCheckEnabled: Boolean
        get() = true

    override fun onResume() {
        super.onResume()
        if(isFirstResume) {
            isFirstResume = false
            if (isUpdateCheckEnabled) {
                org.akkord.lib.Utils.checkUpdate()
            }
        }
    }

    protected fun loadNativeLibs() {
        System.loadLibrary("main")
    }
}