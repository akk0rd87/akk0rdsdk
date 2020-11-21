#define __AKKORD_SDK_GETGLESPROCADDR__ // требуется явно получать адреса GLES-функций
#include "video_opengl_win.h"
#undef __AKKORD_SDK_GETGLESPROCADDR__

static VideoAdapter_OPENGLWIN winAdapter;

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_OPENGLWIN>();
}

VideoAdapter* VideoAdapter::GetInstance() {
    return &winAdapter;
};