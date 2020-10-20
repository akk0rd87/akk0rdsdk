#include "video_opengles.h"
#include "video_metal.h"

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::unique_ptr<VideoAdapter_OPENGLES>(new VideoAdapter_OPENGLES());
}