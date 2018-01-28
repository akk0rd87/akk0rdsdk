#include "SDL.h"
#include "SDL_image.h"
#include "openglesdriver.h"
#include "basewrapper.h"

// Shader sources
const GLchar* vertexSource =
"attribute vec4 position;    \n"
"void main()                  \n"
"{                            \n"
"   gl_Position = vec4(position.xyz, 1.0);  \n"
"}                            \n";
const GLchar* fragmentSource =
"precision mediump float;\n"
"void main()                                  \n"
"{                                            \n"
"  gl_FragColor = vec4 (1.0, 1.0, 1.0, 1.0 );\n"
"}                                            \n";

int main(int argc, char *argv[])
{       
    BWrapper::SetLogPriority(BWrapper::LogPriority::Verbose);
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        BWrapper::Log(BWrapper::LogPriority::Error, "SDL Init error %s", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    auto window = SDL_CreateWindow("SDL2 OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        BWrapper::Log(BWrapper::LogPriority::Error, "Window create error %s", SDL_GetError());
        return 0;
    }

    auto context = SDL_GL_CreateContext(window);
    if (!context)
    {
        BWrapper::Log(BWrapper::LogPriority::Error, "Context create error %s", SDL_GetError());
        return 0;
    }

    auto Driver = GLESDriver::GetInstance();
    Driver->Init();

    /*
    SDL_Surface* image = IMG_Load("../assets/2.png");
    if (!image)
    {
        BWrapper::Log(BWrapper::LogPriority::Error, "Image create error %s", SDL_GetError());
        return 0;
    } 
    */

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetSwapInterval(0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    GLuint vbo;
    Driver->glGenBuffers(1, &vbo);

    GLfloat vertices[] = { -0.5f, -0.5f,
                           0.5f, -0.5f, 
                           -0.5f, 0.5f,
                           0.5f, 0.5f                          
                         };

    Driver->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    Driver->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create and compile the vertex shader
    GLuint vertexShader = Driver->glCreateShader(GL_VERTEX_SHADER);
    Driver->glShaderSource(vertexShader, 1, &vertexSource, NULL);
    Driver->glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = Driver->glCreateShader(GL_FRAGMENT_SHADER);
    Driver->glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    Driver->glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = Driver->glCreateProgram();
    Driver->glAttachShader(shaderProgram, vertexShader);
    Driver->glAttachShader(shaderProgram, fragmentShader);
    // glBindFragDataLocation(shaderProgram, 0, "outColor");
    Driver->glLinkProgram(shaderProgram); Driver->CheckError(__LINE__);
    Driver->glUseProgram(shaderProgram); Driver->CheckError(__LINE__);

    // Specify the layout of the vertex data
    GLint posAttrib = Driver->glGetAttribLocation(shaderProgram, "position");
    Driver->glEnableVertexAttribArray(posAttrib);
    Driver->glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    while (1)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT) goto end;
        }

        // Clear the screen to black
        Driver->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        Driver->glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle from the 3 vertices
        Driver->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        SDL_GL_SwapWindow(window);
    };

    SDL_Delay(3000);

    end:
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    

    return 0;
}