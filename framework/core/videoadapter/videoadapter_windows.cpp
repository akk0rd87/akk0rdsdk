#include "video_opengl_win.h"

static VideoAdapter_OPENGLWIN winAdapter;

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::make_unique<VideoAdapter_OPENGLWIN>();
}

VideoAdapter* VideoAdapter::GetInstance() {
    return &winAdapter;
};