#include "SDL.h"
#include "SDL_image.h"
#include "openglesdriver.h"
#include "basewrapper.h"

GLuint texture;
AkkordTexture AkTexture2;


const GLchar* vertexSource =
"varying lowp vec4 result_color; \n\
varying mediump vec2 result_uv; \n\
uniform lowp vec4 sdf_outline_color; \n\
uniform mediump vec4 sdf_params; \n\
uniform mediump mat4 mat; \n\
attribute vec3 position; \n\
attribute vec4 color; \n\
attribute vec2 uv; \n\
void main()  \n\
{\n\
    gl_Position = mat * vec4(position, 1.0);  \n\
    result_color = color; \n\
    result_uv = uv; \n\
}\n";


const GLchar* fragmentSource =
"varying lowp vec4 result_color; \n\
varying mediump vec2 result_uv; \n\
uniform lowp vec4 sdf_outline_color; \n\
uniform mediump vec4 sdf_params; \n\
uniform lowp sampler2D base_texture; \n\
lowp vec4 get_base_sdf() \n\
{\n\
    lowp float tx = texture2D(base_texture, result_uv).r; \n\
#ifdef SDF_OUTLINE \n\
    lowp float b =   min((tx - sdf_params.z) * sdf_params.w, 1.0); \n\
    lowp float a = clamp((tx - sdf_params.x) * sdf_params.y, 0.0, 1.0); \n\
    lowp vec4 res = (sdf_outline_color + (result_color - sdf_outline_color)*a) * b; \n\
#else \n\
    lowp float a = min((tx - sdf_params.x) * sdf_params.y, 1.0); \n\
    lowp vec4 res = result_color * a; \n\
#endif \n\
    return res;\n\
} \n\
void main() \n\
{	\n\
	gl_FragColor = get_base_sdf();\n\
}\n";


/*
// Shaders from https://habrahabr.ru/post/282191/
const GLchar* vertexSource =
"precision mediump float; \
attribute mediump vec2 Vertex; \
uniform highp mat4 MVP; \
uniform mediump vec2 cords[4]; \
varying mediump vec2 outTexCord; \
void main(){ \
    outTexCord = Vertex*cords[3] + cords[2]; \
    gl_Position = MVP * vec4(Vertex*cords[1] + cords[0], 0.0, 1.0); \
}";

const GLchar* fragmentSource =
"precision mediump float; \
varying mediump vec2 outTexCord; \
uniform lowp sampler2D tex0; \
uniform mediump vec4 color; \
uniform mediump vec2 params; \
void main(void){ \
    float tx = texture2D(tex0, outTexCord).r; \
    float a = min((tx - params.x)*params.y, 1.0); \
    gl_FragColor = vec4(color.rgb, a*color.a); \
}";
*/
static const GLfloat squareVertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.0f
};

static const GLfloat textureVertices[] = {
    0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
};

static const GLfloat Color[] = 
{ 1.0f, 1.0f, 0.0f, 1.0f, 
  1.0f, 1.0f, 0.0f, 1.0f,
  1.0f, 1.0f, 0.0f, 1.0f,
  1.0f, 1.0f, 0.0f, 1.0f
};


GLfloat uv_const = 0.5f;
static const GLfloat UV[] =
{ uv_const, uv_const,
uv_const, uv_const,
uv_const, uv_const,
uv_const, uv_const
};

enum {
    ATTRIB_POSITION,
    ATTRIB_COLOR,
    ATTRIB_UV,
    NUM_ATTRIBUTES
};

AkkordTexture Text;

/*
static const GLfloat Mat[] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};
*/

static const GLfloat Mat[] =
{
    0.00208333344f, 0.0f, 0.0f, 0.0f,
    0.0f, -0.00312500005, 0.0f, 0.0f,
    0.0f, 0.0f, 0.000100001998f, 0.0f,
    -1.00000000f, 1.0f, -0.0000200004015f, 1.0f,
};

// -2.00004015e-005

int main(int argc, char *argv[])
{
    GLint oldProgramId;
    BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);
    if (SDL_Init(SDL_INIT_EVERYTHING | SDL_VIDEO_OPENGL) < 0)
    {
        logError("SDL Init error %s", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");    

    auto window = SDL_CreateWindow("SDL2 OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        logError("Window create error %s", SDL_GetError());
        return 0;
    }
    BWrapper::SetActiveWindow(window);

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

    auto Renderer = BWrapper::CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!Renderer)
    {
        logError("Renderer create error %s", SDL_GetError());
        return 0;
    }
    BWrapper::SetActiveRenderer(Renderer);

    auto Driver = GLESDriver::GetInstance();
    Driver->Init();

    SDL_Surface* image = IMG_Load("assets/sdf/font_0.png");
    //SDL_Surface* image = IMG_Load("assets/img/1.png");
    if (!image)
    {
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
    
    Driver->glBindAttribLocation(shaderProgram, ATTRIB_POSITION, "position"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    Driver->glBindAttribLocation(shaderProgram, ATTRIB_COLOR, "color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    Driver->glBindAttribLocation(shaderProgram, ATTRIB_UV, "uv"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
        
    Driver->glLinkProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    logDebug("My Shader Program = %u", shaderProgram);

    /*
    logDebug("Vertex");
    Driver->PrintShaderSource(vertexShader);
    logDebug("Fragment");
    Driver->PrintShaderSource(fragmentShader);
    */

    auto sdf_outline_color = Driver->glGetUniformLocation(shaderProgram, "sdf_outline_color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto sdf_params        = Driver->glGetUniformLocation(shaderProgram, "sdf_params"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto mat               = Driver->glGetUniformLocation(shaderProgram, "mat"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto base_texture      = Driver->glGetUniformLocation(shaderProgram, "base_texture"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    
    Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
    Driver->glUseProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

    if (sdf_outline_color >= 0) Driver->glUniform4f(sdf_outline_color, 1.0f, 1.0f, 1.0f, 1.0f); Driver->CheckError(__LINE__);
    Driver->glUniform4f(sdf_params, 0.5f, 11.0f, 0.4f, 11.0f); Driver->CheckError(__LINE__);
    Driver->glUniformMatrix4fv(mat, 1, GL_FALSE, Mat);    Driver->CheckError(__LINE__);    
    Driver->glUniform1i(base_texture, 0);

    Driver->glUseProgram(oldProgramId);

    logDebug("sdf_outline_color = %d; sdf_params = %d; mat = %d, base_texture = %d", sdf_outline_color, sdf_params, mat, base_texture);

    Text.LoadFromFile("img/1.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);
    AkTexture2.LoadFromFile("2.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);

    BWrapper::SetCurrentColor(AkkordColor(255, 255, 255));    
    
    Driver->glGenTextures(1, &texture); Driver->CheckError(__LINE__);

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
                Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
                Driver->glUseProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
                //logDebug("My Shader Program = %u", shaderProgram);

                Driver->glBindTexture(GL_TEXTURE_2D, texture); Driver->CheckError(__LINE__);                

                //Driver->glUniform1i(Frame, 0);
                Driver->glEnableVertexAttribArray(ATTRIB_POSITION); Driver->CheckError(__LINE__);
                Driver->glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, 0, 0, squareVertices); Driver->CheckError(__LINE__);
                
                Driver->glEnableVertexAttribArray(ATTRIB_UV); Driver->CheckError(__LINE__);
                Driver->glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 0, 0, UV); Driver->CheckError(__LINE__);
                
                Driver->glEnableVertexAttribArray(ATTRIB_COLOR); Driver->CheckError(__LINE__);
                Driver->glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, 0, 0, Color); Driver->CheckError(__LINE__);
                
                
                Driver->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); Driver->CheckError(__LINE__);

                //Driver->glDisableVertexAttribArray(ATTRIB_POSITION);
                //Driver->glDisableVertexAttribArray(ATTRIB_UV);
                //Driver->glDisableVertexAttribArray(ATTRIB_COLOR);

                Driver->glUseProgram(oldProgramId); Driver->CheckError(__LINE__);
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