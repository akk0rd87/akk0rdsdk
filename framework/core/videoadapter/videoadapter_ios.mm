#include "video_opengles.h"
#include "video_metal.h"

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_OPENGLES>();
}