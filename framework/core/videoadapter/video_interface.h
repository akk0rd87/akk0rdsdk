#ifndef __AKK0RD_SDK_VIDEOADAPTER_INTERFACE_H__
#define __AKK0RD_SDK_VIDEOADAPTER_INTERFACE_H__
#include <memory>
#include "basewrapper.h"
#include "SDL.h"

struct VideoFloatRect { float x, y, w, h; };
struct VideoBufferAppendParams {
    const VideoFloatRect* DestRect;
    const VideoFloatRect* SrcRect;
    float ScrenW;
    float ScrenH;
    float TextureW;
    float TextureH;
};

struct VideoSDFBufferDrawParams {
    SDL_Texture* Texture;
    const AkkordColor* Color;
    const AkkordColor* OutlineColor;
    bool Outline;
    float Scale;
    float Border;
    int Spread;
};

class VideoBuffer {
public:
    virtual void Clear() = 0;
    virtual void Reserve(unsigned Count) = 0;
    virtual void Append(const VideoBufferAppendParams& Params) = 0;
    virtual void DrawSDF(const VideoSDFBufferDrawParams& Params) = 0;
    virtual ~VideoBuffer() {};
};

class VideoAdapter {
public:
    enum struct Type : unsigned char { Auto, OPENGL, OPENGLES, Metal, Vulkan };

    virtual void PreInit() = 0;
    virtual void PostInit() = 0;
    virtual void InitSDFPlain() = 0;
    virtual void InitSDFOutline() = 0;
    virtual void InitGradient() = 0;

    virtual std::unique_ptr<VideoBuffer> CreateVideoBuffer() = 0;
    virtual void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) = 0;

    static std::unique_ptr<VideoAdapter> CreateVideoAdapter();
    static VideoAdapter* GetInstance();
    virtual ~VideoAdapter() {}
protected:
    static constexpr const char* adapterOpenGLES = "opengles2";
    static constexpr const char* adapterOpenGL = "opengl";
    static constexpr const char* adapterMetal = "metal";
    //static constexpr const char* adapterVulkan = "";
};

#endif // __AKK0RD_SDK_VIDEOADAPTER_INTERFACE_H__