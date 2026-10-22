#define __AKKORD_SDK_GETGLESPROCADDR__ // требуется явно получать адреса GLES-функций
#include "video_opengl_win.h"
#undef __AKKORD_SDK_GETGLESPROCADDR__
#include "video_gpu.h"

static VideoAdapter_OPENGLWIN winAdapter;
static VideoAdapter_GPU gpuAdapter;

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_OPENGLWIN>();
}

// бэкенд — по рендереру, который фактически создан (VideoDriver::Init вызывается после BWrapper::CreateRenderer)
VideoAdapter* VideoAdapter::GetInstance() {
    if (VideoAdapter_GPU::IsGPURenderer(BWrapper::GetActiveRenderer())) {
        return &gpuAdapter;
    }
    return &winAdapter;
};
