#pragma once
#ifndef __AKK0RD_OPENGLES_DRIVER_H__
#define __AKK0RD_OPENGLES_DRIVER_H__

#include "SDL_opengles2.h"
#include "basewrapper.h"

///================================
// OPENGLES-функции, которых нет в SDL_opengles2.h, но которые так нужны
#define ADDITIONAL_OPENGL_PROCS \
SDL_PROC(void, glBlendFunc, (GLenum, GLenum)) \
SDL_PROC(void, glGetShaderSource, (GLuint, GLsizei, GLsizei*, GLchar *)) \
SDL_PROC(void, glDrawElements, (GLenum, GLsizei, GLenum, const GLvoid *))
///================================

class GLESDriverInstance
{
public:    
#define SDL_PROC(ret,func,params) typedef ret (APIENTRY * func##_fnc)params; func##_fnc func = 0;
#include "../src/render/opengles2/SDL_gles2funcs.h"
	ADDITIONAL_OPENGL_PROCS // включаем дополнительные OPENGLES-функции
#undef SDL_PROC
    void Init()
    {
#define SDL_PROC(ret,func,params) func = (func##_fnc)SDL_GL_GetProcAddress(#func); if(func) logVerbose("GLESDriver:: " #func " pointer was loaded successfully"); else logError("GLESDriver:: " #func " pointer was not loaded");
#include "../src/render/opengles2/SDL_gles2funcs.h"		
	ADDITIONAL_OPENGL_PROCS // включаем дополнительные OPENGLES-функции
#undef SDL_PROC
    }

    bool CheckError(unsigned Line)
    {                 
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
            logError("glGetError() = %u on line = %u, Msg = %s", glErr, Line, ErrorMsg.c_str());
            return true;
        }
        return false;        
    };

    void PrintProgamLog(GLuint Program, unsigned Line)
    {
        GLint logLength;
        glGetProgramiv(Program, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetProgramInfoLog(Program, logLength, &logLength, log);
            if (strlen(log) > 0)
                logDebug("Program log [line=%u]: %s", Line, log);
            free(log);
        }
    }    

    void PrintShaderLog(GLuint Shader, unsigned Line)
    {
        GLint logLength;
        glGetShaderiv (Shader, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(Shader, logLength, &logLength, log);
            if (strlen(log) > 0)
                logDebug("Shader log [line=%u]: %s", Line, log);
            free(log);
        }
    };

    void PrintShaderSource(GLuint Shader)
    {
        GLint logLength;
        glGetShaderiv(Shader, GL_SHADER_SOURCE_LENGTH, &logLength);
        if (logLength > 0)
        { 
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderSource(Shader, logLength, &logLength, log);
            if (strlen(log) > 0)
                logDebug("Shader Source:\n %s\n\n", log);
            free(log);
        }
    }
};

static GLESDriverInstance _GLESDriver;

class GLESDriver
{
    public:
    static GLESDriverInstance* GetInstance()
    {
        return &_GLESDriver;
    }
};

#endif // __AKK0RD_OPENGLES_DRIVER_H__

