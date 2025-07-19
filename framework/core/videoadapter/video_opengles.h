#ifndef __AKK0RD_SDK_VIDEOADAPTER_OPENGLES_H__
#define __AKK0RD_SDK_VIDEOADAPTER_OPENGLES_H__

#include <cstring>
#include <SDL3/SDL_opengles2.h>
#include "video_interface.h"
#include <array>

class GLESBaseProgram {
public:
    GLuint programId{ 0 }, vertexShader{ 0 }, fragmentShader{ 0 };
    virtual ~GLESBaseProgram() {};
};

class GLESLinearGradientProgram : public GLESBaseProgram {
};

class GLESSDFProgram : public GLESBaseProgram {
public:
    GLint sdf_outline_color{ 0 }, font_color{ 0 }, smooth{ 0 }, border{ 0 };
    GLESSDFProgram() : GLESBaseProgram() {}
};

#ifdef __AKK0RD_SDK_DEBUG_MACRO__
#define CheckGLESError()            CheckError    (         __FILE__, __FUNCTION__, __LINE__)
#define PrintGLESProgamLog(Program) PrintProgamLog(Program, __FILE__, __FUNCTION__, __LINE__)
#define PrintGLESShaderLog(Shader)  PrintShaderLog(Shader , __FILE__, __FUNCTION__, __LINE__)
#else
#define CheckGLESError()
#define PrintGLESProgamLog(Program)
#define PrintGLESShaderLog(Shader)
#endif

#ifdef __AKKORD_SDK_GETGLESPROCADDR__
#define OPENGLES2_ADDITIONAL_FUNC_LIST \
SDL_PROC(void, glGetVertexAttribiv, (GLuint, GLenum, GLint*)) \
SDL_PROC(void, glUniform1f, (GLint, GLfloat)) \
SDL_PROC(void, glDrawElements, (GLenum, GLsizei, GLenum, const GLvoid*))
#endif

class VideoBuffer_OPENGLES;
class VideoAdapter_OPENGLES : public VideoAdapter {
public:
    virtual std::unique_ptr<VideoBuffer> CreateVideoBuffer() override; // forward declaration

    ~VideoAdapter_OPENGLES() {
        DeleteProgram(&SDFPlainProgram);
        DeleteProgram(&SDFOutlineProgram);
        DeleteProgram(&LinearGradientProgram);
    }

    virtual void PreInit() override {
#ifdef __AKKORD_SDK_GETGLESPROCADDR__
#define SDL_PROC(ret,func,params) func = (func##_fnc)SDL_GL_GetProcAddress(#func); if(func) logVerbose("GLESDriver:: " #func " pointer was loaded successfully"); else logError("GLESDriver:: " #func " pointer was not loaded");
#include "../src/render/opengles2/SDL_gles2funcs.h"
        OPENGLES2_ADDITIONAL_FUNC_LIST // включаем дополнительные OPENGLES-функции
#undef SDL_PROC
#endif
            ;
        SDFOutlineProgram.programId = SDFPlainProgram.programId = LinearGradientProgram.programId = 0;
    };

    virtual void PostInit() override {};

    virtual void InitSDFPlain() override {
        if (!CompileSDFPlain()) {
            logError("error compile SDFPlainProgram");
            return;
        }

        if (!BindSDFProgram(SDFPlainProgram)) {
            logError("error bind SDFPlainProgram");
            return;
        }
    };

    virtual void InitSDFOutline() override {
        if (!CompileSDFOutline()) {
            logError("error compile SDFPlainProgram");
            return;
        }

        if (!BindSDFProgram(SDFOutlineProgram)) {
            logError("error bind SDFOutlineProgram");
            return;
        }
    };

    virtual void InitGradient() override {
        if (!CompileLinearGradient()) {
            logError("error compile SDFPlainProgram");
            return;
        }

        glBindAttribLocation(LinearGradientProgram.programId, Attributes::SDF_ATTRIB_UV, "vertex_color"); CheckGLESError(); PrintGLESProgamLog(LinearGradientProgram.programId);
        glBindAttribLocation(LinearGradientProgram.programId, Attributes::SDF_ATTRIB_POSITION, "a_position"); CheckGLESError(); PrintGLESProgamLog(LinearGradientProgram.programId);
        glLinkProgram(LinearGradientProgram.programId); CheckGLESError(); PrintGLESProgamLog(LinearGradientProgram.programId);
        GLint oldProgramId{ 0 };
        glGetIntegerv(static_cast<GLenum>(GL_CURRENT_PROGRAM), &oldProgramId);
        glUseProgram(LinearGradientProgram.programId); CheckGLESError(); PrintGLESProgamLog(LinearGradientProgram.programId);

        if (oldProgramId > 0) {
            glUseProgram(oldProgramId);
        }
    };
protected:
    GLESSDFProgram SDFPlainProgram, SDFOutlineProgram;
    GLESLinearGradientProgram LinearGradientProgram;

    bool CompileGLProgram(GLESBaseProgram* Program, const char* VertextShader, const char* FragmentShader) {
        // Create and compile the fragment shader
        Program->vertexShader = glCreateShader(GL_VERTEX_SHADER); CheckGLESError(); PrintGLESShaderLog(Program->vertexShader);
        glShaderSource(Program->vertexShader, 1, &VertextShader, NULL); CheckGLESError(); PrintGLESShaderLog(Program->vertexShader);
        glCompileShader(Program->vertexShader); CheckGLESError(); PrintGLESShaderLog(Program->vertexShader);

        // Create and compile the fragment shader
        Program->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); CheckGLESError(); PrintGLESShaderLog(Program->fragmentShader);
        glShaderSource(Program->fragmentShader, 1, &FragmentShader, NULL); CheckGLESError(); PrintGLESShaderLog(Program->fragmentShader);
        glCompileShader(Program->fragmentShader); CheckGLESError(); PrintGLESShaderLog(Program->fragmentShader);

        // Link the vertex and fragment shader into a shader program
        Program->programId = glCreateProgram(); CheckGLESError(); PrintGLESProgamLog(Program->programId);
        glAttachShader(Program->programId, Program->vertexShader); CheckGLESError(); PrintGLESProgamLog(Program->programId);
        glAttachShader(Program->programId, Program->fragmentShader); CheckGLESError(); PrintGLESProgamLog(Program->programId);

        return true;
    }

    virtual const char* getTextureIdProperty() const {
        return SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_NUMBER;
    }

    virtual const char* getTextureTargetProperty() const {
        return SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_TARGET_NUMBER;
    }

private:
    static constexpr const GLchar* SDF_outlineVertexSource =
        "#define SDF_OUTLINE \n"
        "varying highp vec4 result_color; \
varying highp vec2 result_uv; \
uniform highp vec4 font_color; \
uniform highp float smooth_param; \
attribute highp vec2 a_position; \
attribute highp vec2 a_texCoord; \
varying highp float SmoothDistance; \
varying highp float center; \n\
#ifdef SDF_OUTLINE \n\
uniform highp vec4 sdf_outline_color; \
uniform highp float border; \
varying highp vec4 outBorderCol; \
varying highp float outlineMaxValue0; \
varying highp float outlineMaxValue1; \n\
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

    static constexpr const GLchar* SDF_outlineFragmentSource =
        "#define SDF_OUTLINE \n"
        "varying highp vec4 result_color; \
varying highp vec2 result_uv; \
uniform highp vec4 sdf_outline_color; \
uniform highp sampler2D base_texture; \
varying highp float SmoothDistance; \
varying highp float outlineMaxValue0; \
varying highp float outlineMaxValue1; \
varying highp float center; \
varying highp vec4 outBorderCol; \
highp float my_smoothstep(highp float edge0, highp float edge1, highp float x) { \
x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); \
return x * x * (3.0 - 2.0 * x); \
} \
void main() \
{ \
  highp float distAlpha = texture2D(base_texture, result_uv).a; \
  highp vec4 rgba = result_color; \n\
#ifdef SDF_OUTLINE \n\
  rgba.xyzw = mix(rgba.xyzw, outBorderCol.xyzw, my_smoothstep(outlineMaxValue1, outlineMaxValue0, distAlpha)); \n\
#endif \n\
  rgba.a *= my_smoothstep(center - SmoothDistance, center + SmoothDistance, distAlpha); \
  gl_FragColor = rgba; \
}";

    static constexpr const GLchar* Gradient_vertexSource =
        "attribute highp vec2 a_position; \
attribute highp vec4 vertex_color; \
varying highp vec4 result_color; \
void main() \
{ \
result_color = vertex_color; \
gl_Position = vec4(a_position, 0.0, 1.0); \
}";

    static constexpr const GLchar* Gradient_fragmentSource =
        "varying highp vec4 result_color; \
void main() \
{ \
gl_FragColor = result_color; \
}";

#ifdef __AKKORD_SDK_GETGLESPROCADDR__
#define SDL_PROC(ret,func,params) typedef ret (APIENTRY * func##_fnc)params; func##_fnc func = nullptr;
#include "../src/render/opengles2/SDL_gles2funcs.h"
    OPENGLES2_ADDITIONAL_FUNC_LIST // включаем дополнительные OPENGLES-функции
#undef SDL_PROC
#endif
        struct Attributes {
        enum : GLuint {
            SDF_ATTRIB_POSITION = 0,
            SDF_ATTRIB_UV = 1
            //SDF_NUM_ATTRIBUTES = 7,
            //ATTRIB_COLOR = 8
        };
    };

    void DeleteProgram(GLESBaseProgram* Program) {
        if (Program->vertexShader > 0) {
            glDeleteShader(Program->vertexShader);
        }

        if (Program->fragmentShader > 0) {
            glDeleteShader(Program->fragmentShader);
        }

        if (Program->programId > 0) {
            glDeleteProgram(Program->programId);
        }
    };

    bool CheckError(const char* File, const char* Function, unsigned Line) {
#ifdef __AKK0RD_SDK_DEBUG_MACRO__
        const auto glErr = glGetError();
        if (glErr != GL_NO_ERROR)
        {
            std::string ErrorMsg;
            switch (glErr)
            {
            case GL_INVALID_ENUM:                  ErrorMsg = "GL_INVALID_ENUM"; break;
            case GL_INVALID_OPERATION:             ErrorMsg = "GL_INVALID_OPERATION"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: ErrorMsg = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            case GL_OUT_OF_MEMORY:                 ErrorMsg = "GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_VALUE:                 ErrorMsg = "GL_INVALID_VALUE"; break;
            default:                               ErrorMsg = "UNKNOWN ERROR"; break;
            }
            // тут именно вызов функции напрямую, а не через макрос, чтбоы не терять информацию о месте возникнования события
            BWrapper::Log(BWrapper::LogPriority::Error, File, Function, Line, "glGetError() = %u, Msg = %s", glErr, ErrorMsg.c_str());
            return true;
        }
#endif
        return false;
    }

    void PrintProgamLog(GLuint Program, const char* File, const char* Function, unsigned Line)
    {
#ifdef __AKK0RD_SDK_DEBUG_MACRO__
        GLint logLength = 0;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string slog(logLength + 1, 0);
            glGetProgramInfoLog(Program, logLength, &logLength, &slog.front());
            if (!slog.empty()) {
                // тут именно вызов функции напрямую, а не через макрос, чтбоы не терять информацию о месте возникнования события
                BWrapper::Log(BWrapper::LogPriority::Debug, File, Function, Line, "Program log [Program=%u]: %s", Program, slog.c_str());
            }
        }
#endif
    }

    void PrintShaderLog(GLuint Shader, const char* File, const char* Function, unsigned Line)
    {
#ifdef __AKK0RD_SDK_DEBUG_MACRO__
        GLint logLength = 0;
        glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            std::string slog(logLength + 1, 0);
            glGetShaderInfoLog(Shader, logLength, &logLength, &slog.front());
            if (!slog.empty()) {
                // тут именно вызов функции напрямую, а не через макрос, чтбоы не терять информацию о месте возникнования события
                BWrapper::Log(BWrapper::LogPriority::Debug, File, Function, Line, "Shader log [Shader=%u]: %s", Shader, slog.c_str());
            }
        }
#endif
    };

    void PrintShaderSource(GLuint Shader)
    {
#ifdef __AKK0RD_SDK_DEBUG_MACRO__
        GLint logLength = 0;
        glGetShaderiv(Shader, GL_SHADER_SOURCE_LENGTH, &logLength);
        if (logLength > 0) {
            std::string slog(logLength + 1, 0);
            glGetShaderSource(Shader, logLength, &logLength, &slog.front());
            if (!slog.empty()) {
                logDebug("Shader Source:\n %s\n\n", slog.c_str());
            }
        }
#endif
    }
    struct OpenGLState {
        GLint attr_0_enabled, attr_1_enabled, attr_2_enabled;
        GLint ProgramId;
        OpenGLState() : attr_0_enabled(GL_FALSE), attr_1_enabled(GL_FALSE), attr_2_enabled(GL_FALSE), ProgramId(0) {}
    };

    OpenGLState BackupOpenGLState() {
        OpenGLState p;
        glGetIntegerv(static_cast<GLenum>(GL_CURRENT_PROGRAM), &p.ProgramId); CheckGLESError();
        glGetVertexAttribiv(static_cast<GLuint>(0), static_cast<GLenum>(GL_VERTEX_ATTRIB_ARRAY_ENABLED), &p.attr_0_enabled); CheckGLESError();
        glGetVertexAttribiv(static_cast<GLuint>(1), static_cast<GLenum>(GL_VERTEX_ATTRIB_ARRAY_ENABLED), &p.attr_1_enabled); CheckGLESError();
        glGetVertexAttribiv(static_cast<GLuint>(2), static_cast<GLenum>(GL_VERTEX_ATTRIB_ARRAY_ENABLED), &p.attr_2_enabled); CheckGLESError();
        return p;
    }

    virtual bool CompileSDFPlain() {
        const auto SDF_vertexSource = std::strchr(SDF_outlineVertexSource, '\n') + 1;
        const auto SDF_fragmentSource = std::strchr(SDF_outlineFragmentSource, '\n') + 1;
        if (!CompileGLProgram(&SDFPlainProgram, SDF_vertexSource, SDF_fragmentSource)) {
            logError("SDF program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileSDFOutline() {
        if (!CompileGLProgram(&SDFOutlineProgram, SDF_outlineVertexSource, SDF_outlineFragmentSource)) {
            logError("SDF Outline program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileLinearGradient() {
        if (!CompileGLProgram(&LinearGradientProgram, Gradient_vertexSource, Gradient_fragmentSource)) {
            logError("SDF LinearGradient program compilation error!");
            return false;
        }
        return true;
    }

    bool BindSDFProgram(GLESSDFProgram& SDFProgam) {
        glBindAttribLocation(SDFProgam.programId, Attributes::SDF_ATTRIB_POSITION, "a_position"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);
        glBindAttribLocation(SDFProgam.programId, Attributes::SDF_ATTRIB_UV, "a_texCoord"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);
        glLinkProgram(SDFProgam.programId); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);

        GLint oldProgramId = 0;
        glGetIntegerv(static_cast<GLenum>(GL_CURRENT_PROGRAM), &oldProgramId);
        glUseProgram(SDFProgam.programId); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);

        const auto base_texture = glGetUniformLocation(SDFProgam.programId, "base_texture"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);
        SDFProgam.sdf_outline_color = glGetUniformLocation(SDFProgam.programId, "sdf_outline_color"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);
        SDFProgam.font_color = glGetUniformLocation(SDFProgam.programId, "font_color"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);
        SDFProgam.smooth = glGetUniformLocation(SDFProgam.programId, "smooth_param"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);
        SDFProgam.border = glGetUniformLocation(SDFProgam.programId, "border"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.programId);

        glUniform1i(base_texture, 0);
        if (oldProgramId > 0) {
            glUseProgram(oldProgramId);
        }
        return true;
    }

    template <class UVBuffer, class squareVerticesBuffer, class IndicesBuffer>
    void DrawElements(const UVBuffer& UV, const squareVerticesBuffer& squareVertices, const IndicesBuffer& Indices, GLint UVElementLogicalSize) {
        glVertexAttribPointer(Attributes::SDF_ATTRIB_POSITION, static_cast<GLint>(2), static_cast<GLenum>(GL_FLOAT), static_cast<GLboolean>(GL_FALSE), static_cast<GLsizei>(0), &squareVertices.front()); CheckGLESError();
        glVertexAttribPointer(Attributes::SDF_ATTRIB_UV, UVElementLogicalSize, static_cast<GLenum>(GL_FLOAT), static_cast<GLboolean>(GL_FALSE), static_cast<GLsizei>(0), &UV.front()); CheckGLESError();
        glDrawElements(static_cast<GLenum>(GL_TRIANGLES), static_cast<GLsizei>(Indices.size()), static_cast<GLenum>(GL_UNSIGNED_SHORT), &Indices.front()); CheckGLESError();
    }

    void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) override {
        if (!LinearGradientProgram.programId) {
            logError("Gradient program is initialized!");
            return;
        }
        const auto screenSize = BWrapper::GetScreenSize();
        const float ScrenW = static_cast<decltype(ScrenW)>(screenSize.GetX());
        const float ScrenH = static_cast<decltype(ScrenH)>(screenSize.GetY());

        const std::array<GLushort, 6> Indices = { 0, 1, 3, 1, 2, 3 };

        const auto x0 = static_cast<float>(2 * Rect.GetX());
        const auto x1 = static_cast<float>(2 * (Rect.GetX() + Rect.GetW()));
        const auto y0 = static_cast<float>(2 * (screenSize.GetY() - Rect.GetY()));
        const auto y1 = static_cast<float>(2 * (screenSize.GetY() - Rect.GetY() - Rect.GetH()));

        const std::array<GLfloat, 8> squareVertices = {
            x0 / ScrenW - 1.0F, y0 / ScrenH - 1.0F,
            x1 / ScrenW - 1.0F, y0 / ScrenH - 1.0F,
            x1 / ScrenW - 1.0F, y1 / ScrenH - 1.0F,
            x0 / ScrenW - 1.0F, y1 / ScrenH - 1.0F
        };

        const std::array<GLfloat, 16>UV = {
            static_cast<float>(X0Y0.GetR()) / 255.0F, static_cast<float>(X0Y0.GetG()) / 255.0F, static_cast<float>(X0Y0.GetB()) / 255.0F, static_cast<float>(X0Y0.GetA()) / 255.0F,
            static_cast<float>(X1Y0.GetR()) / 255.0F, static_cast<float>(X1Y0.GetG()) / 255.0F, static_cast<float>(X1Y0.GetB()) / 255.0F, static_cast<float>(X1Y0.GetA()) / 255.0F,
            static_cast<float>(X1Y1.GetR()) / 255.0F, static_cast<float>(X1Y1.GetG()) / 255.0F, static_cast<float>(X1Y1.GetB()) / 255.0F, static_cast<float>(X1Y1.GetA()) / 255.0F,
            static_cast<float>(X0Y1.GetR()) / 255.0F, static_cast<float>(X0Y1.GetG()) / 255.0F, static_cast<float>(X0Y1.GetB()) / 255.0F, static_cast<float>(X0Y1.GetA()) / 255.0F
        };

        const auto openGLState = BackupOpenGLState();
        {
            // эти два атрибута нужно включить, если они выключены
            if (openGLState.attr_0_enabled == GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(0)); CheckGLESError(); }
            if (openGLState.attr_1_enabled == GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(1)); CheckGLESError(); }

            // эти атрибуты выключаем всегда, если они включены
            if (openGLState.attr_2_enabled != GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(2)); CheckGLESError(); }
        }

        if (openGLState.ProgramId != LinearGradientProgram.programId) {
            glUseProgram(LinearGradientProgram.programId); CheckGLESError(); PrintGLESProgamLog(LinearGradientProgram.programId);
        }

        DrawElements(UV, squareVertices, Indices, 4);

        if (LinearGradientProgram.programId != openGLState.ProgramId && openGLState.ProgramId > 0) {
            glUseProgram(openGLState.ProgramId); CheckGLESError();
        }

        { // возвращаем все исходное состояние
          // в рамках обработки градиента мы включили эти атрибуты. Возвращаем их в исходное состояние
            if (openGLState.attr_0_enabled == GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(0)); CheckGLESError(); }
            if (openGLState.attr_1_enabled == GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(1)); CheckGLESError(); }

            if (openGLState.attr_2_enabled != GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(2)); CheckGLESError(); }
        }
    }

    friend class VideoBuffer_OPENGLES;
    void DrawSDFBuffer(const VideoBuffer_OPENGLES& Buffer, const VideoSDFBufferDrawParams& Params);
};

class VideoBuffer_OPENGLES : public VideoBuffer {
public:
    friend class VideoAdapter_OPENGLES;
    virtual void Clear() override {
        UV.clear();
        squareVertices.clear();
        Indices.clear();
    };

    virtual void Reserve(unsigned Count) override {
        UV.reserve(Count * 4);
        squareVertices.reserve(Count * 4);
        Indices.reserve(Count * 6);
    };

    virtual void Append(const VideoBufferAppendParams& Params) override {
        VideoFloatRect Dest;

        Dest.x = Params.SrcRect->x / Params.TextureW;                       //px1
        Dest.y = (Params.SrcRect->x + Params.SrcRect->w) / Params.TextureW; //px2
        Dest.w = (Params.SrcRect->y + Params.SrcRect->h) / Params.TextureH; //py1
        Dest.h = Params.SrcRect->y / Params.TextureH;                       //py2

        const float& px1 = Dest.x;
        const float& px2 = Dest.y;
        const float& py1 = Dest.w;
        const float& py2 = Dest.h;

        const auto initialSizeUV = UV.size();
        const auto initialSizeSV = squareVertices.size();
        const auto initialSizeIndices = Indices.size();

        UV.insert(UV.cend(),
            {
                px1, py1,
                px2, py1,
                px1, py2,
                px2, py2
            });

        Dest.x = static_cast<float>(2 * Params.DestRect->x) / Params.ScrenW - 1.0F;
        Dest.y = static_cast<float>(2 * (Params.ScrenH - Params.DestRect->y)) / Params.ScrenH - 1.0F;
        Dest.w = static_cast<float>(2 * (Params.DestRect->x + Params.DestRect->w)) / Params.ScrenW - 1.0F;
        Dest.h = static_cast<float>(2 * (Params.ScrenH - Params.DestRect->y - Params.DestRect->h)) / Params.ScrenH - 1.0F;

        squareVertices.insert(squareVertices.cend(),
            {
                Dest.x, Dest.h,
                Dest.w, Dest.h,
                Dest.x, Dest.y,
                Dest.w, Dest.y
            });

        const decltype(Indices)::value_type PointsCnt0 = static_cast<decltype(Indices)::value_type>((Indices.size() / 6) * 4);
        const decltype(PointsCnt0) PointsCnt1 = PointsCnt0 + 1;
        const decltype(PointsCnt0) PointsCnt2 = PointsCnt0 + 2;
        const decltype(PointsCnt0) PointsCnt3 = PointsCnt0 + 3;

        Indices.insert(Indices.cend(),
            {
                PointsCnt0, PointsCnt1, PointsCnt2,
                PointsCnt1, PointsCnt2, PointsCnt3
            });

        // если вставка хотя бы в один вектор прошла неуспешно, надо откатить их в предыдущее состояние
        if ((initialSizeUV + 8 != UV.size()) || (initialSizeSV + 8 != squareVertices.size()) || (initialSizeIndices + 6 != Indices.size())) {
            logError("Error append element to SDF Buffer");
            UV.erase(UV.begin() + initialSizeUV, UV.end());
            squareVertices.erase(squareVertices.begin() + initialSizeSV, squareVertices.end());
            Indices.erase(Indices.begin() + initialSizeIndices, Indices.end());
        }
    };

    virtual void DrawSDF(const VideoSDFBufferDrawParams& Params) override {
        if (!Indices.empty()) {
            videoAdapter->DrawSDFBuffer(*this, Params);
        }
    };

    virtual ~VideoBuffer_OPENGLES() {};

    VideoBuffer_OPENGLES(VideoAdapter_OPENGLES* VideoAdapter) : videoAdapter(VideoAdapter) {}
private:
    std::vector<GLfloat>UV;
    std::vector<GLfloat>squareVertices;
    std::vector<GLushort>Indices;
    VideoAdapter_OPENGLES* videoAdapter;
};

std::unique_ptr<VideoBuffer> VideoAdapter_OPENGLES::CreateVideoBuffer() {
    return std::make_unique<VideoBuffer_OPENGLES>(this);
}

void VideoAdapter_OPENGLES::DrawSDFBuffer(const VideoBuffer_OPENGLES& Buffer, const VideoSDFBufferDrawParams& Params) {
    { // костыль: вот тут нужно сыграть на побочном эффекте и забиндить текстуру через вызов рисования
        const AkkordFRect srcrect(0.0F, 0.0F, 0.0F, 0.0F);
        const AkkordFRect dstrect(0.0F, 0.0F, 0.0F, 0.0F);
        SDL_RenderTexture(BWrapper::GetActiveRenderer(), Params.Texture, &srcrect, &dstrect);
    }

    BWrapper::FlushRenderer();
    GLESSDFProgram* shaderProgram{ nullptr };
    if (Params.Outline) {
        if (!SDFOutlineProgram.programId) {
            logError("SDF Outline program not initialized!");
            return;
        }
        shaderProgram = &SDFOutlineProgram;
    }
    else {
        if (!SDFPlainProgram.programId) {
            logError("SDF program not initialized!");
            return;
        }
        shaderProgram = &SDFPlainProgram;
    }

    const auto openGLState = BackupOpenGLState();
    {
        // эти два атрибута нужно включить, если они выключены
        if (openGLState.attr_0_enabled == GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(0)); CheckGLESError(); }
        if (openGLState.attr_1_enabled == GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(1)); CheckGLESError(); }

        // эти атрибуты выключаем всегда, если они включены
        if (openGLState.attr_2_enabled != GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(2)); CheckGLESError(); }
    }

    if (openGLState.ProgramId != shaderProgram->programId) {
        glUseProgram(shaderProgram->programId); CheckGLESError(); PrintGLESProgamLog(shaderProgram->programId);
    }

    //bindTexture(Params.Texture);
    SDL_PropertiesID props = SDL_GetTextureProperties(Params.Texture);
    const auto textureType = SDL_GetNumberProperty(props, getTextureTargetProperty(), 0);
    const auto textureId = SDL_GetNumberProperty(props, getTextureIdProperty(), 0);
    //glEnable(textureType); CheckGLESError();
    glBindTexture(textureType, textureId); CheckGLESError();

#if SDL_HAVE_YUV

#endif

    glUniform4f(shaderProgram->font_color, GLfloat(Params.Color->GetR()) / 255.0F, GLfloat(Params.Color->GetG()) / 255.0F, GLfloat(Params.Color->GetB()) / 255.0F, GLfloat(Params.Color->GetA()) / 255.0F); CheckGLESError();
    const GLfloat smoothness = std::min(0.3F, 0.25F / static_cast<GLfloat>(Params.Spread) / Params.Scale * 1.5F) * 850.0F / 255.0F / 3.333F;
    glUniform1f(shaderProgram->smooth, smoothness); CheckGLESError();

    if (Params.Outline) {
        if (shaderProgram->sdf_outline_color >= 0) {
            glUniform4f(shaderProgram->sdf_outline_color, GLfloat(Params.OutlineColor->GetR()) / 255.0F, GLfloat(Params.OutlineColor->GetG()) / 255.0F, GLfloat(Params.OutlineColor->GetB()) / 255.0F, GLfloat(Params.OutlineColor->GetA()) / 255.0F); CheckGLESError();
        }
        else {
            logError("shaderProgram->sdf_outline_color error %d", shaderProgram->sdf_outline_color);
        }

        if (shaderProgram->border >= 0) {
            glUniform1f(shaderProgram->border, Params.Border / 6.666F); CheckGLESError();
        }
        else {
            logError("shaderProgram->border error %d", shaderProgram->border);
        }
    }

    DrawElements(Buffer.UV, Buffer.squareVertices, Buffer.Indices, 2);
    // unbind texture
    glBindTexture(textureType, 0); CheckGLESError();

    if (shaderProgram->programId != openGLState.ProgramId && openGLState.ProgramId > 0) {
        glUseProgram(openGLState.ProgramId); CheckGLESError();
    }

    { // возвращаем все исходное состояние
        // в рамках обработки SDF мы включили эти атрибуты. Возвращаем их в исходное состояние
        if (openGLState.attr_0_enabled == GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(0)); CheckGLESError(); }
        if (openGLState.attr_1_enabled == GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(1)); CheckGLESError(); }

        if (openGLState.attr_2_enabled != GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(2)); CheckGLESError(); }
    }
};

#undef CheckGLESError
#undef PrintGLESProgamLog
#undef PrintGLESShaderLog

#undef OPENGLES2_ADDITIONAL_FUNC_LIST
#undef SDL_PROC

#endif // __AKK0RD_SDK_VIDEOADAPTER_OPENGLES_H__