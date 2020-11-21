#define __AKKORD_SDK_GETGLESPROCADDR__ // требуется явно получать адреса GLES-функций
#include "video_opengles.h"
#undef __AKKORD_SDK_GETGLESPROCADDR__
#include "video_metal.h"

static VideoAdapter_OPENGLES glesAdapter;

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_OPENGLES>();
}

VideoAdapter* VideoAdapter::GetInstance() {
    return &glesAdapter;
};