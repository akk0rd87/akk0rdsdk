#pragma once
#ifndef __AKK0RD_OPENGL_DRIVER_H__
#define __AKK0RD_OPENGL_DRIVER_H__
#include "basewrapper.h"
#include "SDL_opengl.h"

// Объявляем тип для адреса функции и инициализируем его в ноль
#define DEFINE_GL_FUNC_TYPE(FUNCTION, ...) typedef void (APIENTRY * FUNCTION##_Func)(__VA_ARGS__); FUNCTION##_Func FUNCTION##_FuncPtr = 0;

// Объявляем функцию для получения адреса GL-функции
#define DEFINE_GL_LOAD_ADDRESS(FUNCTION) bool FUNCTION##_load() { FUNCTION##_FuncPtr = (FUNCTION##_Func)SDL_GL_GetProcAddress(#FUNCTION); if(!FUNCTION##_FuncPtr) { BWrapper::Log(BWrapper::LogSeverity::Error, "OpenGLDriver::Init " #FUNCTION " load error %s", SDL_GetError()); return false;} BWrapper::Log(BWrapper::LogSeverity::Verbose, "OpenGLDriver::Init " #FUNCTION " load success %d", FUNCTION##_FuncPtr); return true; } 

// Объявляем функцию вызова GL-функции
#define DEFINE_GL_EXEC(FUNCTION, ...) bool FUNCTION(__VA_ARGS__) { if(!FUNCTION##_FuncPtr) {BWrapper::Log(BWrapper::LogSeverity::Error, "OpenGLDriver:: " #FUNCTION " pointer was not loaded"); return false;} return true; }

// Включаем вместе перечисленные выше макросы
#define DEFINE_GL_FUNCTION(FUNCTION, ...) protected: DEFINE_GL_FUNC_TYPE(FUNCTION, __VA_ARGS__); DEFINE_GL_LOAD_ADDRESS(FUNCTION); //public: DEFINE_GL_EXEC(FUNCTION, __VA_ARGS__);

class GLDriver
{

    DEFINE_GL_FUNCTION(glEnable, GLenum cap);
    DEFINE_GL_FUNCTION(glGenTextures, GLsizei n, GLuint * textures);
    DEFINE_GL_FUNCTION(glBindTexture, GLenum target, GLuint texture);
    DEFINE_GL_FUNCTION(glTexParameteri, GLenum target, GLenum pname, GLint param);
    DEFINE_GL_FUNCTION(glTexImage2D, GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * data);
    DEFINE_GL_FUNCTION(glOrtho, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal);
    DEFINE_GL_FUNCTION(glClear, GLbitfield mask);
    DEFINE_GL_FUNCTION(glClearColor, GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
    DEFINE_GL_FUNCTION(glColor3b, GLbyte red, GLbyte green, GLbyte blue);
    DEFINE_GL_FUNCTION(glBegin, GLenum mode);
    DEFINE_GL_FUNCTION(glEnd, void);
    DEFINE_GL_FUNCTION(glPopMatrix, void);
    DEFINE_GL_FUNCTION(glPushMatrix, void);
    DEFINE_GL_FUNCTION(glTexCoord2f, GLfloat s, GLfloat t);
    DEFINE_GL_FUNCTION(glVertex3f, GLfloat x, GLfloat y, GLfloat z);
    DEFINE_GL_FUNCTION(glColor3f, GLfloat red, GLfloat green, GLfloat blue);    
    //DEFINE_GL_FUNCTION(glEnd, void);
//    glPopMatrix

public: 
    void Init()
    {
        glEnable_load();
        glGenTextures_load();
        glBindTexture_load();
        glTexParameteri_load();
        glTexImage2D_load();
        glOrtho_load();
        glClear_load();
        glClearColor_load();
        glColor3b_load();
        glBegin_load();
        glEnd_load();
        glTexCoord2f_load();
        glVertex3f_load();
        glColor3f_load();
        glPopMatrix_load();
        glPushMatrix_load();
    }

    bool glEnable(GLenum cap)
    {
        if (glEnable_FuncPtr)
        {
            glEnable_FuncPtr(cap);
            return true;
        }
        return false;
    }

    bool glGenTextures(GLsizei n, GLuint * textures)
    {
        if (glGenTextures_FuncPtr)
        {
            glGenTextures_FuncPtr(n, textures);
            return true;
        }
        return false;
    }

    bool glBindTexture(GLenum target, GLuint texture)
    {
        if (glBindTexture_FuncPtr)
        {
            glBindTexture_FuncPtr(target, texture);
            return true;
        }
        return false;
    };

    bool glTexParameteri(GLenum target, GLenum pname, GLint param)
    {
        if (glTexParameteri_FuncPtr)
        {
            glTexParameteri_FuncPtr(target, pname, param);
            return true;
        }
        return false;
    };
    bool glTexImage2D(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * data)
    {
        if (glTexImage2D_FuncPtr)
        {
            glTexImage2D_FuncPtr(target, level, internalFormat, width, height, border, format, type, data);
            return true;
        }
        return false;
    };
    bool glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble nearVal, GLdouble farVal)
    {
        if (glOrtho_FuncPtr)
        {
            glOrtho_FuncPtr(left, right, bottom, top, nearVal, farVal);
            return true;
        }
        return false;
    };
    bool glClear(GLbitfield mask)
    {
        if (glClear_FuncPtr)
        {
            glClear_FuncPtr(mask);
            return true;
        }
        return false;
    };
    bool glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
    {
        if (glClearColor_FuncPtr)
        {
            glClearColor_FuncPtr(red, green, blue, alpha);
            return true;
        }
        return false;
    };
    bool glColor3b(GLbyte red, GLbyte green, GLbyte blue)
    {
        if (glColor3b_FuncPtr)
        {
            glColor3b_FuncPtr(red, green, blue);
            return true;
        }
        return false;
    };
    bool glBegin(GLenum mode)
    {
        if (glBegin_FuncPtr)
        {
            glBegin_FuncPtr(mode);
            return true;
        }
        return false;
    };
    bool glEnd(void)
    {
        if (glEnd_FuncPtr)
        {
            glEnd_FuncPtr();
            return true;
        }
    }
    bool glTexCoord2f(GLfloat s, GLfloat t)
    {
        if (glTexCoord2f_FuncPtr)
        {
            glTexCoord2f_FuncPtr(s, t);
            return true;
        }
        return false;
    };
    bool glVertex3f(GLfloat x, GLfloat y, GLfloat z)
    {
        if (glVertex3f_FuncPtr)
        {
            glVertex3f_FuncPtr(x, y, z);
            return true;
        }
        return false;
    };
    bool glColor3f(GLfloat red, GLfloat green, GLfloat blue)
    {
        if (glColor3f_FuncPtr)
        {
            glColor3f_FuncPtr(red, green, blue);
            return true;
        }
        return false;
    };
    bool glPopMatrix(void)
    {
        if (glPopMatrix_FuncPtr)
        {
            glPopMatrix_FuncPtr();
            return true;
        }
        return false;
    };
    bool glPushMatrix(void)
    {
        if (glPushMatrix_FuncPtr)
        {
            glPushMatrix_FuncPtr();
            return true;
        }
        return false;
    }
};

// Сделать наследование, а потом override методов вызова -- Как вариант

static GLDriver _GLDriver;


class OpenGLDriver
{
public: 
    static GLDriver* GetInstance()
    {
        return &_GLDriver;
    }
};




#endif // __AKK0RD_OPENGL_DRIVER_H__