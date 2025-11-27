#-keepclasseswithmembers *SDL*
#-keepclasseswithmembers *SDL*

-keep public class * extends org.akkord.lib.AkkordActivity {
  public *;
}

-keep public class org.akkord.lib.** {
  public *;
}

-keep public class org.libsdl.app.** {
  public *;
}

# Временный workaround для падения "Downloader constructor missing"
-keep class yads.** {
    *;
}