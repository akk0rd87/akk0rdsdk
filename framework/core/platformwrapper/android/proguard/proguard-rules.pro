-assumenosideeffects class android.util.Log {
public static boolean isLoggable(...);
public static int d(...);
public static int v(...);
public static int i(...);
public static int w(...);
public static int e(...);
public static int wtf(...);
    }

-dontwarn **
-target 1.7
-dontusemixedcaseclassnames
-dontskipnonpubliclibraryclasses
-dontpreverify
-verbose
-optimizations !code/simplification/arithmetic,!code/allocation/variable
-keep class **
-keepclassmembers class *{*;}
-keepattributes *Annotation*
-keepattributes SourceFile,LineNumberTable        # Keep file names and line numbers.
-keep public class * extends java.lang.Exception  # Optional: Keep custom exceptions.