#include "android_utils_wrapper.h"

MessageBoxCallback* AndroidUtilsWrapper::callback{ nullptr };

void AndroidUtilsWrapper::onMessageboxCallback(int Code, int Result) {
    if (callback) {
        callback->onResult(Code, Result);
    }
}

extern "C" {
    JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_MessageBoxCallback(JNIEnv* mEnv, jclass cls, jint Code, jint Result)
    {
        AndroidUtilsWrapper::onMessageboxCallback(Code, Result);
    };
}