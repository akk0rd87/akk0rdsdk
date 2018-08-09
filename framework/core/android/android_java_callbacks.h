#pragma once
#ifndef __AKK0RD_ANDROID_JAVACALLBACKS_H__
#define __AKK0RD_ANDROID_JAVACALLBACKS_H__

#include "basewrapper.h"
#include "customevents.h"
#include "jni.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "core/core_defines.h"

extern "C" {
JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_AkkordCallback(JNIEnv*, jclass, jstring);
//JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_AdCallback(JNIEnv*, jclass, jint, jint, jint);
}

JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_AkkordCallback(JNIEnv* mEnv, jclass cls, jstring data)
{
    //const char* str = (*mEnv)->GetStringUTFChars(mEnv, data, NULL);
    const char* str = mEnv->GetStringUTFChars(data, 0);
    TestEvent* Event = new TestEvent();
    Event->Name = std::string(str);
    CustomEvents::GenerateSDKEvent(CustomEvents::SDKEventType::Test, (BaseCustomEvent*)Event);
    logDebug(str);
    mEnv->ReleaseStringUTFChars(data, str);
};

JNIEXPORT void JNICALL Java_org_akkord_lib_Utils_MessageBoxCallback(JNIEnv* mEnv, jclass cls, int Code, int Result)
{
	CustomEvents::MessageBoxCallback(Code, Result);	
};

/*
JNIEXPORT void JNICALL Java_org_akkord_lib_AdMobAdapter_AdCallback(JNIEnv*, jclass, jint AdType, jint EventType, jint Code)
{
    AdEvent* Event = new AdEvent();
    Event->AdType     = (int)AdType;
    Event->EventType  = (int)EventType;
    Event->Code       = (int)Code;
    logDebug("Add callback event %d %d %d", Event->AdType, Event->EventType, Event->Code);
    CustomEvents::GenerateSDKEvent(CustomEvents::SDKEventType::AdCallback, (BaseCustomEvent*)Event);
}
*/

#endif // __AKK0RD_ANDROID_BASEWRAPPER_H__