#include "SDL.h"
#include "SDL_image.h"
#include "openglesdriver.h"
#include "basewrapper.h"

GLuint texture;
AkkordTexture AkTexture2;

const GLchar* vertexSource =
"attribute vec4 position;\n"
"attribute vec4 inputTextureCoordinate;\n"
"varying vec2 textureCoordinate;\n"
"void main()\n"
"{    gl_Position = position;\n"
"     textureCoordinate = inputTextureCoordinate.xy;\n"
"} \n";
const GLchar* fragmentSource =
"precision mediump float;\n"
"varying vec2 textureCoordinate;\n"
"uniform sampler2D videoFrame;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(videoFrame, textureCoordinate);\n"
"}\n";

static const GLfloat squareVertices[] = {
    -0.5f, -0.5f,
    0.5f, -0.5f,
    -0.5f, 0.5f,
    0.5f, 0.5f,
};

static const GLfloat textureVertices[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, };

enum {
    ATTRIB_VERTEX,
    ATTRIB_TEXTUREPOSITON,
    NUM_ATTRIBUTES
};

AkkordTexture Text;

int main(int argc, char *argv[])
{
    BWrapper::SetLogPriority(BWrapper::LogPriority::Verbose);
    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_VIDEO_OPENGL) < 0)
    {
        //BWrapper::Log(BWrapper::LogPriority::Error, "SDL Init error %s", SDL_GetError());
		logError("SDL Init error %s", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    auto window = SDL_CreateWindow("SDL2 OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        //BWrapper::Log(BWrapper::LogPriority::Error, "Window create error %s", SDL_GetError());
		logError("Window create error %s", SDL_GetError());
        return 0;
    }
    BWrapper::SetActiveWindow(window);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

    auto Renderer = BWrapper::CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!Renderer)
    {
        //BWrapper::Log(BWrapper::LogPriority::Error, "Renderer create error %s", SDL_GetError());
		logError("Renderer create error %s", SDL_GetError());
        return 0;
    }
    BWrapper::SetActiveRenderer(Renderer);

    auto Driver = GLESDriver::GetInstance();
    Driver->Init();

    SDL_Surface* image = IMG_Load("assets/spritesheet.png");
    if (!image)
    {
        //BWrapper::Log(BWrapper::LogPriority::Error, "Image create error %s", SDL_GetError());
		logError("Image create error %s", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create and compile the vertex shader
    GLuint vertexShader = Driver->glCreateShader(GL_VERTEX_SHADER); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);
    Driver->glShaderSource(vertexShader, 1, &vertexSource, NULL); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);
    Driver->glCompileShader(vertexShader); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);

    // Create and compile the fragment shader
    GLuint fragmentShader = Driver->glCreateShader(GL_FRAGMENT_SHADER); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);
    Driver->glShaderSource(fragmentShader, 1, &fragmentSource, NULL); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);
    Driver->glCompileShader(fragmentShader); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = Driver->glCreateProgram(); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    Driver->glAttachShader(shaderProgram, vertexShader); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    Driver->glAttachShader(shaderProgram, fragmentShader); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    
    Driver->glBindAttribLocation(shaderProgram, ATTRIB_VERTEX, "position"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    Driver->glBindAttribLocation(shaderProgram, ATTRIB_TEXTUREPOSITON, "inputTextureCoordinate"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
        
    Driver->glLinkProgram(shaderProgram); Driver->CheckError(__LINE__);

    auto Frame = Driver->glGetUniformLocation(shaderProgram, "videoFrame"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto Color = Driver->glGetUniformLocation(shaderProgram, "inputColor"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto threshold = Driver->glGetUniformLocation(shaderProgram, "threshold"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

    Driver->glUniform1i(Frame, 0);
    logDebug("My Shader Program = %u", shaderProgram);

    Text.LoadFromFile("img/1.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);
    AkTexture2.LoadFromFile("2.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);

    BWrapper::SetCurrentColor(AkkordColor(255, 255, 255));
    //Driver->glActiveTexture(GL_TEXTURE0); Driver->CheckError(__LINE__);
    Driver->glGenTextures(1, &texture);
    logDebug("Texture ID = %u", texture);
    Driver->glBindTexture(GL_TEXTURE_2D, texture); Driver->CheckError(__LINE__);
    Driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); Driver->CheckError(__LINE__);
    Driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); Driver->CheckError(__LINE__);
    Driver->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels); Driver->CheckError(__LINE__);

    int i = 0; 
    while (1)
    {
        SDL_Event e;
        while (SDL_WaitEvent(&e))
        {
            if (e.type == SDL_QUIT) goto end;
            BWrapper::ClearRenderer();            
            Text.Draw(AkkordRect(10, 10, 100, 100));
            AkTexture2.Draw(AkkordRect(BWrapper::GetScreenWidth() - 110, 10, 100, 100));
            
            ///////////////////////////////////////////////////////
            if(1){
                GLint oldProgramId;
                Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);

                Driver->glUseProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
                //BWrapper::Log(BWrapper::LogPriority::Debug, "My Shader Program = %u", shaderProgram);

                Driver->glBindTexture(GL_TEXTURE_2D, texture); Driver->CheckError(__LINE__); 
                Driver->glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
                Driver->glEnableVertexAttribArray(ATTRIB_VERTEX);
                Driver->glVertexAttribPointer(ATTRIB_TEXTUREPOSITON, 2, GL_FLOAT, 0, 0, textureVertices);
                Driver->glEnableVertexAttribArray(ATTRIB_TEXTUREPOSITON);
                Driver->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                Driver->glUseProgram(oldProgramId);
            }
            ///////////////////////////////////////////////////////
            BWrapper::RefreshRenderer();
            
        }
    };

end:    
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(Renderer);
    SDL_Quit();

    return 0;
}