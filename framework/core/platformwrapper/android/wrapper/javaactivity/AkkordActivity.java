package org.akkord.lib;

import org.libsdl.app.SDLActivity;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.DisplayCutout;
import android.view.View;
import android.view.Window;
import android.view.WindowInsets;
import android.view.WindowInsetsController;
import com.getkeepsafe.relinker.ReLinker;
import org.akkord.lib.Utils;

public class AkkordActivity extends SDLActivity {

    // Отступы безопасной области относительно SDL surface, в пикселях
    private static native void nativeSafeAreaInsetsChanged(int left, int top, int right, int bottom);

    private final int[] mSurfaceLocation = new int[2];
    private int mSafeInsetLeft = -1, mSafeInsetTop = -1, mSafeInsetRight = -1, mSafeInsetBottom = -1;

    public void akkord_loadLibrary(String libraryName) throws UnsatisfiedLinkError, SecurityException, NullPointerException {

        if (libraryName == null) {
            throw new NullPointerException("No library name provided.");
        }

        try {
            ReLinker.loadLibrary(this, libraryName);
            Log.d(Utils.TAG, "Relinker library load success: " + libraryName);
        }
        catch (final Throwable e) {
            // Fall back
            try {
                Log.e(Utils.TAG, "Relinker load error :" + libraryName + ". " + e.getMessage());
                System.loadLibrary(libraryName);
            }
            catch (final UnsatisfiedLinkError ule) {
                throw ule;
            }
            catch (final SecurityException se) {
                throw se;
            }
        }
    }

    // Edge-to-edge обязателен для targetSdk 36 на Android 16+; включаем его на всех версиях, чтобы раскладка была одинаковой.
    // Приложение, не учитывающее safe area, может переопределить и вернуть false (на Android 15+ система всё равно включит edge-to-edge).
    protected boolean isEdgeToEdgeEnabled() {
        return true;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        mEdgeToEdge = isEdgeToEdgeEnabled();
        // полноэкранный режим, выбранный в прошлый раз (BWrapper::SetWindowFullscreen): применяем до первого кадра,
        // чтобы при старте панели не мелькали и раскладка игры не прыгала
        mFullscreenModeActive = org.akkord.lib.Utils.getFullscreenPreference(this);
        super.onCreate(savedInstanceState);
        if (mEdgeToEdge) {
            enableEdgeToEdge();
        }
        if (mFullscreenModeActive) {
            hideSystemBars();
        }
        org.akkord.lib.Utils.Init(this);
        org.akkord.lib.Utils.checkUpdate();
        setupSafeAreaTracking();
    }

    // Прозрачность панелей задаётся темой приложения (statusBarColor / navigationBarColor).
    // SDL повторяет эту настройку в setWindowStyle, здесь — чтобы первый кадр уже был edge-to-edge.
    private void enableEdgeToEdge() {
        final Window window = getWindow();
        if (Build.VERSION.SDK_INT >= 30 /* Android 11 (R) */) {
            window.setDecorFitsSystemWindows(false);
        } else {
            // сохраняем флаги темы (windowLightStatusBar и т.п.)
            final View decorView = window.getDecorView();
            decorView.setSystemUiVisibility(decorView.getSystemUiVisibility()
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        }
    }

    private void setupSafeAreaTracking() {
        if (mBrokenLibraries || mSurface == null) {
            return;
        }

        final View decorView = getWindow().getDecorView();
        decorView.setOnApplyWindowInsetsListener((v, insets) -> {
            // layout с новыми insets ещё не выполнен, поэтому пересчитываем после него
            v.post(this::updateSafeAreaInsets);
            return v.onApplyWindowInsets(insets);
        });
        mSurface.addOnLayoutChangeListener((v, l, t, r, b, ol, ot, or, ob) -> updateSafeAreaInsets());
        decorView.requestApplyInsets();
    }

    // Считаем геометрически: на сколько системные insets окна перекрывают surface.
    // Так результат верен и в edge-to-edge (targetSdk 36+), и когда система сама уже отодвинула контент от панелей.
    private void updateSafeAreaInsets() {
        final View surface = mSurface;
        if (surface == null || mBrokenLibraries) {
            return;
        }

        final WindowInsets insets = surface.getRootWindowInsets();
        if (insets == null || surface.getWidth() <= 0 || surface.getHeight() <= 0) {
            return;
        }

        int wndLeft, wndTop, wndRight, wndBottom;
        if (Build.VERSION.SDK_INT >= 30 /* Android 11 (R) */) {
            final android.graphics.Insets bars = insets.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.displayCutout());
            wndLeft = bars.left;
            wndTop = bars.top;
            wndRight = bars.right;
            wndBottom = bars.bottom;
        } else if (mFullscreenModeActive) {
            // панели скрыты и по свайпу появляются поверх игры; а с SYSTEM_UI_FLAG_LAYOUT_STABLE система
            // всё равно отдаёт отступ под скрытую панель навигации — остаётся только вырез (ниже)
            wndLeft = wndTop = wndRight = wndBottom = 0;
        } else {
            wndLeft = insets.getSystemWindowInsetLeft();
            wndTop = insets.getSystemWindowInsetTop();
            wndRight = insets.getSystemWindowInsetRight();
            wndBottom = insets.getSystemWindowInsetBottom();
        }
        if (Build.VERSION.SDK_INT < 30) {
            if (Build.VERSION.SDK_INT >= 28 /* Android 9 (P) */) {
                final DisplayCutout cutout = insets.getDisplayCutout();
                if (cutout != null) {
                    wndLeft = Math.max(wndLeft, cutout.getSafeInsetLeft());
                    wndTop = Math.max(wndTop, cutout.getSafeInsetTop());
                    wndRight = Math.max(wndRight, cutout.getSafeInsetRight());
                    wndBottom = Math.max(wndBottom, cutout.getSafeInsetBottom());
                }
            }
        }

        final View root = surface.getRootView();
        surface.getLocationInWindow(mSurfaceLocation);
        final int surfaceLeft = mSurfaceLocation[0];
        final int surfaceTop = mSurfaceLocation[1];
        final int surfaceRight = surfaceLeft + surface.getWidth();
        final int surfaceBottom = surfaceTop + surface.getHeight();

        final int left = Math.max(0, wndLeft - surfaceLeft);
        final int top = Math.max(0, wndTop - surfaceTop);
        final int right = Math.max(0, surfaceRight - (root.getWidth() - wndRight));
        final int bottom = Math.max(0, surfaceBottom - (root.getHeight() - wndBottom));

        if (left == mSafeInsetLeft && top == mSafeInsetTop && right == mSafeInsetRight && bottom == mSafeInsetBottom) {
            return;
        }

        try {
            nativeSafeAreaInsetsChanged(left, top, right, bottom);
            mSafeInsetLeft = left;
            mSafeInsetTop = top;
            mSafeInsetRight = right;
            mSafeInsetBottom = bottom;
            Log.d(Utils.TAG, "Safe area insets: left=" + left + " top=" + top + " right=" + right + " bottom=" + bottom);
        }
        catch (final UnsatisfiedLinkError e) {
            Log.e(Utils.TAG, "nativeSafeAreaInsetsChanged error: " + e.getMessage());
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        // После рекламы, диалогов и сворачивания система может снова показать панели — прячем их, если игра в полноэкранном режиме.
        // До Android 11 это делает сам SDL (rehideSystemUi)
        if (hasFocus && mFullscreenModeActive && Build.VERSION.SDK_INT >= 30 /* Android 11 (R) */) {
            hideSystemBars();
        }
    }

    // то же, что делает SDL в setWindowStyle(true), но сразу, без очереди сообщений
    private void hideSystemBars() {
        final Window window = getWindow();
        if (Build.VERSION.SDK_INT >= 30 /* Android 11 (R) */) {
            window.setDecorFitsSystemWindows(false);
            final WindowInsetsController controller = window.getInsetsController();
            if (controller != null) {
                controller.setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
                controller.hide(WindowInsets.Type.systemBars());
            }
        } else {
            window.getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
        }
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            //"c++_shared",
            "SDL2",
            //"SDL2_png",
            //"SDL2_jpeg",
             "SDL2_image",
            // "SDL2_mixer",
            // "SDL2_net",
            // "SDL2_ttf",
            "main"
        };
    }

    @Override
    public void loadLibraries() {
       for (String lib : getLibraries()) {
          //System.loadLibrary(lib);
          akkord_loadLibrary(lib);
       }
    }

    @Override
    protected void onStop() {
        super.onStop();
        org.akkord.lib.Utils.onActivityStop();
    }
}
