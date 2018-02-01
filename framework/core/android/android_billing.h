#pragma once
#ifndef __AKK0RD_ANDROID_INAPP_BILLING_H__
#define __AKK0RD_ANDROID_INAPP_BILLING_H__

#include "basewrapper.h"
#include <jni.h>
#include "core/core_defines.h"

class AndroidBillingManager
{
    
private:    
    static jclass FindBillingClass(JNIEnv *env)
    {
        const char* ClassName = "org/akkord/lib/BillingManager";
        jclass BillingClass = env->FindClass(ClassName);
        if(!BillingClass)
        {
            logError("Cannot find class %s", ClassName);
            return nullptr;
        }
        return BillingClass;
    };
    
public:
    static bool Init();
    static bool QueryProductDetails(const std::vector<std::string>& ProdList);
    static bool RestorePurchases();
    static bool PurchaseProdItem(const char* ProductCode);
    static bool ConsumeProductItem(const char* PurchaseToken);
};

//----------
//----------
//----------

bool AndroidBillingManager::Init()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindBillingClass(env);
    
    // Common Initialize method
    jmethodID BillingInit = env->GetStaticMethodID(activity, "Initialize", "()V");
    if(!BillingInit)
    {
        logError("Initialize Java method not Found");        
        return false;        
    }    
    
    env->CallStaticVoidMethod(activity, BillingInit);
    env->DeleteLocalRef(activity);
    
    return true;
}

bool AndroidBillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindBillingClass(env);

    //https://communities.ca.com/docs/DOC-99575135
    jmethodID QueryProductDetails = env->GetStaticMethodID(activity, "QueryProductDetails", "([Ljava/lang/String;)V");
    if(!QueryProductDetails)
    {
        logError("QueryProductDetails Java method not Found");
        return false;        
    }
    
    jobjectArray Java_array = env->NewObjectArray(ProdList.size(), env->FindClass("java/lang/String"), env->NewStringUTF(""));
    
    //https://stackoverflow.com/questions/20819004/call-jar-file-from-c-how-to-get-and-send-args
    int i = 0;
    for(auto v : ProdList)
    {
        env->SetObjectArrayElement(Java_array, i, env->NewStringUTF(v.c_str()));
        ++i;
    }
    
    env->CallStaticVoidMethod(activity, QueryProductDetails, Java_array);    
    
    //env->DeleteLocalRef(jobjectArray);
    env->DeleteLocalRef(activity);    
    return true;
}

bool AndroidBillingManager::RestorePurchases()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindBillingClass(env);

    jmethodID RestorePurchases = env->GetStaticMethodID(activity, "RestorePurchases", "()V");
    if(!RestorePurchases)
    {
        logError("RestorePurchases Java method not Found");
        return false;        
    }
    
    env->CallStaticVoidMethod(activity, RestorePurchases);
    env->DeleteLocalRef(activity);    
    return true;
}

bool AndroidBillingManager::PurchaseProdItem(const char* ProductCode)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindBillingClass(env);
    
    jmethodID PurchaseProdItem = env->GetStaticMethodID(activity, "PurchaseProdItem", "(Ljava/lang/String;)V");
    if(!PurchaseProdItem)
    {
        logError("PurchaseProdItem Java method not Found");
        return false;        
    }
    
    jstring prod_jstring = (jstring)env->NewStringUTF(ProductCode);
    env->CallStaticVoidMethod(activity, PurchaseProdItem, prod_jstring);
    env->DeleteLocalRef(prod_jstring);
    env->DeleteLocalRef(activity);
    
    return true;
}

bool AndroidBillingManager::ConsumeProductItem(const char* PurchaseToken)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    jclass activity = FindBillingClass(env);

    jmethodID ConsumeProductItem = env->GetStaticMethodID(activity, "ConsumeProductItem", "(Ljava/lang/String;)V");
    if(!ConsumeProductItem)
    {
        logError("ConsumeProductItem Java method not Found");
        return false;        
    }
    
    jstring purch_jstring = (jstring)env->NewStringUTF(PurchaseToken);
    env->CallStaticVoidMethod(activity, ConsumeProductItem, purch_jstring);
    env->DeleteLocalRef(purch_jstring);
    env->DeleteLocalRef(activity);    
}

#endif // __AKK0RD_ANDROID_INAPP_BILLING_H__