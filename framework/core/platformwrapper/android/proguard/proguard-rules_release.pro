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

# Room Database - keep generated implementations
-keep class * extends androidx.room.RoomDatabase
-keep @androidx.room.Database class *
-keep @androidx.room.Dao class *
-keep @androidx.room.Entity class *
-keepclassmembers class * extends androidx.room.RoomDatabase {
    abstract *;
}
-keep class * implements androidx.room.RoomDatabase$Callback { *; }
-keep class **_Impl { *; }

# WorkManager
-keep class androidx.work.** { *; }
-keepclassmembers class * extends androidx.work.Worker {
    public <init>(android.content.Context, androidx.work.WorkerParameters);
}
-keepclassmembers class * extends androidx.work.ListenableWorker {
    public <init>(android.content.Context, androidx.work.WorkerParameters);
}

# AndroidX Startup
-keep class androidx.startup.** { *; }
-keep class * extends androidx.startup.Initializer { *; }

-assumenosideeffects class android.util.Log {
public static boolean isLoggable(...);
public static int d(...);
public static int v(...);
public static int i(...);
public static int w(...);
public static int e(...);
public static int wtf(...);
}