#ifndef __AKK0RD_SDK_ANDROID_JAVA_UTF8_STRING_H__
#define __AKK0RD_SDK_ANDROID_JAVA_UTF8_STRING_H__

#include <jni.h>

class AndroidJavaUTF8String {
public:
    AndroidJavaUTF8String(JNIEnv* jniEnvironment, const char* cString) : javaString(jniEnvironment->NewStringUTF(cString)), jniEnv(jniEnvironment) {

    }

    jstring& get() {
        return javaString;
    }

    ~AndroidJavaUTF8String() {
        if (javaString) {
            jniEnv->DeleteLocalRef(javaString);
        }
    }
private:
    jstring javaString;
    JNIEnv* jniEnv;
};

#endif