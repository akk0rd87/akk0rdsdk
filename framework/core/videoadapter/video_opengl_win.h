#ifndef __AKK0RD_SDK_VIDEOADAPTER_OPENGL_WIN_H__
#define __AKK0RD_SDK_VIDEOADAPTER_OPENGL_WIN_H__

#include "video_opengles.h"

class VideoAdapter_OPENGLWIN : public VideoAdapter_OPENGLES {
private:
    static constexpr const char* winGLSL_Version = "#version 130 \n";

    virtual bool CompileSDFPlain() override {
        const auto SDF_vertexSource = std::strchr(SDF_outlineVertexSource, '\n') + 1;
        const auto SDF_fragmentSource = std::strchr(SDF_outlineFragmentSource, '\n') + 1;
        if (!CompileGLProgram(&SDFPlainProgram, (std::string(winGLSL_Version) + SDF_vertexSource).c_str(), (std::string(winGLSL_Version) + SDF_fragmentSource).c_str())) {
            logError("SDF program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileSDFOutline() override {
        if (!CompileGLProgram(&SDFOutlineProgram, (std::string(winGLSL_Version) + SDF_outlineVertexSource).c_str(), (std::string(winGLSL_Version) + SDF_outlineFragmentSource).c_str())) {
            logError("SDF Outline program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileLinearGradient() override {
        if (!CompileGLProgram(&LinearGradientProgram, (std::string(winGLSL_Version) + Gradient_vertexSource).c_str(), (std::string(winGLSL_Version) + Gradient_fragmentSource).c_str())) {
            logError("SDF LinearGradient program compilation error!");
            return false;
        }
        return true;
    }
};

#endif // __AKK0RD_SDK_VIDEOADAPTER_OPENGL_WIN_H__