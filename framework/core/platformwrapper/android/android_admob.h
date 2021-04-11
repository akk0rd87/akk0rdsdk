#pragma once
#ifndef __AKK0RD_SDK_ANDROIDADMOBWRAPPER_H__
#define __AKK0RD_SDK_ANDROIDADMOBWRAPPER_H__

#include "admob.h"
#include "basewrapper.h"
#include <jni.h>
#include "core/core_defines.h"

static jclass AdMobClass = nullptr;

static jmethodID midInterstitialSetUnitId  = nullptr;
static jmethodID midInterstitialLoad       = nullptr;
static jmethodID midInterstitialShow       = nullptr;
static jmethodID midRewardedVideoSetUnitId = nullptr;
static jmethodID midRewardedVideoLoad      = nullptr;
static jmethodID midRewardedVideoShow      = nullptr;

class AdMobAndroid
{

private:

public:
    static bool Init(int Formats);

    static bool InterstitialSetUnitId(const char* UnitId);
    static bool InterstitialLoad();
    static bool InterstitialShow();

    static bool RewardedVideoSetUnitId(const char* UnitId);
    static bool RewardedVideoLoad();
    static bool RewardedVideoShow();
};


bool AdMobAndroid::Init(int Formats)
{
    bool Result = true;
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass localClass = env->FindClass("org/akkord/lib/AdMobAdapter");

    if(!localClass)
    {
        logError("Could not find AdMobAdapter class");
        return false;
    }

    AdMobClass = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
    env->DeleteLocalRef(localClass);

    jmethodID AdmobInit = env->GetStaticMethodID(AdMobClass, "Initialize", "()V");
    if(!AdmobInit)
    {
        logError("Initialize Java method not Found");
        return false;
    }

    env->CallStaticVoidMethod(AdMobClass, AdmobInit);

    if(Formats & AdMob::Format::Interstitial)
    {
        // InterstitialInit method
        AdmobInit = env->GetStaticMethodID(AdMobClass, "InterstitialInit", "()V");
        if(!AdmobInit)
        {
            logError("InterstitialInit Java method not Found");
            return false;
        }
        env->CallStaticVoidMethod(AdMobClass, AdmobInit);
    }

    if(Formats & AdMob::Format::RewardedVideo)
    {
        // RewardedVideoInit method
        AdmobInit = env->GetStaticMethodID(AdMobClass, "RewardedVideoInit", "()V");
        if(!AdmobInit)
        {
            logError("RewardedVideoInit Java method not Found");
            return false;
        }
        env->CallStaticVoidMethod(AdMobClass, AdmobInit);
    }

    midInterstitialSetUnitId   = env->GetStaticMethodID(AdMobClass, "InterstitialSetUnitId", "(Ljava/lang/String;)V");
    midInterstitialLoad        = env->GetStaticMethodID(AdMobClass, "InterstitialLoad", "()V");
    midInterstitialShow        = env->GetStaticMethodID(AdMobClass, "InterstitialShow", "()I");
    midRewardedVideoSetUnitId  = env->GetStaticMethodID(AdMobClass, "RewardedVideoSetUnitId", "(Ljava/lang/String;)V");
    midRewardedVideoLoad       = env->GetStaticMethodID(AdMobClass, "RewardedVideoLoad", "()V");
    midRewardedVideoShow       = env->GetStaticMethodID(AdMobClass, "RewardedVideoShow", "()I");

    if(nullptr == midInterstitialSetUnitId ) { Result = false; logError("midInterstitialSetUnitId  Java method not found"); }
    if(nullptr == midInterstitialLoad      ) { Result = false; logError("midInterstitialLoad       Java method not found"); }
    if(nullptr == midInterstitialShow      ) { Result = false; logError("midInterstitialShow       Java method not found"); }
    if(nullptr == midRewardedVideoSetUnitId) { Result = false; logError("midRewardedVideoSetUnitId Java method not found"); }
    if(nullptr == midRewardedVideoLoad     ) { Result = false; logError("midRewardedVideoLoad      Java method not found"); }
    if(nullptr == midRewardedVideoShow     ) { Result = false; logError("midRewardedVideoShow      Java method not found");    }

    return Result;
};

bool AdMobAndroid::InterstitialSetUnitId(const char* UnitId)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midInterstitialSetUnitId)
    {
        logError("InterstitialSetUnitId Java method not Found");
        return false;
    }
    jstring url_jstring = (jstring)env->NewStringUTF(UnitId);
    env->CallStaticVoidMethod(AdMobClass, midInterstitialSetUnitId, url_jstring);
    env->DeleteLocalRef(url_jstring);
    return true;
};

bool AdMobAndroid::InterstitialLoad()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midInterstitialLoad)
    {
        logError("InterstitialLoad Java method not Found");
        return false;
    }
    env->CallStaticVoidMethod(AdMobClass, midInterstitialLoad);

    return true;
};

bool AdMobAndroid::InterstitialShow()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midInterstitialShow)
    {
        logError("InterstitialShow Java method not Found");
        return false;
    }
    jint value = env->CallStaticIntMethod(AdMobClass, midInterstitialShow);
    int retval = (int)value;
    //logDebug("retval %d", retval);

    if(retval) return true;
    return false;
};

bool AdMobAndroid::RewardedVideoSetUnitId(const char* UnitId)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midRewardedVideoSetUnitId)
    {
        logError("RewardedVideoSetUnitId Java method not Found");
        return false;
    }
    jstring url_jstring = (jstring)env->NewStringUTF(UnitId);
    env->CallStaticVoidMethod(AdMobClass, midRewardedVideoSetUnitId, url_jstring);
    env->DeleteLocalRef(url_jstring);
    return true;
};

bool AdMobAndroid::RewardedVideoLoad()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midRewardedVideoLoad)
    {
        logError("RewardedVideoLoad Java method not Found");
        return false;
    }
    env->CallStaticVoidMethod(AdMobClass, midRewardedVideoLoad);

    return true;
};

bool AdMobAndroid::RewardedVideoShow()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    if(!midRewardedVideoShow)
    {
        logError("RewardedVideoShow Java method not Found");
        return false;
    }
    jint value = env->CallStaticIntMethod(AdMobClass, midRewardedVideoShow);
    int retval = (int)value;

    if(retval) return true;
    return false;
};

#endif // __AKK0RD_SDK_ANDROIDADMOBWRAPPER_H__