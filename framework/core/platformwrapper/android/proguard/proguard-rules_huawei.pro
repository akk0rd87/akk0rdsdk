# Huawei HMS (store/facade/huawei: проверка обновления через com.huawei.hms:appservice).
# Подключается только флейвору Huawei AppGallery вдобавок к правилам build type.

# Системные классы прошивки Huawei / HarmonyOS: есть только на устройстве, SDK обновления проверяет их сам
-dontwarn com.huawei.android.app.PackageManagerEx
-dontwarn com.huawei.android.content.pm.ApplicationInfoEx
-dontwarn com.huawei.ohos.localability.**
-dontwarn com.huawei.system.BuildEx

# Правила HMS лежат в consumer-правилах AAR. Широкое -keep class com.huawei.hms.** не ставим: оно держит неиспользуемые
# части (donation, вход в HUAWEI ID), которые ссылаются на Kit-ы не из нашей сборки, и R8 падает на missing classes.
# SDK обновления разбирает ответы магазина рефлексией - держим его целиком, как советует Huawei.
-keepattributes Signature
-keep class com.huawei.updatesdk.**{*;}
