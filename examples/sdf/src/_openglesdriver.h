#pragma once
#ifndef __AKK0RD_OPENGLES_DRIVER_H__
#define __AKK0RD_OPENGLES_DRIVER_H__

#include <string>
#include "SDL_opengles2.h"

class GLESDriverInstance
{
public:    
#define SDL_PROC(ret,func,params) typedef ret (APIENTRY * func##_fnc)params; func##_fnc func = 0;
#include "../src/render/opengles2/SDL_gles2funcs.h"
    SDL_PROC(void, glBlendFunc, (GLenum, GLenum))
    SDL_PROC(void, glGetShaderSource, (GLuint, GLsizei, GLsizei*, GLchar *))
#undef SDL_PROC
    void Init()
    {
#define SDL_PROC(ret,func,params) func = (func##_fnc)SDL_GL_GetProcAddress(#func); if(func) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE, "GLESDriver:: " #func " pointer was loaded successfully"); else SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "GLESDriver:: " #func " pointer was not loaded");
#include "../src/render/opengles2/SDL_gles2funcs.h"
        SDL_PROC(void, glBlendFunc, (GLenum, GLenum))
        SDL_PROC(void, glGetShaderSource, (GLuint, GLsizei, GLsizei*, GLchar *))
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
            SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "glGetError() = %u on line = %u, Msg = %s", glErr, Line, ErrorMsg.c_str());
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
                SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Program log [line=%u]: %s", Line, log);
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
                SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Shader log [line=%u]: %s", Line, log);
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
                SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Shader Source:\n %s\n\n", log);
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

