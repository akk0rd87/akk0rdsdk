# Samsung Galaxy Store: покупки на Samsung IAP SDK (framework/billing/samsung, com.samsung.developer:iap).
# Подключается только флейвору Samsung вдобавок к правилам build type.
# Своих consumer-правил у AAR нет, а IAP разбирает ответы Galaxy Store в свои vo-классы и связывается с ним по AIDL:
# держим библиотеку целиком, как советует Samsung.
-keep class com.samsung.android.sdk.iap.lib.** { *; }
-keep interface com.samsung.android.iap.** { *; }
-keep class com.samsung.android.iap.** { *; }
