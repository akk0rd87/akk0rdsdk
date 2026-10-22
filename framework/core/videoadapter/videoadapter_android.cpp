// На Android не требуется явно получать адреса GLES-функций
#include "video_opengles.h"
#include "video_gpu.h"

static VideoAdapter_OPENGLES glesAdapter;
static VideoAdapter_GPU gpuAdapter;

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_OPENGLES>();
}

// бэкенд — по рендереру, который фактически создан (VideoDriver::Init вызывается после BWrapper::CreateRenderer)
VideoAdapter* VideoAdapter::GetInstance() {
    if (VideoAdapter_GPU::IsGPURenderer(BWrapper::GetActiveRenderer())) {
        return &gpuAdapter;
    }
    return &glesAdapter;
};
