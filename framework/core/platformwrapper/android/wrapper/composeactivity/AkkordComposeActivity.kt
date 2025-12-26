package org.akkord.lib;

import android.os.Bundle
import androidx.activity.ComponentActivity
import com.getkeepsafe.relinker.ReLinker

open class AkkordComposeActivity : ComponentActivity() {
    private var isFirstResume = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        org.akkord.lib.Utils.Init(this)
        loadNativeLibs()
    }

    override fun onStop() {
        super.onStop()
        org.akkord.lib.Utils.onActivityStop()
    }

    override fun onResume() {
        super.onResume()
        if(isFirstResume) {
            isFirstResume = false
            org.akkord.lib.Utils.checkUpdate()
        }
    }

    private fun loadNativeLib(libName : String) {
        try {
            ReLinker.loadLibrary(this, libName)
        } catch (e: Throwable) {
            System.loadLibrary(libName)
        } finally {
            // optional finally block
        }
    }

    protected fun loadNativeLibs() {
        loadNativeLib("main")
    }
}