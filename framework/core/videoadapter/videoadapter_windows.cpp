#include "video_opengl_win.h"

std::unique_ptr<VideoAdapter> VideoAdapter::CreateVideoAdapter() {
    return std::unique_ptr<VideoAdapter_OPENGLWIN>(new VideoAdapter_OPENGLWIN());
}