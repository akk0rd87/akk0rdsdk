#pragma once
#ifndef __AKK0RD_SDK_ANDROID_INAPP_BILLING_H__
#define __AKK0RD_SDK_ANDROID_INAPP_BILLING_H__

#include <jni.h>

static jclass    AndroidBillingClass    = nullptr;
static jmethodID midQueryProductDetails = nullptr;
static jmethodID midRestorePurchases    = nullptr;
static jmethodID midPurchaseProdItem    = nullptr;
static jmethodID midConsumeProductItem  = nullptr;

class AndroidBillingManager
{

private:

public:
    static bool Init(JNIEnv* jniEnv);
    static bool QueryProductDetails(JNIEnv* jniEnv, const std::vector<std::string>& ProductSKUList);
    static bool RestorePurchases(JNIEnv* jniEnv);
    static bool PurchaseProdItem(JNIEnv* jniEnv, const char* ProductSKU);
    static bool ConsumeProductItem(JNIEnv* jniEnv, const char* PurchaseToken, const char* ProductSKU);

    //Запрещаем создавать экземпляр класса AndroidBillingManager
    AndroidBillingManager() = delete;
    ~AndroidBillingManager() = delete;
    AndroidBillingManager(AndroidBillingManager& rhs)  = delete; // Копирующий: конструктор
    AndroidBillingManager(AndroidBillingManager&& rhs) = delete; // Перемещающий: конструктор
    AndroidBillingManager& operator= (AndroidBillingManager&& rhs) = delete; // Оператор перемещающего присваивания
};

//----------
//----------
//----------

bool AndroidBillingManager::Init(JNIEnv* jniEnv)
{
    bool Result = true;
    jclass localClass =  jniEnv->FindClass("org/akkord/lib/BillingManager");
    if(!localClass)
    {
        logError("Could not find BillingManager class");
        return false;
    }

    AndroidBillingClass = reinterpret_cast<jclass>(jniEnv->NewGlobalRef(localClass));
    jniEnv->DeleteLocalRef(localClass);

    // Common Initialize method
    jmethodID BillingInit = jniEnv->GetStaticMethodID(AndroidBillingClass, "initialize", "()V");
    if(!BillingInit)
    {
        logError("Initialize Java method not Found");
        return false;
    }

    jniEnv->CallStaticVoidMethod(AndroidBillingClass, BillingInit);

    midQueryProductDetails = jniEnv->GetStaticMethodID(AndroidBillingClass, "queryProductDetails", "([Ljava/lang/String;)V");
    midRestorePurchases    = jniEnv->GetStaticMethodID(AndroidBillingClass, "restorePurchases", "()V");
    midPurchaseProdItem    = jniEnv->GetStaticMethodID(AndroidBillingClass, "purchaseProdItem", "(Ljava/lang/String;)V");
    midConsumeProductItem  = jniEnv->GetStaticMethodID(AndroidBillingClass, "consumeProductItem", "(Ljava/lang/String;Ljava/lang/String;)V");

    if(midQueryProductDetails == nullptr) { Result = false; logError("Java midQueryProductDetails load error"); }
    if(midRestorePurchases    == nullptr) { Result = false; logError("Java midRestorePurchases    load error"); }
    if(midPurchaseProdItem    == nullptr) { Result = false; logError("Java midPurchaseProdItem    load error"); }
    if(midConsumeProductItem  == nullptr) { Result = false; logError("Java midConsumeProductItem  load error"); }

    return Result;
}

bool AndroidBillingManager::QueryProductDetails(JNIEnv* jniEnv, const std::vector<std::string>& ProductSKUList)
{
    //https://communities.ca.com/docs/DOC-99575135
    //jmethodID QueryProductDetails = jniEnv->GetStaticMethodID(AndroidBillingClass, "QueryProductDetails", "([Ljava/lang/String;)V");
    if(!midQueryProductDetails)
    {
        logError("QueryProductDetails Java method not Found");
        return false;
    }

    jobjectArray Java_array = jniEnv->NewObjectArray(ProductSKUList.size(), jniEnv->FindClass("java/lang/String"), jniEnv->NewStringUTF(""));

    //https://stackoverflow.com/questions/20819004/call-jar-file-from-c-how-to-get-and-send-args
    // how to release https://stackoverflow.com/questions/12207941/proper-way-to-clean-up-new-object-array-in-jni
    int i = 0;
    for(const auto& v : ProductSKUList) {
        jniEnv->SetObjectArrayElement(Java_array, i, jniEnv->NewStringUTF(v.c_str()));
        ++i;
    }

    jniEnv->CallStaticVoidMethod(AndroidBillingClass, midQueryProductDetails, Java_array);
    //jniEnv->DeleteLocalRef(jobjectArray);
    return true;
}

bool AndroidBillingManager::RestorePurchases(JNIEnv* jniEnv)
{
    if(!midRestorePurchases)
    {
        logError("RestorePurchases Java method not Found");
        return false;
    }

    jniEnv->CallStaticVoidMethod(AndroidBillingClass, midRestorePurchases);

    return true;
}

bool AndroidBillingManager::PurchaseProdItem(JNIEnv* jniEnv, const char* ProductSKU)
{
    if(!midPurchaseProdItem)
    {
        logError("PurchaseProdItem Java method not Found");
        return false;
    }

    jstring prod_jstring = (jstring)jniEnv->NewStringUTF(ProductSKU);
    jniEnv->CallStaticVoidMethod(AndroidBillingClass, midPurchaseProdItem, prod_jstring);
    jniEnv->DeleteLocalRef(prod_jstring);

    return true;
}

bool AndroidBillingManager::ConsumeProductItem(JNIEnv* jniEnv, const char* PurchaseToken, const char* ProductSKU)
{
    if(!midConsumeProductItem)
    {
        logError("ConsumeProductItem Java method not Found");
        return false;
    }

    jstring purch_jstring = (jstring)jniEnv->NewStringUTF(PurchaseToken);
    jstring prod_jstring  = (jstring)jniEnv->NewStringUTF(ProductSKU);
    jniEnv->CallStaticVoidMethod(AndroidBillingClass, midConsumeProductItem, purch_jstring, prod_jstring);
    jniEnv->DeleteLocalRef(purch_jstring);
    jniEnv->DeleteLocalRef(prod_jstring);

    return true;
}

#endif // __AKK0RD_SDK_ANDROID_INAPP_BILLING_H__