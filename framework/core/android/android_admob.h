#pragma once
#ifndef __AKK0RD_ANDROIDADMOBWRAPPER_H__
#define __AKK0RD_ANDROIDADMOBWRAPPER_H__

#include "admob.h"
#include "basewrapper.h"
#include <jni.h>
#include "core/core_defines.h"

class AdMobAndroid
{

private:    
    static jclass FindAdmobClass(JNIEnv *env)
    {
        const char* ClassName = "org/akkord/lib/AdMobAdapter";
        jclass admobClass = env->FindClass(ClassName);
        if(!admobClass)
        {
            logError("Cannot find class %s", ClassName);
            return nullptr;
        }
        return admobClass;
    };
public:    
    static bool                       Init(const char* PublisherID, int Formats);
    
    static bool                       InterstitialSetUnitId(const char* UnitId);
    static bool                       InterstitialLoad();
    static bool                       InterstitialShow();
    
    static bool                       RewardedVideoSetUnitId(const char* UnitId);
    static bool                       RewardedVideoLoad();
    static bool                       RewardedVideoShow();
};


bool AdMobAndroid::Init(const char* PublisherID, int Formats)
{    
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAdmobClass(env);
    
    // Common Initialize method
    jmethodID AdmobInit = env->GetStaticMethodID(activity, "Initialize", "(Ljava/lang/String;)V");
    if(!AdmobInit)
    {
        logError("Initialize Java method not Found");        
        return false;        
    }
    
    jstring url_jstring = (jstring)env->NewStringUTF(PublisherID);
    env->CallStaticVoidMethod(activity, AdmobInit, url_jstring);
    env->DeleteLocalRef(url_jstring);     
    
    if(Formats & AdMob::Format::Interstitial)
    {
        // InterstitialInit method
        AdmobInit = env->GetStaticMethodID(activity, "InterstitialInit", "()V");
        if(!AdmobInit)
        {
            logError("InterstitialInit Java method not Found");        
            return false;        
        }    
        env->CallStaticVoidMethod(activity, AdmobInit);
    }
    
    if(Formats & AdMob::Format::RewardedVideo)
    {
        // RewardedVideoInit method
        AdmobInit = env->GetStaticMethodID(activity, "RewardedVideoInit", "()V");
        if(!AdmobInit)
        {
            logError("RewardedVideoInit Java method not Found");        
            return false;        
        }    
        env->CallStaticVoidMethod(activity, AdmobInit);
    }
    
    env->DeleteLocalRef(activity);
    
    return true;
};

bool AdMobAndroid::InterstitialSetUnitId(const char* UnitId)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAdmobClass(env);
    jmethodID InterstitialSetUnitId = env->GetStaticMethodID(activity, "InterstitialSetUnitId", "(Ljava/lang/String;)V");
    if(!InterstitialSetUnitId)
    {
        logError("InterstitialSetUnitId Java method not Found");        
        return false;        
    }
    jstring url_jstring = (jstring)env->NewStringUTF(UnitId);
    env->CallStaticVoidMethod(activity, InterstitialSetUnitId, url_jstring);
    env->DeleteLocalRef(url_jstring);   
    env->DeleteLocalRef(activity);    
    return true;
};

bool AdMobAndroid::InterstitialLoad()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAdmobClass(env);
    jmethodID AdmobLoad = env->GetStaticMethodID(activity, "InterstitialLoad", "()V");
    if(!AdmobLoad)
    {
        logError("InterstitialLoad Java method not Found");        
        return false;        
    }    
    env->CallStaticVoidMethod(activity, AdmobLoad);
    env->DeleteLocalRef(activity);
    
    return true;
};

bool AdMobAndroid::InterstitialShow()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAdmobClass(env);
    jmethodID InterstitialShow = env->GetStaticMethodID(activity, "InterstitialShow", "()I");
    if(!InterstitialShow)
    {
        logError("InterstitialShow Java method not Found");        
        return false;        
    }        
    jint value = env->CallStaticIntMethod(activity, InterstitialShow);        
    int retval = (int)value;
    logDebug("retval %d", retval);
    env->DeleteLocalRef(activity);
    
    if(retval) return true;
    return false;
};

bool AdMobAndroid::RewardedVideoSetUnitId(const char* UnitId)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAdmobClass(env);
    jmethodID RewardedVideoSetUnitId = env->GetStaticMethodID(activity, "RewardedVideoSetUnitId", "(Ljava/lang/String;)V");
    if(!RewardedVideoSetUnitId)
    {
        logError("RewardedVideoSetUnitId Java method not Found");        
        return false;        
    }
    jstring url_jstring = (jstring)env->NewStringUTF(UnitId);
    env->CallStaticVoidMethod(activity, RewardedVideoSetUnitId, url_jstring);
    env->DeleteLocalRef(url_jstring);   
    env->DeleteLocalRef(activity);    
    return true;
};

bool AdMobAndroid::RewardedVideoLoad()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAdmobClass(env);
    jmethodID AdmobLoad = env->GetStaticMethodID(activity, "RewardedVideoLoad", "()V");
    if(!AdmobLoad)
    {
        logError("RewardedVideoLoad Java method not Found");        
        return false;        
    }    
    env->CallStaticVoidMethod(activity, AdmobLoad);
    env->DeleteLocalRef(activity);
    
    return true;
};

bool AdMobAndroid::RewardedVideoShow()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindAdmobClass(env);
    jmethodID RewardedVideoShow = env->GetStaticMethodID(activity, "RewardedVideoShow", "()I");
    if(!RewardedVideoShow)
    {
        logError("RewardedVideoShow Java method not Found");        
        return false;        
    }        
    jint value = env->CallStaticIntMethod(activity, RewardedVideoShow);        
    int retval = (int)value;
    logDebug("retval %d", retval);
    env->DeleteLocalRef(activity);
    
    if(retval) return true;
    return false;
};

#endif // __AKK0RD_ANDROIDADMOBWRAPPER_H__