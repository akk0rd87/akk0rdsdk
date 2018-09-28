MY_LOCAL_PATH := $(call my-dir)

####### INCLUDE SDL2 - Added by myself
include $(CLEAR_VARS)
LOCAL_MODULE := SDL2
LOCAL_SRC_FILES := $(AKKORD_SDK_HOME)/libraries/SDL/android/lib/$(TARGET_ARCH_ABI)/libSDL2.so
LOCAL_EXPORT_C_INCLUDES  = $(AKKORD_SDK_HOME)/libraries/SDL/SDL/include
include $(PREBUILT_SHARED_LIBRARY)
#######

####### INCLUDE SDL2_image - Added by myself
include $(CLEAR_VARS)
LOCAL_MODULE := SDL2_image
LOCAL_SRC_FILES := $(AKKORD_SDK_HOME)/libraries/SDL_image/android/lib/$(TARGET_ARCH_ABI)/libSDL2_image.so
LOCAL_EXPORT_C_INCLUDES += $(AKKORD_SDK_HOME)/libraries/SDL_image/SDL_image
include $(PREBUILT_SHARED_LIBRARY)
#######

include $(CLEAR_VARS)
# MY_LOCAL_PATH := $(call my-dir)

#c++_static
LOCAL_MODULE       := main
LOCAL_C_INCLUDES   := $(AKKORD_SDK_HOME)/libraries/SDL/SDL/include
LOCAL_C_INCLUDES   += $(AKKORD_SDK_HOME)/libraries/SDL_image/SDL_image
LOCAL_C_INCLUDES   += $(AKKORD_SDK_HOME)/framework/

# https://stackoverflow.com/questions/8980284/android-mk-include-all-cpp-files
APP_LOCAL_SRC_FILE_LIST  := $(wildcard $(MY_LOCAL_PATH)/../../src/*.cpp)

LOCAL_SRC_FILES    := $(AKKORD_SDK_HOME)/framework/admob.cpp \
                      $(AKKORD_SDK_HOME)/framework/billing.cpp \
                      $(AKKORD_SDK_HOME)/framework/basewrapper.cpp \
                      $(APP_LOCAL_SRC_FILE_LIST)

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_image
TARGET_PLATFORM := android-14

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -funsigned-char
LOCAL_CPPFLAGS  = -Wall -std=c++14
#LOCAL_CPP_FEATURES += -std=c++11
#APP_CPPFLAGS += -std=c++11
#APP_CFLAGS += -std=c++11
#LOCAL_CFLAGS+= -std=c++11
#LOCAL_LDLIBS := -ldl -lGLESv1_CM -lGLESv2 -llog -landroid
LOCAL_LDLIBS =  -landroid -latomic

include $(BUILD_SHARED_LIBRARY)