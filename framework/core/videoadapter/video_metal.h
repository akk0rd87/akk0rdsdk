#ifndef __AKK0RD_SDK_VIDEOADAPTER_METAL_H__
#define __AKK0RD_SDK_VIDEOADAPTER_METAL_H__

#include "video_interface.h"
#import <Metal/Metal.h>

class VideoBuffer_Metal;

class VideoAdapter_Metal : public VideoAdapter {
public:
    virtual void PreInit() override {};
    virtual void PostInit() override {};
    virtual void InitSDFPlain() override {};
    virtual void InitSDFOutline() override {};
    virtual void InitGradient() override {};

    virtual std::unique_ptr<VideoBuffer> CreateVideoBuffer() override;
    virtual void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) override {};

private:
    friend class VideoBuffer_Metal;
    void DrawSDFBuffer(const VideoBuffer_Metal& Buffer, const VideoSDFBufferDrawParams& Params);
};

class VideoBuffer_Metal : public VideoBuffer {
public:
    friend class VideoAdapter_Metal;
    virtual void Clear() override {};
    virtual void Reserve(unsigned Count) override {};
    virtual void Append(const VideoBufferAppendParams& Params) override {};
    virtual void DrawSDF(const VideoSDFBufferDrawParams& Params) override {
        // check is not empty
        videoAdapter->DrawSDFBuffer(*this, Params);
    };

    VideoBuffer_Metal(VideoAdapter_Metal* VideoAdapter) : videoAdapter(VideoAdapter) {}
private:
    VideoAdapter_Metal* videoAdapter;
};

std::unique_ptr<VideoBuffer> VideoAdapter_Metal::CreateVideoBuffer() {
    return std::make_unique<VideoBuffer_Metal>(this);
};

void VideoAdapter_Metal::DrawSDFBuffer(const VideoBuffer_Metal& Buffer, const VideoSDFBufferDrawParams& Params) {

}

#endif __AKK0RD_SDK_VIDEOADAPTER_METAL_H__