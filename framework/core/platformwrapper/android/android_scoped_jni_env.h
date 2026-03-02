#ifndef __AKK0RD_SDK_ANDROID_SCOPED_JNI_ENV_H__
#define __AKK0RD_SDK_ANDROID_SCOPED_JNI_ENV_H__

#include <jni.h>

struct ScopedJNIEnv {
    JavaVM* vm;
    JNIEnv* env;
    bool needsDetach;

    explicit ScopedJNIEnv(JavaVM* javaVM) : vm(javaVM), env(nullptr), needsDetach(false) {
        if (!vm) return;
        jint status = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
        if (status == JNI_EDETACHED) {
            if (vm->AttachCurrentThread(&env, nullptr) == JNI_OK) {
                needsDetach = true;
            } else {
                env = nullptr;
            }
        } else if (status != JNI_OK) {
            env = nullptr;
        }
    }

    ~ScopedJNIEnv() {
        if (needsDetach && vm) {
            vm->DetachCurrentThread();
        }
    }

    ScopedJNIEnv(const ScopedJNIEnv&) = delete;
    ScopedJNIEnv& operator=(const ScopedJNIEnv&) = delete;

    ScopedJNIEnv(ScopedJNIEnv&& other) noexcept
        : vm(other.vm), env(other.env), needsDetach(other.needsDetach) {
        other.needsDetach = false;
        other.env = nullptr;
    }
    ScopedJNIEnv& operator=(ScopedJNIEnv&&) = delete;

    JNIEnv* get() const { return env; }
};

#endif
