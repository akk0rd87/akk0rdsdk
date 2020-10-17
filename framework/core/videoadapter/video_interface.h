#ifndef __AKK0RD_SDK_VIDEOADAPTER_INTERFACE_H__
#define __AKK0RD_SDK_VIDEOADAPTER_INTERFACE_H__
#include <memory>
#include "basewrapper.h"

class VideoAdapter {
public:
    enum struct Type : unsigned char { Auto, OPENGL, OPENGLES, Metal, Vulkan };

    virtual void PreInit() = 0;
    virtual void PostInit() = 0;
    virtual void InitSDFPlain() = 0;
    virtual void InitSDFOutline() = 0;
    virtual void InitGradient() = 0;

    virtual void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) = 0;
    virtual void DrawSDF(SDL_Texture* Texture, bool Outline, const AkkordColor& Color, const AkkordColor& OutlineColor, const std::vector<GLfloat>& UV, const std::vector<GLfloat>& squareVertices, const std::vector <GLushort>& Indices, GLfloat Scale, GLfloat Border, int Spread) = 0;

    static std::unique_ptr<VideoAdapter> CreateVideoAdapter();
    virtual ~VideoAdapter() {}
protected:
    static constexpr const char* adapterOpenGLES = "opengles2";
    static constexpr const char* adapterOpenGL = "opengl";
    static constexpr const char* adapterMetal = "metal";
    //static constexpr const char* adapterVulkan = "";
};

#endif // __AKK0RD_SDK_VIDEOADAPTER_INTERFACE_H__