LOCAL_PATH:= $(call my-dir)
DAV1D_PATH := $(LOCAL_PATH)/../dav1d

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/include \
    $(DAV1D_PATH)/android \
    $(DAV1D_PATH)/include \

LOCAL_SRC_FILES :=  \
    src/alpha.c     \
    src/avif.c      \
    src/codec_dav1d.c \
    src/colr.c      \
    src/diag.c      \
    src/exif.c      \
    src/io.c        \
    src/mem.c       \
    src/obu.c       \
    src/rawdata.c   \
    src/read.c      \
    src/reformat.c  \
    src/reformat_libsharpyuv.c \
    src/reformat_libyuv.c \
    src/scale.c     \
    src/stream.c    \
    src/utils.c     \
    src/write.c     \

LOCAL_CFLAGS := -DAVIF_CODEC_DAV1D

LOCAL_MODULE := avif

include $(BUILD_STATIC_LIBRARY)
