#include "SDL.h"
#include "SDL_image.h"
#include "openglesdriver.h"

const bool DrawWithRender = true;
const bool DrawSDF        = true;

const GLchar* vertexSource =
"varying lowp vec4 result_color; \n\
varying mediump vec2 result_uv; \n\
uniform lowp vec4 sdf_outline_color; \n\
uniform mediump vec4 sdf_params; \n\
uniform mediump mat4 mat; \n\
uniform vec4 font_color; \n\
attribute vec2 position; \n\
attribute vec2 uv; \n\
void main()  \n\
{\n\
    gl_Position = mat * vec4(position, 0.0, 1.0);  \n\
    result_color = font_color; \n\
    result_uv = uv; \n\
}\n";


const GLchar* fragmentSource =
"#define SDF_OUTLINE \n\
varying lowp vec4 result_color; \n\
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



static const GLfloat squareVertices1[] = {
    -0.8f, -1.0f,
    0.8f, -1.0f, 
    -0.8f, 1.0f, 
    0.8f, 1.0f
};


static const GLfloat squareVertices2[] = {
    -0.8f, -1.0f,
    0.8f, -1.0f,
    -0.8f, 0.0f,
    0.8f, 0.0f
};

const float r = (float)39 / 255;
const float g = (float)87 / 255;
const float b = (float)220 / 255;
/*
static const GLfloat Color[] = 
{ r, g, b, 1.0f, 
r, g, b, 1.0f,
r, g, b, 1.0f,
r, g, b, 1.0f,
};
*/

GLfloat uv_const = 0.5f;


static const GLfloat UV[] =
{ 0.0f, 1.0f,
1.0f, 1.0f,
0.0f, 0.0f,
1.0f, 0.0f
};


/*
static const GLfloat UV[] =
{ 0.0f, 0.1f,
0.1f, 0.1f,
0.0f, 0.0f,
0.1f, 0.0f
};
*/

enum {
    ATTRIB_POSITION = 10, // Начинаем не с нуля, чтобы индексы не пересеклись с другими программами
    //ATTRIB_COLOR,
    ATTRIB_UV,
    NUM_ATTRIBUTES
};

static const GLfloat Mat[] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

/*
static const GLfloat Mat[] =
{
    0.00208333344f, 0.0f, 0.0f, 0.0f,
    0.0f, -0.00312500005, 0.0f, 0.0f,
    0.0f, 0.0f, 0.000100001998f, 0.0f,
    -1.00000000f, 1.0f, -0.0000200004015f, 1.0f,
};
*/

int main(int argc, char *argv[])
{
    GLint oldProgramId;
    GLuint texture;

    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
    if (SDL_Init(SDL_INIT_EVERYTHING /*| SDL_VIDEO_OPENGL*/) < 0)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "SDL Init error %s", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");    

    auto window = SDL_CreateWindow("SDL2 SDF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Window create error %s", SDL_GetError());
        return 0;
    }    

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

    auto Renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!Renderer)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Renderer create error %s", SDL_GetError());
        return 0;
    }    

    auto Context = SDL_GL_GetCurrentContext();
    if (!Context)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Context get error %s", SDL_GetError());
        return 0;
    }

    auto Driver = GLESDriver::GetInstance();
    Driver->Init();

    // Load SDF Font Atlas image
    SDL_Surface* image = IMG_Load("assets/sdf/font_0.png");
    //SDL_Surface* image = IMG_Load("assets/sdf/calibri_new.png");    
    if (!image)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Image create error %s", SDL_GetError());
        return 0;
    }
    
    auto Img1 = IMG_Load("assets/img/1.png");
    if (!Img1)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Image 1.png load error %s", SDL_GetError());
        return 0;
    }
    auto tex1 = SDL_CreateTextureFromSurface(Renderer, Img1);
    SDL_FreeSurface(Img1);

    auto Img2 = IMG_Load("assets/img/2.png");
    if (!Img2)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Image 2.png load error %s", SDL_GetError());
        return 0;
    }
    auto tex2 = SDL_CreateTextureFromSurface(Renderer, Img2);
    SDL_FreeSurface(Img2);

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
    //Driver->glBindAttribLocation(shaderProgram, ATTRIB_COLOR, "color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    Driver->glBindAttribLocation(shaderProgram, ATTRIB_UV, "uv"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
        
    Driver->glLinkProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

    auto sdf_outline_color = Driver->glGetUniformLocation(shaderProgram, "sdf_outline_color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto sdf_params        = Driver->glGetUniformLocation(shaderProgram, "sdf_params"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto mat               = Driver->glGetUniformLocation(shaderProgram, "mat"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto base_texture      = Driver->glGetUniformLocation(shaderProgram, "base_texture"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    auto font_color = Driver->glGetUniformLocation(shaderProgram, "font_color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
    
    Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
    Driver->glUseProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

    if (sdf_outline_color >= 0) Driver->glUniform4f(sdf_outline_color, 1.0f, 1.0f, 1.0f, 1.0f); Driver->CheckError(__LINE__);
    Driver->glUniform4f(sdf_params, 0.5f, 11.0f, 0.4f, 11.0f); Driver->CheckError(__LINE__);
    //Driver->glUniform4f(sdf_params, 0.490000010f, 69.7942657f, 0.340000004, 69.7942657f); Driver->CheckError(__LINE__);
    Driver->glUniformMatrix4fv(mat, 1, GL_FALSE, Mat);    Driver->CheckError(__LINE__);    
    Driver->glUniform1i(base_texture, 0);

    Driver->glUseProgram(oldProgramId);

    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "sdf_outline_color = %d; sdf_params = %d; mat = %d, base_texture = %d, font_color = %d", sdf_outline_color, sdf_params, mat, base_texture, font_color);

    SDL_SetRenderDrawColor(Renderer, 100, 100, 100, 100);
    
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
            SDL_RenderClear(Renderer);
            
            if (DrawWithRender)
            {
                SDL_Rect r;
                r.x = r.y = 10; r.w = r.h = 100;
                SDL_RenderCopy(Renderer, tex1, nullptr, &r);

                SDL_GetWindowSize(window, &r.x, nullptr);
                r.y = 10; r.w = r.h = 100;
                r.x = r.x - r.w - 10;
                SDL_RenderCopy(Renderer, tex2, nullptr, &r);                
                //SDL_RenderPresent(Renderer);
                //SDL_GL_SwapWindow(window);
            }
            //Renderer

            ///////////////////////////////////////////////////////
            if (DrawSDF)
            {
                //Driver->glClearColor(1.0, 1.0, 1.0, 1.0);
                //Driver->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
                Driver->glUseProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);                

                Driver->glBindTexture(GL_TEXTURE_2D, texture); Driver->CheckError(__LINE__);                
                
                Driver->glEnableVertexAttribArray(ATTRIB_POSITION); Driver->CheckError(__LINE__);
                Driver->glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, 0, 0, squareVertices1); Driver->CheckError(__LINE__);
                
                Driver->glEnableVertexAttribArray(ATTRIB_UV); Driver->CheckError(__LINE__);
                Driver->glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 0, 0, UV); Driver->CheckError(__LINE__);
                
                //Driver->glEnableVertexAttribArray(ATTRIB_COLOR); Driver->CheckError(__LINE__);
                //Driver->glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, 0, 0, Color); Driver->CheckError(__LINE__);                
                Driver->glUniform4f(font_color, r, g, b, 1.0f); Driver->CheckError(__LINE__);
                Driver->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); Driver->CheckError(__LINE__);






                Driver->glEnableVertexAttribArray(ATTRIB_POSITION); Driver->CheckError(__LINE__);
                Driver->glVertexAttribPointer(ATTRIB_POSITION, 2, GL_FLOAT, 0, 0, squareVertices2); Driver->CheckError(__LINE__);

                //Driver->glEnableVertexAttribArray(ATTRIB_UV); Driver->CheckError(__LINE__);
                //Driver->glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 0, 0, UV); Driver->CheckError(__LINE__);

                //Driver->glEnableVertexAttribArray(ATTRIB_COLOR); Driver->CheckError(__LINE__);
                //Driver->glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, 0, 0, Color); Driver->CheckError(__LINE__);
                Driver->glUniform4f(font_color, 1.0f, 1.0f, 0.4f, 1.0f); Driver->CheckError(__LINE__);
                Driver->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); Driver->CheckError(__LINE__);






                Driver->glDisableVertexAttribArray(ATTRIB_POSITION);
                Driver->glDisableVertexAttribArray(ATTRIB_UV);
                //Driver->glDisableVertexAttribArray(ATTRIB_COLOR);

                Driver->glUseProgram(oldProgramId); Driver->CheckError(__LINE__);                
            }
            //SDL_GL_SwapWindow(window);
            ///////////////////////////////////////////////////////     
            //SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "Drawing");
            SDL_RenderPresent(Renderer);
        }
    };

end:    
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(Renderer);
    SDL_Quit();

    return 0;
}