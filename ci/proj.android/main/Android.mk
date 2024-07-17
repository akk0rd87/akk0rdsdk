LOCAL_SRC_HOME := $(call my-dir)/../src/

include $(CLEAR_VARS)
include $(AKKORD_SDK_HOME)/libraries/SDL/SDL/Android.mk
include $(CLEAR_VARS)
include $(AKKORD_SDK_HOME)/libraries/SDL_image/SDL_image/Android.mk

include $(CLEAR_VARS)

#c++_static
LOCAL_MODULE       := main
LOCAL_C_INCLUDES   := $(AKKORD_SDK_HOME)/libraries/libharu/include
LOCAL_C_INCLUDES   += $(AKKORD_SDK_HOME)/libraries/libharu/out/build/android/include
LOCAL_C_INCLUDES   += $(AKKORD_SDK_HOME)/framework/

LOCAL_SRC_FILES := \
$(wildcard $(LOCAL_SRC_HOME)/*.cpp) \
$(wildcard $(AKKORD_SDK_HOME)/libraries/libharu/src/*.c) \
$(AKKORD_SDK_HOME)/framework/billing.cpp \
$(AKKORD_SDK_HOME)/framework/adrandomizer.cpp \
$(AKKORD_SDK_HOME)/framework/core/platformwrapper/android/android_wrapper.cpp \
$(AKKORD_SDK_HOME)/framework/core/platformwrapper/android/android_utils_wrapper.cpp \
$(AKKORD_SDK_HOME)/framework/basewrapper.cpp \
$(AKKORD_SDK_HOME)/framework/ads/admob/android/admobfactory.cpp \
$(AKKORD_SDK_HOME)/framework/ads/yandex/android/yandexadsfactory.cpp \
$(AKKORD_SDK_HOME)/framework/gdpr/android/gdpr_consentpolicymanager_android.cpp \
$(AKKORD_SDK_HOME)/framework/configmanager.cpp \
$(AKKORD_SDK_HOME)/framework/core/videoadapter/videoadapter_android.cpp \
$(AKKORD_SDK_HOME)/framework/videodriver.cpp

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_image

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -funsigned-char
LOCAL_CPPFLAGS  = -Wall -std=c++17
LOCAL_LDLIBS =  -landroid -lGLESv2

include $(BUILD_SHARED_LIBRARY)