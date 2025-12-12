#-keepclasseswithmembers *SDL*
#-keepclasseswithmembers *SDL*

-keep public class * extends org.akkord.lib.AkkordActivity {
  public *;
}

-keep public class * extends org.akkord.lib.AkkordComposeActivity {
  public *;
}

-keep public class org.akkord.lib.** {
  public *;
}

-keep public class org.libsdl.app.** {
  public *;
}

# Keep all native methods (JNI)
-keepclasseswithmembernames,includedescriptorclasses class * {
    native <methods>;
}

-assumenosideeffects class android.util.Log {
public static boolean isLoggable(...);
public static int d(...);
public static int v(...);
public static int i(...);
public static int w(...);
public static int e(...);
public static int wtf(...);
}