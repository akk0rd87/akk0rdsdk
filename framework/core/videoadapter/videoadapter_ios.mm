#include <TargetConditionals.h>
#if TARGET_OS_SIMULATOR
#include "video_opengles.h"
#endif
#include "video_gpu.h"

// На устройстве — только GPU-рендерер SDL3 (Metal), SDL3 собирается без OpenGL ES (libraries/SDL/ios/build_sdl3.sh).
// В симуляторе GPU-рендерер не создается (Metal-устройство симулятора — MTLGPUFamilyApple2, SDL_GPU нужен Apple3),
// поэтому там SDL3 собирается с OpenGL ES и SDF рисует GL-бэкенд
static VideoAdapter_GPU gpuAdapter;
#if TARGET_OS_SIMULATOR
static VideoAdapter_OPENGLES glesAdapter;
#endif

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_GPU>();
}

// бэкенд — по рендереру, который фактически создан (VideoDriver::Init вызывается после BWrapper::CreateRenderer)
VideoAdapter* VideoAdapter::GetInstance() {
#if TARGET_OS_SIMULATOR
    if (!VideoAdapter_GPU::IsGPURenderer(BWrapper::GetActiveRenderer())) {
        return &glesAdapter;
    }
#endif
    return &gpuAdapter;
};
