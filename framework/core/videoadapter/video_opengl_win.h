#ifndef __AKK0RD_SDK_VIDEOADAPTER_OPENGL_WIN_H__
#define __AKK0RD_SDK_VIDEOADAPTER_OPENGL_WIN_H__

#include "video_opengles.h"

class VideoAdapter_OPENGLWIN : public VideoAdapter_OPENGLES {
private:
    // https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.1.20.pdf
    static constexpr const char* winGLSL_Version = "#version 120 \n";

    static constexpr const GLchar* SDF_outlineVertexSource_win =
        "#define SDF_OUTLINE \n"
        "varying /*highp*/ vec4 result_color; \
varying /*highp*/ vec2 result_uv; \
uniform /*highp*/ vec4 font_color; \
uniform /*highp*/ float smooth_param; \
attribute /*highp*/ vec2 a_position; \
attribute /*highp*/ vec2 a_texCoord; \
varying /*highp*/ float SmoothDistance; \
varying /*highp*/ float center; \n\
#ifdef SDF_OUTLINE \n\
uniform /*highp*/ vec4 sdf_outline_color; \
uniform /*highp*/ float border; \
varying /*highp*/ vec4 outBorderCol; \
varying /*highp*/ float outlineMaxValue0; \
varying /*highp*/ float outlineMaxValue1; \n\
#endif \n\
void main() \
{\
gl_Position = vec4(a_position, 0.0, 1.0); \
result_color = font_color; \
result_uv = a_texCoord; \
SmoothDistance = smooth_param; \n\
#ifdef SDF_OUTLINE \n\
outBorderCol = sdf_outline_color; \
outlineMaxValue0 = 0.5 - border; \
outlineMaxValue1 = 0.5 + border; \
center = outlineMaxValue0 - border; \n\
#else \n\
center = 0.5; \n\
#endif \n\
}";

    static constexpr const GLchar* SDF_outlineFragmentSource_win =
        "#define SDF_OUTLINE \n"
        "varying /*highp*/ vec4 result_color; \
varying /*highp*/ vec2 result_uv; \
uniform /*highp*/ vec4 sdf_outline_color; \
uniform /*highp*/ sampler2D base_texture; \
varying /*highp*/ float SmoothDistance; \
varying /*highp*/ float outlineMaxValue0; \
varying /*highp*/ float outlineMaxValue1; \
varying /*highp*/ float center; \
varying /*highp*/ vec4 outBorderCol; \
/*highp*/ float my_smoothstep(/*highp*/ float edge0, /*highp*/ float edge1, /*highp*/ float x) { \
x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); \
return x * x * (3.0 - 2.0 * x); \
} \
void main() \
{ \
  /*highp*/ float distAlpha = texture2D(base_texture, result_uv).a; \
  /*highp*/ vec4 rgba = result_color; \n\
#ifdef SDF_OUTLINE \n\
  rgba.xyzw = mix(rgba.xyzw, outBorderCol.xyzw, my_smoothstep(outlineMaxValue1, outlineMaxValue0, distAlpha)); \n\
#endif \n\
  rgba.a *= my_smoothstep(center - SmoothDistance, center + SmoothDistance, distAlpha); \
  gl_FragColor = rgba; \
}";

    static constexpr const GLchar* Gradient_vertexSource_win =
        "attribute /*highp*/ vec2 a_position; \
attribute /*highp*/ vec4 vertex_color; \
varying /*highp*/ vec4 result_color; \
void main() \
{ \
result_color = vertex_color; \
gl_Position = vec4(a_position, 0.0, 1.0); \
}";

    static constexpr const GLchar* Gradient_fragmentSource_win =
        "varying /*highp*/ vec4 result_color; \
void main() \
{ \
gl_FragColor = result_color; \
}";

    virtual bool CompileSDFPlain() override {
        const auto SDF_vertexSource = std::strchr(SDF_outlineVertexSource_win, '\n') + 1;
        const auto SDF_fragmentSource = std::strchr(SDF_outlineFragmentSource_win, '\n') + 1;
        if (!CompileGLProgram(&SDFPlainProgram, (std::string(winGLSL_Version) + SDF_vertexSource).c_str(), (std::string(winGLSL_Version) + SDF_fragmentSource).c_str())) {
            logError("SDF program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileSDFOutline() override {
        if (!CompileGLProgram(&SDFOutlineProgram, (std::string(winGLSL_Version) + SDF_outlineVertexSource_win).c_str(), (std::string(winGLSL_Version) + SDF_outlineFragmentSource_win).c_str())) {
            logError("SDF Outline program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileLinearGradient() override {
        if (!CompileGLProgram(&LinearGradientProgram, (std::string(winGLSL_Version) + Gradient_vertexSource_win).c_str(), (std::string(winGLSL_Version) + Gradient_fragmentSource_win).c_str())) {
            logError("SDF LinearGradient program compilation error!");
            return false;
        }
        return true;
    }
};

#endif // __AKK0RD_SDK_VIDEOADAPTER_OPENGL_WIN_H__