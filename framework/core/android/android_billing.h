#pragma once
#ifndef __AKK0RD_ANDROID_INAPP_BILLING_H__
#define __AKK0RD_ANDROID_INAPP_BILLING_H__

#include "basewrapper.h"
#include <jni.h>
#include "core/core_defines.h"

static jclass    AndroidBillingClass    = nullptr;
static jmethodID midQueryProductDetails = nullptr;
static jmethodID midRestorePurchases    = nullptr;
static jmethodID midPurchaseProdItem    = nullptr;
static jmethodID midConsumeProductItem  = nullptr;

class AndroidBillingManager
{
    
private:
    
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
    jclass localClass =  env->FindClass("org/akkord/lib/BillingManager");
	AndroidBillingClass = reinterpret_cast<jclass>(env->NewGlobalRef(localClass));
	env->DeleteLocalRef(localClass);
    
    // Common Initialize method
    jmethodID BillingInit = env->GetStaticMethodID(AndroidBillingClass, "Initialize", "()V");
    if(!BillingInit)
    {
        logError("Initialize Java method not Found");        
        return false;        
    }    
    
    env->CallStaticVoidMethod(AndroidBillingClass, BillingInit);
	
	midQueryProductDetails = env->GetStaticMethodID(AndroidBillingClass, "QueryProductDetails", "([Ljava/lang/String;)V");
    midRestorePurchases    = env->GetStaticMethodID(AndroidBillingClass, "RestorePurchases", "()V");
	midPurchaseProdItem    = env->GetStaticMethodID(AndroidBillingClass, "PurchaseProdItem", "(Ljava/lang/String;)V");
	midConsumeProductItem  = env->GetStaticMethodID(AndroidBillingClass, "PurchaseProdItem", "(Ljava/lang/String;)V");
	
    return true;
}

bool AndroidBillingManager::QueryProductDetails(const std::vector<std::string>& ProdList)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    //https://communities.ca.com/docs/DOC-99575135
    //jmethodID QueryProductDetails = env->GetStaticMethodID(AndroidBillingClass, "QueryProductDetails", "([Ljava/lang/String;)V");
    if(!midQueryProductDetails)
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
	
    env->CallStaticVoidMethod(AndroidBillingClass, midQueryProductDetails, Java_array);
    //env->DeleteLocalRef(jobjectArray);        
    return true;
}

bool AndroidBillingManager::RestorePurchases()
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    if(!midRestorePurchases)
    {
        logError("RestorePurchases Java method not Found");
        return false;        
    }
    
    env->CallStaticVoidMethod(AndroidBillingClass, midRestorePurchases);
        
    return true;
}

bool AndroidBillingManager::PurchaseProdItem(const char* ProductCode)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();
    
    if(!midPurchaseProdItem)
    {
        logError("PurchaseProdItem Java method not Found");
        return false;        
    }
    
    jstring prod_jstring = (jstring)env->NewStringUTF(ProductCode);
    env->CallStaticVoidMethod(AndroidBillingClass, midPurchaseProdItem, prod_jstring);
    env->DeleteLocalRef(prod_jstring);    
    
    return true;
}

bool AndroidBillingManager::ConsumeProductItem(const char* PurchaseToken)
{
    JNIEnv *env = (JNIEnv*) SDL_AndroidGetJNIEnv();    
    
    if(!midConsumeProductItem)
    {
        logError("ConsumeProductItem Java method not Found");
        return false;        
    }
    
    jstring purch_jstring = (jstring)env->NewStringUTF(PurchaseToken);
    env->CallStaticVoidMethod(AndroidBillingClass, midConsumeProductItem, purch_jstring);
    env->DeleteLocalRef(purch_jstring);
    
    return true;	
}

#endif // __AKK0RD_ANDROID_INAPP_BILLING_H__