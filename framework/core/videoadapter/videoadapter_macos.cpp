#include "video_gpu.h"

// На macOS — только GPU-рендерер SDL3 (Metal): SDF-шейдер есть в MSL, как на iOS-устройстве
static VideoAdapter_GPU gpuAdapter;

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_GPU>();
}

VideoAdapter* VideoAdapter::GetInstance() {
    return &gpuAdapter;
};
