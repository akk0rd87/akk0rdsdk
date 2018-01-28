call cls
%ANDROID_NDK_HOME%\toolchains\arm-linux-androideabi-4.9\prebuilt\windows\arm-linux-androideabi\bin\nm -D main\build\outputs\native\debug\lib\armeabi\libSDL2.so | findstr /i java
%ANDROID_NDK_HOME%\toolchains\arm-linux-androideabi-4.9\prebuilt\windows\arm-linux-androideabi\bin\nm -D main\build\outputs\native\debug\lib\armeabi\libmain.so | findstr /i java
