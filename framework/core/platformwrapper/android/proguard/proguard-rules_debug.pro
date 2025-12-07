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