#ifndef __AKK0RD_SDK_VIDEOADAPTER_OPENGLES_H__
#define __AKK0RD_SDK_VIDEOADAPTER_OPENGLES_H__

#include "SDL_opengles2.h"
#include "video_interface.h"
#include <array>

// выборочный список OPENGLES2-функций из файла "../src/render/opengles2/SDL_gles2funcs.h"
// некоторых нижеперечисленных ф-ий нет в файле "../src/render/opengles2/SDL_gles2funcs.h"
#if __NACL__ || __ANDROID__
#define DEFINE_glShaderSource SDL_PROC(void, glShaderSource, (GLuint, GLsizei, const GLchar **, const GLint *))
#else
#define DEFINE_glShaderSource SDL_PROC(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint *))
#endif

#ifdef __AKK0RD_SDK_DEBUG_MACRO__
#define CheckGLESError()            CheckError    (         __FILE__, __FUNCTION__, __LINE__)
#define PrintGLESProgamLog(Program) PrintProgamLog(Program, __FILE__, __FUNCTION__, __LINE__)
#define PrintGLESShaderLog(Shader)  PrintShaderLog(Shader , __FILE__, __FUNCTION__, __LINE__)
#else
#define CheckGLESError()
#define PrintGLESProgamLog(Program)
#define PrintGLESShaderLog(Shader)
#endif

#define OPENGLES2_FUCNTION_LIST \
DEFINE_glShaderSource \
SDL_PROC(void, glBufferData, (GLenum, GLsizeiptr, const GLvoid *, GLenum)) \
SDL_PROC(void, glBufferSubData, (GLenum, GLintptr, GLsizeiptr, const GLvoid *)) \
SDL_PROC(void, glGenBuffers, (GLsizei, GLuint *)) \
SDL_PROC(void, glBindBuffer, (GLenum, GLuint)) \
SDL_PROC(void, glBlendFunc, (GLenum, GLenum)) \
SDL_PROC(void, glGetShaderSource, (GLuint, GLsizei, GLsizei*, GLchar*)) \
SDL_PROC(void, glDrawElements, (GLenum, GLsizei, GLenum, const GLvoid*)) \
SDL_PROC(GLuint, glCreateShader, (GLenum)) \
SDL_PROC(void, glCompileShader, (GLuint)) \
SDL_PROC(GLuint, glCreateProgram, (void)) \
SDL_PROC(void, glAttachShader, (GLuint, GLuint)) \
SDL_PROC(void, glBindAttribLocation, (GLuint, GLuint, const char*)) \
SDL_PROC(void, glLinkProgram, (GLuint)) \
SDL_PROC(void, glGetIntegerv, (GLenum, GLint*)) \
SDL_PROC(void, glUseProgram, (GLuint)) \
SDL_PROC(GLint, glGetUniformLocation, (GLuint, const char*)) \
SDL_PROC(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat*)) \
SDL_PROC(void, glUniform1i, (GLint, GLint)) \
SDL_PROC(void, glGenTextures, (GLsizei, GLuint*)) \
SDL_PROC(void, glBindTexture, (GLenum, GLuint)) \
SDL_PROC(void, glTexParameteri, (GLenum, GLenum, GLint)) \
SDL_PROC(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*)) \
SDL_PROC(void, glEnableVertexAttribArray, (GLuint)) \
SDL_PROC(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void*)) \
SDL_PROC(void, glUniform1f, (GLint, GLfloat)) \
SDL_PROC(void, glUniform4f, (GLint, GLfloat, GLfloat, GLfloat, GLfloat)) \
SDL_PROC(void, glDisableVertexAttribArray, (GLuint)) \
SDL_PROC(GLenum, glGetError, (void)) \
SDL_PROC(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)) \
SDL_PROC(void, glGetProgramiv, (GLuint, GLenum, GLint*)) \
SDL_PROC(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei*, char*)) \
SDL_PROC(void, glGetShaderiv, (GLuint, GLenum, GLint*)) \
SDL_PROC(void, glGetVertexAttribiv, (GLuint, GLenum, GLint*))

class VideoBuffer_OPENGLES;
class VideoAdapter_OPENGLES : public VideoAdapter {
public:
    virtual std::unique_ptr<VideoBuffer> CreateVideoBuffer() override; // forward declaration

    virtual void PreInit() override {
        LoadFuncs();
        InitVBO();
        SDFPlainProgram.shaderProgram = 0;
        SDFOutlineProgram.shaderProgram = 0;
        shaderLinearProgram = 0;
    };

    virtual void PostInit() override {};

    virtual void InitSDFPlain() override {
        if (!CompileSDFPlain(SDFPlainProgram.shaderProgram)) {
            logError("error compile SDFPlainProgram");
            return;
        }

        if (!BindSDFProgram(SDFPlainProgram)) {
            logError("error bind SDFPlainProgram");
            return;
        }
    };

    virtual void InitSDFOutline() override {
        if (!CompileSDFOutline(SDFOutlineProgram.shaderProgram)) {
            logError("error compile SDFPlainProgram");
            return;
        }

        if (!BindSDFProgram(SDFOutlineProgram)) {
            logError("error bind SDFOutlineProgram");
            return;
        }
    };

    virtual void InitGradient() override {
        if (!CompileLinearGradient(shaderLinearProgram)) {
            logError("error compile SDFPlainProgram");
            return;
        }

        glBindAttribLocation(this->shaderLinearProgram, Attributes::SDF_ATTRIB_UV, "vertex_color"); CheckGLESError(); PrintGLESProgamLog(this->shaderLinearProgram);
        glBindAttribLocation(this->shaderLinearProgram, Attributes::SDF_ATTRIB_POSITION, "a_position"); CheckGLESError(); PrintGLESProgamLog(this->shaderLinearProgram);
        glLinkProgram(this->shaderLinearProgram); CheckGLESError(); PrintGLESProgamLog(this->shaderLinearProgram);
        GLint oldProgramId;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
        glUseProgram(this->shaderLinearProgram); CheckGLESError(); PrintGLESProgamLog(this->shaderLinearProgram);

        if (oldProgramId > 0) {
            glUseProgram(oldProgramId);
        }
    };

    void DrawSDFBuffer(const VideoBuffer_OPENGLES& Buffer, const VideoSDFBufferDrawParams& Params);
protected:
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

    bool CompileGLProgram(GLuint& ProgramID, const char* VertextShader, const char* FragmentShader) {
        // Create and compile the fragment shader
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); CheckGLESError(); PrintGLESShaderLog(vertexShader);
        glShaderSource(vertexShader, 1, &VertextShader, NULL); CheckGLESError(); PrintGLESShaderLog(vertexShader);
        glCompileShader(vertexShader); CheckGLESError(); PrintGLESShaderLog(vertexShader);

        // Create and compile the fragment shader
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); CheckGLESError(); PrintGLESShaderLog(fragmentShader);
        glShaderSource(fragmentShader, 1, &FragmentShader, NULL); CheckGLESError(); PrintGLESShaderLog(fragmentShader);
        glCompileShader(fragmentShader); CheckGLESError(); PrintGLESShaderLog(fragmentShader);

        // Link the vertex and fragment shader into a shader program
        ProgramID = glCreateProgram(); CheckGLESError(); PrintGLESProgamLog(ProgramID);
        glAttachShader(ProgramID, vertexShader); CheckGLESError(); PrintGLESProgamLog(ProgramID);
        glAttachShader(ProgramID, fragmentShader); CheckGLESError(); PrintGLESProgamLog(ProgramID);

        return true;
    }

private:
#define SDL_PROC(ret,func,params) typedef ret (APIENTRY * func##_fnc)params; func##_fnc func = 0;
    OPENGLES2_FUCNTION_LIST // включаем дополнительные OPENGLES-функции

        struct Attributes {
        enum : GLuint {
            SDF_ATTRIB_POSITION = 0, // Начинаем не с нуля, чтобы индексы не пересеклись с другими программами
            SDF_ATTRIB_UV = 1
            //SDF_NUM_ATTRIBUTES = 7,
            //ATTRIB_COLOR = 8
        };
    };

    struct SDFShaderProgramStruct
    {
        GLuint shaderProgram{ 0 };
        GLint sdf_outline_color{ 0 }, font_color{ 0 }, smooth{ 0 }, border{ 0 };
        ~SDFShaderProgramStruct() { shaderProgram = 0; }
    } SDFPlainProgram, SDFOutlineProgram;

    GLuint shaderLinearProgram;

    void LoadFuncs() {
#undef  SDL_PROC
#define SDL_PROC(ret,func,params) func = (func##_fnc)SDL_GL_GetProcAddress(#func); if(func) logVerbose("GLESDriver:: " #func " pointer was loaded successfully"); else logError("GLESDriver:: " #func " pointer was not loaded");
        //#include "../src/render/opengles2/SDL_gles2funcs.h"
        OPENGLES2_FUCNTION_LIST // включаем дополнительные OPENGLES-функции
#undef SDL_PROC
    }

    bool CheckError(const char* File, const char* Function, unsigned Line) {
#ifdef __AKK0RD_SDK_DEBUG_MACRO__
        const auto glErr = this->glGetError();
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

    struct VBOBufferStruct {
        static constexpr GLsizei constBufferSize = 8;

        std::array<GLuint, constBufferSize> ArrayBufferID;
        std::array<GLuint, constBufferSize> ElementBufferID;

        std::array<GLsizeiptr, constBufferSize> ArrayBufferSize;
        std::array<GLsizeiptr, constBufferSize> ElementBufferSize;

        GLsizei CurrentBuffer;
    } VBO;

    struct OpenGLState {
        GLint attr_0_enabled{ GL_FALSE }, attr_1_enabled{ GL_FALSE }, attr_2_enabled{ GL_FALSE }, attr_3_enabled{ GL_FALSE };
        GLint ProgramId{ 0 };
    };

    void InitVBO() {
        VBO.CurrentBuffer = 0;
        std::fill(VBO.ArrayBufferID.begin(), VBO.ArrayBufferID.end(), 0);
        std::fill(VBO.ElementBufferID.begin(), VBO.ElementBufferID.end(), 0);
        std::fill(VBO.ArrayBufferSize.begin(), VBO.ArrayBufferSize.end(), 0);
        std::fill(VBO.ElementBufferSize.begin(), VBO.ElementBufferSize.end(), 0);
        glGenBuffers(VBO.constBufferSize, &VBO.ArrayBufferID.front()); CheckGLESError();
        glGenBuffers(VBO.constBufferSize, &VBO.ElementBufferID.front()); CheckGLESError();
    }

    OpenGLState BackupOpenGLState() {
        OpenGLState p;
        glGetIntegerv((GLenum)GL_CURRENT_PROGRAM, &p.ProgramId); CheckGLESError();
        glGetVertexAttribiv((GLuint)0, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &p.attr_0_enabled); CheckGLESError();
        glGetVertexAttribiv((GLuint)1, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &p.attr_1_enabled); CheckGLESError();
        glGetVertexAttribiv((GLuint)2, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &p.attr_2_enabled); CheckGLESError();
        glGetVertexAttribiv((GLuint)3, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &p.attr_3_enabled); CheckGLESError();
        return p;
    }

    virtual bool CompileSDFPlain(GLuint& ProgramID) {
        const auto SDF_vertexSource = std::strchr(SDF_outlineVertexSource, '\n') + 1;
        const auto SDF_fragmentSource = std::strchr(SDF_outlineFragmentSource, '\n') + 1;
        if (!CompileGLProgram(ProgramID, SDF_vertexSource, SDF_fragmentSource)) {
            logError("SDF program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileSDFOutline(GLuint& ProgramID) {
        if (!CompileGLProgram(ProgramID, SDF_outlineVertexSource, SDF_outlineFragmentSource)) {
            logError("SDF Outline program compilation error!");
            return false;
        }
        return true;
    }

    virtual bool CompileLinearGradient(GLuint& ProgramID) {
        if (!CompileGLProgram(ProgramID, Gradient_vertexSource, Gradient_fragmentSource)) {
            logError("SDF LinearGradient program compilation error!");
            return false;
        }
        return true;
    }

    bool BindSDFProgram(SDFShaderProgramStruct& SDFProgam) {
        glBindAttribLocation(SDFProgam.shaderProgram, Attributes::SDF_ATTRIB_POSITION, "a_position"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);
        glBindAttribLocation(SDFProgam.shaderProgram, Attributes::SDF_ATTRIB_UV, "a_texCoord"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);
        glLinkProgram(SDFProgam.shaderProgram); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);

        GLint oldProgramId;
        glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
        glUseProgram(SDFProgam.shaderProgram); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);

        const auto base_texture = glGetUniformLocation(SDFProgam.shaderProgram, "base_texture"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);
        SDFProgam.sdf_outline_color = glGetUniformLocation(SDFProgam.shaderProgram, "sdf_outline_color"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);
        SDFProgam.font_color = glGetUniformLocation(SDFProgam.shaderProgram, "font_color"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);
        SDFProgam.smooth = glGetUniformLocation(SDFProgam.shaderProgram, "smooth_param"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);
        SDFProgam.border = glGetUniformLocation(SDFProgam.shaderProgram, "border"); CheckGLESError(); PrintGLESProgamLog(SDFProgam.shaderProgram);

        glUniform1i(base_texture, 0);
        if (oldProgramId > 0) {
            glUseProgram(oldProgramId);
        }
        return true;
    }

    template <class UVBuffer, class squareVerticesBuffer, class IndicesBuffer>
    void DrawElements(const UVBuffer& UV, const squareVerticesBuffer& squareVertices, const IndicesBuffer& Indices, GLint UVElementLogicalSize) {
        const auto uvSize = static_cast<GLsizeiptr>(UV.size() * sizeof(UV.front()));
        const auto svSize = static_cast<GLsizeiptr>(squareVertices.size() * sizeof(squareVertices.front()));
        const auto bufSize = uvSize + svSize;
        const auto indSize = static_cast<GLsizeiptr>(Indices.size() * sizeof(decltype(Indices.front())));

        // работаем c GL_ARRAY_BUFFER
        glBindBuffer(GL_ARRAY_BUFFER, VBO.ArrayBufferID[VBO.CurrentBuffer]); CheckGLESError();
        if (VBO.ArrayBufferSize[VBO.CurrentBuffer] < bufSize) { // размера недостаточно и нужно выделить память
            glBufferData(GL_ARRAY_BUFFER, bufSize, nullptr, GL_STREAM_DRAW); CheckGLESError();
            VBO.ArrayBufferSize[VBO.CurrentBuffer] = bufSize;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, uvSize, &UV.front()); CheckGLESError();
        glBufferSubData(GL_ARRAY_BUFFER, uvSize, svSize, &squareVertices.front()); CheckGLESError();

        // работаем c GL_ELEMENT_ARRAY_BUFFER
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO.ElementBufferID[VBO.CurrentBuffer]); CheckGLESError();
        if (VBO.ElementBufferSize[VBO.CurrentBuffer] < indSize) { // размера недостаточно и нужно выделить память
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, &Indices.front(), GL_STREAM_DRAW); CheckGLESError();
            VBO.ElementBufferSize[VBO.CurrentBuffer] = indSize;
        }
        else { // если размера хватает, заполняем текущее подмножество
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indSize, &Indices.front()); CheckGLESError();
        }

        glVertexAttribPointer(Attributes::SDF_ATTRIB_POSITION, static_cast<GLint>(2), (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, static_cast<GLsizei>(0), (const GLvoid*)static_cast<GLsizeiptr>(UV.size() * sizeof(UV.front()))); CheckGLESError();
        glVertexAttribPointer(Attributes::SDF_ATTRIB_UV, UVElementLogicalSize, (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, static_cast<GLsizei>(0), nullptr); CheckGLESError();
        glDrawElements((GLenum)GL_TRIANGLES, static_cast<GLsizei>(Indices.size()), (GLenum)GL_UNSIGNED_SHORT, nullptr); CheckGLESError();

        // переходим к следующему буфферу, который будет использоваться при следующем обращении
        if (++VBO.CurrentBuffer >= VBO.constBufferSize) {
            VBO.CurrentBuffer = 0;
        }
    }

    void DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) override {
        if (!this->shaderLinearProgram) {
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
            if (openGLState.attr_0_enabled == GL_FALSE) { glEnableVertexAttribArray((GLuint)0); CheckGLESError(); }
            if (openGLState.attr_1_enabled == GL_FALSE) { glEnableVertexAttribArray((GLuint)1); CheckGLESError(); }

            // эти атрибуты выключаем всегда, если они включены
            if (openGLState.attr_2_enabled != GL_FALSE) { glDisableVertexAttribArray((GLuint)2); CheckGLESError(); }
            if (openGLState.attr_3_enabled != GL_FALSE) { glDisableVertexAttribArray((GLuint)3); CheckGLESError(); }
        }

        if (openGLState.ProgramId != this->shaderLinearProgram) {
            glUseProgram(this->shaderLinearProgram); CheckGLESError(); PrintGLESProgamLog(this->shaderLinearProgram);
        }

        DrawElements(UV, squareVertices, Indices, 4);

        if (this->shaderLinearProgram != openGLState.ProgramId && openGLState.ProgramId > 0) {
            glUseProgram(openGLState.ProgramId); CheckGLESError();
        }

        { // возвращаем все исходное состояние
          // в рамках обработки градиента мы включили эти атрибуты. Возвращаем их в исходное состояние
            if (openGLState.attr_0_enabled == GL_FALSE) { glDisableVertexAttribArray((GLuint)0); CheckGLESError(); }
            if (openGLState.attr_1_enabled == GL_FALSE) { glDisableVertexAttribArray((GLuint)1); CheckGLESError(); }

            if (openGLState.attr_2_enabled != GL_FALSE) { glEnableVertexAttribArray((GLuint)2); CheckGLESError(); }
            if (openGLState.attr_3_enabled != GL_FALSE) { glEnableVertexAttribArray((GLuint)3); CheckGLESError(); }
        }
    }
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

        const decltype(Indices)::value_type PointsCnt0 = Indices.size() / 6 * 4;
        const decltype(PointsCnt0) PointsCnt1 = PointsCnt0 + 1;
        const decltype(PointsCnt0) PointsCnt2 = PointsCnt0 + 2;
        const decltype(PointsCnt0) PointsCnt3 = PointsCnt0 + 3;

        Indices.insert(Indices.cend(),
            {
                PointsCnt0, PointsCnt1, PointsCnt2,
                PointsCnt1, PointsCnt2, PointsCnt3
            });
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
    BWrapper::FlushRenderer();
    decltype(SDFPlainProgram)* shaderProgram{ nullptr };
    if (Params.Outline) {
        if (!SDFOutlineProgram.shaderProgram) {
            logError("SDF Outline program not initialized!");
            return;
        }
        shaderProgram = &SDFOutlineProgram;
    }
    else {
        if (!SDFPlainProgram.shaderProgram) {
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
        if (openGLState.attr_3_enabled != GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(3)); CheckGLESError(); }
    }

    if (openGLState.ProgramId != shaderProgram->shaderProgram) {
        glUseProgram(shaderProgram->shaderProgram); CheckGLESError(); PrintGLESProgamLog(shaderProgram->shaderProgram);
    }
    SDL_GL_BindTexture(Params.Texture, nullptr, nullptr);
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
    SDL_GL_UnbindTexture(Params.Texture);

    if (shaderProgram->shaderProgram != openGLState.ProgramId && openGLState.ProgramId > 0) {
        glUseProgram(openGLState.ProgramId); CheckGLESError();
    }

    { // возвращаем все исходное состояние
        // в рамках обработки SDF мы включили эти атрибуты. Возвращаем их в исходное состояние
        if (openGLState.attr_0_enabled == GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(0)); CheckGLESError(); }
        if (openGLState.attr_1_enabled == GL_FALSE) { glDisableVertexAttribArray(static_cast<GLuint>(1)); CheckGLESError(); }

        if (openGLState.attr_2_enabled != GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(2)); CheckGLESError(); }
        if (openGLState.attr_3_enabled != GL_FALSE) { glEnableVertexAttribArray(static_cast<GLuint>(3)); CheckGLESError(); }
    }
};

#undef CheckGLESError
#undef PrintGLESProgamLog
#undef PrintGLESShaderLog

#undef DEFINE_glShaderSource
#undef OPENGLES2_FUCNTION_LIST

#undef SDL_PROC

#endif // __AKK0RD_SDK_VIDEOADAPTER_OPENGLES_H__