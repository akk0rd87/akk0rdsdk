### **SDL2 Homepage**

http://libsdl.org/
***

### **Repository setup**

set AKKORD_SDK_HOME env variable including full path to this repository on your computer.
***

### **Windows Setup**     

**Microsoft Visual Studio**

Read about recommended versions of MS Visual Studio on SDL installation guide https://wiki.libsdl.org/Installation

    

**DLL setup**

To run SDL apps on windows put dll-files in windows:

* SysWOW64 on 64-bit Windows OS

* system32 on 32-bit Windows OS

from 

1. libraries/SDL/win32/Release/

2. libraries/SDL_image/win32/Release/

http://headerphile.com/sdl2/sdl2-part-0-setting-up-visual-studio-for-sdl2/
***

### **Android Setup**

     

**Gradle**

Gradle install https://gradle.org/install/ Gradle 2.14.1

https://services.gradle.org/distributions/gradle-2.14.1-bin.zip

**Java**

http://www.oracle.com/technetwork/java/javase/downloads/index.html
  
**Android NDK - need to compile native C++ libraries**

https://developer.android.com/ndk/index.html

https://developer.android.com/ndk/guides/index.html#download-ndk

https://developer.android.com/ndk/downloads/index.html

https://developer.android.com/ndk/downloads/older_releases.html

Now I use android-ndk-r15c: https://dl.google.com/android/repository/android-ndk-r15c-windows-x86_64.zip

SET ANDROID_NDK_HOME=C:\PortablePrograms\android-ndk-r15c

    

**Android SDK - need to build android apk files**

Android Studio (Not required)

https://developer.android.com/studio/index.html

Just download command line tools. I don't use Android Stuidio UI interface.

https://developer.android.com/studio/index.html#downloads

SET ANDROID_HOME=C:\PortablePrograms\android-sdk

**Android KeyStore Directory**

ANDROID_KEYSTORE_HOME
     

**ADB utility**

Reference: https://developer.android.com/studio/command-line/adb.html

Set path to adb utility to PATH env variable. It located on %ANDROID_HOME%/platform-tools/

**Hints**

Ensure you have download android SDK platforms version specified in your build.gradle file.

Do not forget change android app name in these files:

    - package attribute in AndroidManifest.xml

    - proj.android\debug.bat

    - package name in proj.android\app\src\main\java\MainActivity.java
    
Do not init static global std::strings.

sdkmanager.bat extras;google;google_play_services
***

### **iOS**
    
    - Set AKKORD_SDK_HOME path in Xcode: Xcode->Preferences->Locations->Custom Paths
    
    - Открыть проект SDL CodeProj и собрать библиотеку (только первый раз, сборка делается только один раз, для других проектов повторять не надо).
    
    - Скопировать стандартный ios-template в рабочую попку из исходников SDL libraries\SDL\SDL\Xcode-iOS\Template
    
    - Открыть скопированный шаблон.
    
    - Удалить добавить ссылку на SDL.xcodeproj и SDL_image.xcodeproj.
    
    - Добавить ссылку на libraries/SDL/ios/libSDL2.a (значок должен быть домик с колоннами)
    
    - Добавить фреймворки CoreAudio (CoreAudio.framework), AudioToolBox.Frameworks, AVFoundation.Frameworks
    
    - Добавить фреймворки ImageIo Framework, MobileCoreServices framework, Foundation, UIKit, Metal.framework.    
    
    - Свои файлы исходного references, а потом еще раз эти же файлы из референсов
    
    - Target dependencies пусто, а библиотеки выбраны в Link binary with libraries
    
    - Выставить поддержку с++11: Build settings - Apple LLVM 8.0 - Language C++
    
    - Выставить макрос препроцессора Debug
    
    - Assets
    
    - Выставить BitCode = NO (пункт Build Options -> Enable Bitcode https://i.stack.imgur.com/VkI72.png)
    
    - Char type as unsigned
    
    - В build settings проекта в code signing identity везде поставить ios Developer, а в таргете выбрать automatic signing

    - Видимо после каждого добавления файла во фреймворк нужно пересоздавать group-ссылки на framework
    
    - Имя executable должен содержать валидные символы (без $)
    
    - Auth with app store error: https://forums.developer.apple.com/thread/76803
***

### **LibGDX SDF Generator**

Download last LibGDX build from https://libgdx.badlogicgames.com/ci/nightlies/ and set *LIBGDX_HOME* env variable for extracted path
***

https://stackoverflow.com/questions/8980284/android-mk-include-all-cpp-files

https://stackoverflow.com/questions/34732/how-do-i-list-the-symbols-in-a-so-file

https://gist.github.com/SuperV1234/5c5ad838fe5fe1bf54f9

https://habrahabr.ru/post/233265/

http://www.acodersjourney.com/2016/05/top-10-c-header-file-mistakes-and-how-to-fix-them/

http://theorangeduck.com/page/issues-sdl-ios-and-android

Font links:

https://github.com/grimfang4/SDL_FontCache

https://stackoverflow.com/questions/8847899/how-to-draw-text-using-only-opengl-methods

https://stackoverflow.com/questions/2115758/how-do-i-display-an-alert-dialog-on-android


AdMob

https://developers.google.com/admob/android/quick-start

https://developers.google.com/admob/ios/quick-start

https://github.com/wasabeef/awesome-android-ui
