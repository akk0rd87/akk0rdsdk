#ifndef __AKK0RD_OPENGLES_DRIVER_H__
#define __AKK0RD_OPENGLES_DRIVER_H__

#include "SDL_opengles2.h"
#include "basewrapper.h"

// выборочный список OPENGLES2-функций из файла "../src/render/opengles2/SDL_gles2funcs.h"
// некоторых нижеперечисленных ф-ий нет в файле "../src/render/opengles2/SDL_gles2funcs.h"
#if __NACL__ || __ANDROID__
#define DEFINE_glShaderSource SDL_PROC(void, glShaderSource, (GLuint, GLsizei, const GLchar **, const GLint *))
#else
#define DEFINE_glShaderSource SDL_PROC(void, glShaderSource, (GLuint, GLsizei, const GLchar* const*, const GLint *))
#endif

#define OPENGLES2_FUCNTION_LIST \
DEFINE_glShaderSource \
SDL_PROC(void, glBlendFunc, (GLenum, GLenum)) \
SDL_PROC(void, glGetShaderSource, (GLuint, GLsizei, GLsizei*, GLchar *)) \
SDL_PROC(void, glDrawElements, (GLenum, GLsizei, GLenum, const GLvoid *)) \
SDL_PROC(GLuint, glCreateShader, (GLenum)) \
SDL_PROC(void, glCompileShader, (GLuint)) \
SDL_PROC(GLuint, glCreateProgram, (void)) \
SDL_PROC(void, glAttachShader, (GLuint, GLuint)) \
SDL_PROC(void, glBindAttribLocation, (GLuint, GLuint, const char *)) \
SDL_PROC(void, glLinkProgram, (GLuint)) \
SDL_PROC(void, glGetIntegerv, (GLenum, GLint *)) \
SDL_PROC(void, glUseProgram, (GLuint)) \
SDL_PROC(GLint, glGetUniformLocation, (GLuint, const char *)) \
SDL_PROC(void, glUniformMatrix4fv, (GLint, GLsizei, GLboolean, const GLfloat *)) \
SDL_PROC(void, glUniform1i, (GLint, GLint)) \
SDL_PROC(void, glGenTextures, (GLsizei, GLuint *)) \
SDL_PROC(void, glBindTexture, (GLenum, GLuint)) \
SDL_PROC(void, glTexParameteri, (GLenum, GLenum, GLint)) \
SDL_PROC(void, glTexImage2D, (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void *)) \
SDL_PROC(void, glEnableVertexAttribArray, (GLuint)) \
SDL_PROC(void, glVertexAttribPointer, (GLuint, GLint, GLenum, GLboolean, GLsizei, const void *)) \
SDL_PROC(void, glUniform1f, (GLint, GLfloat)) \
SDL_PROC(void, glUniform4f, (GLint, GLfloat, GLfloat, GLfloat, GLfloat)) \
SDL_PROC(void, glDisableVertexAttribArray, (GLuint)) \
SDL_PROC(GLenum, glGetError, (void)) \
SDL_PROC(void, glGetProgramInfoLog, (GLuint, GLsizei, GLsizei*, GLchar*)) \
SDL_PROC(void, glGetProgramiv, (GLuint, GLenum, GLint *)) \
SDL_PROC(void, glGetShaderInfoLog, (GLuint, GLsizei, GLsizei *, char *)) \
SDL_PROC(void, glGetShaderiv, (GLuint, GLenum, GLint *)) \
SDL_PROC(void, glGetVertexAttribiv, (GLuint, GLenum, GLint *))

#ifdef __AKK0RD_DEBUG_MACRO__
#define CheckGLESError()            GLESDriver::GetInstance().CheckError    (         __FILE__, __FUNCTION__, __LINE__)
#define PrintGLESProgamLog(Program) GLESDriver::GetInstance().PrintProgamLog(Program, __FILE__, __FUNCTION__, __LINE__)
#define PrintGLESShaderLog(Shader)  GLESDriver::GetInstance().PrintShaderLog(Shader , __FILE__, __FUNCTION__, __LINE__)
#else
#define CheckGLESError()
#define PrintGLESProgamLog(Program)
#define PrintGLESShaderLog(Shader)
#endif

class GLESDriver
{
public:    
#define SDL_PROC(ret,func,params) typedef ret (APIENTRY * func##_fnc)params; func##_fnc func = 0;
//#include "../src/render/opengles2/SDL_gles2funcs.h"
    OPENGLES2_FUCNTION_LIST // включаем дополнительные OPENGLES-функции
#undef SDL_PROC
    void Init();
    bool CheckError(const char* File, const char* Function, unsigned Line);
    void PrintProgamLog(GLuint Program, const char* File, const char* Function, unsigned Line);
    void PrintShaderLog(GLuint Shader, const char* File, const char* Function, unsigned Line);
    void PrintShaderSource(GLuint Shader);
    
    static GLESDriver& GetInstance();

	GLESDriver() {};

	//Запрещаем создавать экземпляр класса GLESDriver	
	GLESDriver(GLESDriver& rhs) = delete; // Копирующий: конструктор
	GLESDriver(GLESDriver&& rhs) = delete; // Перемещающий: конструктор	
	GLESDriver& operator= (GLESDriver&& rhs) = delete; // Оператор перемещающего присваивания
};

#endif // __AKK0RD_OPENGLES_DRIVER_H__

