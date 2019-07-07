#include "openglesdriver.h"


    void GLESDriver::Init()
    {
#define SDL_PROC(ret,func,params) func = (func##_fnc)SDL_GL_GetProcAddress(#func); if(func) logVerbose("GLESDriver:: " #func " pointer was loaded successfully"); else logError("GLESDriver:: " #func " pointer was not loaded");
//#include "../src/render/opengles2/SDL_gles2funcs.h"
    OPENGLES2_FUCNTION_LIST // включаем дополнительные OPENGLES-функции
#undef SDL_PROC
    }

    bool GLESDriver::CheckError(const char* File, const char* Function, unsigned Line)
    {
#ifdef __AKK0RD_DEBUG_MACRO__
        auto glErr = this->glGetError();
        if (glErr != GL_NO_ERROR)
        {
            std::string ErrorMsg;
            switch (glErr)
            {
                case GL_INVALID_ENUM:                  ErrorMsg =  "GL_INVALID_ENUM"; break;
                case GL_INVALID_OPERATION:             ErrorMsg =  "GL_INVALID_OPERATION"; break;
                case GL_INVALID_FRAMEBUFFER_OPERATION: ErrorMsg =  "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
                case GL_OUT_OF_MEMORY:                 ErrorMsg =  "GL_OUT_OF_MEMORY"; break;
                case GL_INVALID_VALUE:                 ErrorMsg =  "GL_INVALID_VALUE"; break;
                default:                               ErrorMsg =  "UNKNOWN ERROR"; break;
            }
            // тут именно вызов функции напрямую, а не через макрос, чтбоы не терять информацию о месте возникнования события
            BWrapper::Log(BWrapper::LogPriority::Error, File, Function, Line, "glGetError() = %u, Msg = %s", glErr, ErrorMsg.c_str());
            return true;
        }
#endif
        return false;
    };

    void GLESDriver::PrintProgamLog(GLuint Program, const char* File, const char* Function, unsigned Line)
    {
#ifdef __AKK0RD_DEBUG_MACRO__
        GLint logLength = 0;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(Program, logLength, &logLength, log);
            if (std::string(log).size() > 0)
            {
                // тут именно вызов функции напрямую, а не через макрос, чтбоы не терять информацию о месте возникнования события
                BWrapper::Log(BWrapper::LogPriority::Debug, File, Function, Line, "Program log [Program=%u]: %s", Program, log);
            }
            free(log);
        }
#endif
    }

    void GLESDriver::PrintShaderLog(GLuint Shader, const char* File, const char* Function, unsigned Line)
    {
#ifdef __AKK0RD_DEBUG_MACRO__
        GLint logLength = 0;
        glGetShaderiv (Shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(Shader, logLength, &logLength, log);
            if (std::string(log).size() > 0)
            {
                // тут именно вызов функции напрямую, а не через макрос, чтбоы не терять информацию о месте возникнования события
                BWrapper::Log(BWrapper::LogPriority::Debug, File, Function, Line, "Shader log [Shader=%u]: %s", Shader, log);
            }
            free(log);
        }
#endif
    };

    void GLESDriver::PrintShaderSource(GLuint Shader)
    {
#ifdef __AKK0RD_DEBUG_MACRO__
        GLint logLength = 0;
        glGetShaderiv(Shader, GL_SHADER_SOURCE_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderSource(Shader, logLength, &logLength, log);
            if (std::string(log).size() > 0)
            {
                logDebug("Shader Source:\n %s\n\n", log);
            }
            free(log);
        }
#endif
    }


GLESDriver& GLESDriver::GetInstance()
{
    static GLESDriver Driver;
    return Driver;
}
