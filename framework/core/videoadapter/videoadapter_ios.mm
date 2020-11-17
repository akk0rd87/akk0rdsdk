#include "video_opengles.h"
#include "video_metal.h"

static VideoAdapter_OPENGLES glesAdapter;

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_OPENGLES>();
}

VideoAdapter* VideoAdapter::GetInstance() {
    return &glesAdapter;
};