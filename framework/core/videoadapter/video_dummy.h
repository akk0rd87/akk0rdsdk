#ifndef __AKK0RD_SDK_VIDEOADAPTER_DUMMY_H__
#define __AKK0RD_SDK_VIDEOADAPTER_DUMMY_H__

#include "video_interface.h"

class VideoBuffer_Dummy : public VideoBuffer {
public:
    virtual void Clear() override {};
    virtual void Reserve(unsigned Count) override {};
    virtual void Append(const VideoBufferAppendParams& Params) override {};
    virtual void DrawSDF(const VideoSDFBufferDrawParams& Params) override {};
    VideoBuffer_Dummy() {}
};

class VideoAdapter_Dummy : public VideoAdapter {
public:

    virtual void PreInit() override {};
    virtual void PostInit() override {};
    virtual void InitSDFPlain() override {};
    virtual void InitSDFOutline() override {};
    virtual void InitGradient() override {};
    virtual void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) override {};
    virtual std::unique_ptr<VideoBuffer> CreateVideoBuffer() override { return std::make_unique<VideoBuffer_Dummy>(); };
};

#endif //__AKK0RD_SDK_VIDEOADAPTER_DUMMY_H__