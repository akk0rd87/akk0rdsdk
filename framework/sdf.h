#pragma once
#ifndef __AKK0RD_SDFFONT_H__
#define __AKK0RD_SDFFONT_H__

// https://github.com/libgdx/libgdx/wiki/distance-field-fonts

/*
Call stack:
1) SDFFontBuffer -> SDFFont -> SDFTexture
2) SDFAtlas -> SDFTexture
*/

#ifdef __CODEBLOCKS
typedef void             GLvoid;
typedef char             GLchar;
typedef unsigned int     GLenum;
typedef unsigned char    GLboolean;
typedef unsigned int     GLbitfield;
//typedef khronos_int8_t   GLbyte;
typedef short            GLshort;
typedef int              GLint;
typedef int              GLsizei;
//typedef khronos_uint8_t  GLubyte;
typedef unsigned short   GLushort;
typedef unsigned int     GLuint;
typedef /*khronos_float_t*/ float  GLfloat;
//typedef khronos_float_t  GLclampf;
//typedef khronos_int32_t  GLfixed;
#endif

#include <map>
#include "basewrapper.h"
#include "SDL_image.h"

/*
https://github.com/libgdx/libgdx/wiki/Hiero
"java -cp gdx.jar;gdx-natives.jar;gdx-backend-lwjgl.jar;gdx-backend-lwjgl-natives.jar;extensions\gdx-freetype\gdx-freetype.jar;extensions\gdx-freetype\gdx-freetype-natives.jar;extensions\gdx-tools\gdx-tools.jar com.badlogic.gdx.tools.hiero.Hiero"
*/

// https://habrahabr.ru/post/282191/
static inline unsigned int UTF2Unicode(const /*unsigned*/ char *txt, unsigned int &i){
    unsigned int a = txt[i++];
    if ((a & 0x80) == 0)return a;
    if ((a & 0xE0) == 0xC0){
        a = (a & 0x1F) << 6;
        a |= txt[i++] & 0x3F;
    }
    else if ((a & 0xF0) == 0xE0){
        a = (a & 0xF) << 12;
        a |= (txt[i++] & 0x3F) << 6;
        a |= txt[i++] & 0x3F;
    }
    else if ((a & 0xF8) == 0xF0){
        a = (a & 0x7) << 18;
        a |= (a & 0x3F) << 12;
        a |= (txt[i++] & 0x3F) << 6;
        a |= txt[i++] & 0x3F;
    }
    return a;
};

#ifndef __CODEBLOCKS
#include "openglesdriver.h"

static const GLchar* SDF_vertexSource =
"varying highp vec4 result_color; \n\
varying highp vec2 result_uv; \n\
uniform highp mat4 mat; \n\
uniform vec4 font_color; \n\
uniform highp float smooth_param; \n\
attribute vec2 position; \n\
attribute vec2 uv; \n\
varying highp float SmoothDistance; \n\
varying highp float center; \n\
#ifdef SDF_OUTLINE \n\
    uniform highp vec4  sdf_outline_color; \n\
    uniform highp float border; \n\
    varying highp vec4  outBorderCol; \n\
    varying highp float	outlineMaxValue0; \n\
    varying highp float	outlineMaxValue1; \n\
#endif \n\
void main()  \n\
{\n\
    gl_Position = mat * vec4(position, 0.0, 1.0);  \n\
    result_color = font_color; \n\
    result_uv = uv; \n\
    SmoothDistance = smooth_param; \n\
#ifdef SDF_OUTLINE \n\
    outBorderCol = sdf_outline_color; \n\
    outlineMaxValue0 = 0.5 - border; \n\
    outlineMaxValue1 = 0.5 + border; \n\
    center = outlineMaxValue0 - border; \n\
#else \n\
    center = 0.5; \n\
#endif \n\
}\n";

static const GLchar* SDF_fragmentSource =
"varying highp vec4 result_color; \n\
varying highp vec2 result_uv; \n\
uniform highp vec4 sdf_outline_color; \n\
uniform highp sampler2D base_texture; \n\
varying highp float SmoothDistance; \n\
varying highp float   outlineMaxValue0; \n\
varying highp float   outlineMaxValue1; \n\
varying highp float   center; \n\
\n\
varying highp vec4       outBorderCol; \n\
\n\
highp float my_smoothstep(highp float edge0, highp float edge1, highp float x) { \n\
x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); \n\
return x * x * (3.0 - 2.0 * x); \n\
}\n\
void main() \n\
{  \n\
    highp float distAlpha = texture2D(base_texture, result_uv).a; \n\
    highp vec4 rgba = result_color; \n\
#ifdef SDF_OUTLINE \n\
    rgba.xyzw = mix(rgba.xyzw, outBorderCol.xyzw, my_smoothstep(outlineMaxValue1, outlineMaxValue0, distAlpha)); \n\
#endif \n\
    rgba.a *= my_smoothstep(center - SmoothDistance, center + SmoothDistance, distAlpha); \n\
    gl_FragColor = rgba; \n\
}\n";

static const GLfloat SDF_Mat[] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

struct ShaderProgramStruct
{
    GLuint shaderProgram;
    GLint sdf_outline_color, font_color, smooth, border;
};

class SDFProgram
{
    ShaderProgramStruct ShaderProgram, ShaderProgramOutline;

    bool CompileProgram(ShaderProgramStruct* Program, const char* VertextShader, const char* FragmentShader)
    {
        GLint oldProgramId;

        auto Driver = GLESDriver::GetInstance();        

        // Create and compile the fragment shader
        GLuint vertexShader = Driver->glCreateShader(GL_VERTEX_SHADER); CheckGLESError(); PrintGLESShaderLog(vertexShader);
        Driver->glShaderSource(vertexShader, 1, &VertextShader, NULL); CheckGLESError(); PrintGLESShaderLog(vertexShader);
        Driver->glCompileShader(vertexShader); CheckGLESError(); PrintGLESShaderLog(vertexShader);

        // Create and compile the fragment shader
        GLuint fragmentShader = Driver->glCreateShader(GL_FRAGMENT_SHADER); CheckGLESError(); PrintGLESShaderLog(fragmentShader);
        Driver->glShaderSource(fragmentShader, 1, &FragmentShader, NULL); CheckGLESError(); PrintGLESShaderLog(fragmentShader);
        Driver->glCompileShader(fragmentShader); CheckGLESError(); PrintGLESShaderLog(fragmentShader);

        // Link the vertex and fragment shader into a shader program
        Program->shaderProgram = Driver->glCreateProgram(); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
        Driver->glAttachShader(Program->shaderProgram, vertexShader); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
        Driver->glAttachShader(Program->shaderProgram, fragmentShader); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

		Driver->glBindAttribLocation(Program->shaderProgram, SDFProgram::Attributes::SDF_ATTRIB_POSITION, "position"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
		Driver->glBindAttribLocation(Program->shaderProgram, SDFProgram::Attributes::SDF_ATTRIB_UV, "uv"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

        Driver->glLinkProgram(Program->shaderProgram); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

        Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
        Driver->glUseProgram(Program->shaderProgram); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

        auto mat = Driver->glGetUniformLocation(Program->shaderProgram, "mat"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
        auto base_texture = Driver->glGetUniformLocation(Program->shaderProgram, "base_texture"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
        Program->sdf_outline_color = Driver->glGetUniformLocation(Program->shaderProgram, "sdf_outline_color"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);                
        Program->font_color = Driver->glGetUniformLocation(Program->shaderProgram, "font_color"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
        Program->smooth = Driver->glGetUniformLocation(Program->shaderProgram, "smooth_param"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
        Program->border = Driver->glGetUniformLocation(Program->shaderProgram, "border"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

        Driver->glUniformMatrix4fv(mat, 1, GL_FALSE, SDF_Mat);    CheckGLESError();
        Driver->glUniform1i(base_texture, 0);

        Driver->glUseProgram(oldProgramId);

        //logDebug("sdf_outline_color = %d; sdf_params = %d; mat = %d, base_texture = %d, font_color = %d", Program->sdf_outline_color, Program->sdf_params, mat, base_texture, Program->font_color);
        return true;
    }

    void Clear()
    {

    }

public :
	struct Attributes 
	{
		enum : GLuint 
		{
			SDF_ATTRIB_POSITION = 10, // Начинаем не с нуля, чтобы индексы не пересеклись с другими программами			
			SDF_ATTRIB_UV = 11,
			SDF_NUM_ATTRIBUTES = 12,
			//ATTRIB_COLOR = 13
		};
	};


    bool Init()
    {        
        if (this->CompileProgram(&ShaderProgram, SDF_vertexSource, SDF_fragmentSource) && this->CompileProgram(&ShaderProgramOutline, (std::string("#define SDF_OUTLINE \n") + SDF_vertexSource).c_str(), (std::string("#define SDF_OUTLINE \n") + SDF_fragmentSource).c_str()))
        {
            return true;
        }
        return false;
    };

    ShaderProgramStruct* GetShaderProgram(bool Outline)
    {
        if (Outline)
            return &ShaderProgramOutline;

        return &ShaderProgram;
    };
    
    SDFProgram()
    {
        logDebug("SDFProgram constructor");
    }

    ~SDFProgram()
    {
        logDebug("SDFProgram destructor");
        Clear();
    }
};

static SDFProgram sdfProgram;
#endif

class SDFGLTexture
{
    SDL_Surface* sdfTexture = nullptr;
    GLuint GLTexture;
public:
    void Clear();
    bool Load(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority);
    bool Draw(bool Outline, GLsizei Count, AkkordColor& FontColor, AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border, int Spread);
    AkkordPoint GetSize();
    ~SDFGLTexture();
};

void SDFGLTexture::Clear()
{
    if (sdfTexture != nullptr)
    {
        SDL_FreeSurface(sdfTexture);
        sdfTexture = nullptr;
    };
}

bool SDFGLTexture::Load(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority)
{
    this->Clear();
#ifndef __CODEBLOCKS        
    auto Driver = GLESDriver::GetInstance();

    unsigned Size;
    auto buffer = BWrapper::File2Buffer(FileNamePNG, SearchPriority, Size);
    auto io = SDL_RWFromMem(buffer, Size);
    sdfTexture = IMG_LoadPNG_RW(io);
    BWrapper::CloseBuffer(buffer);
    SDL_RWclose(io);

    if (nullptr == sdfTexture)
    {
        logError("fontAtlas create error %s", SDL_GetError());
        return false;
    }

    Driver->glGenTextures((GLsizei)1, &GLTexture); CheckGLESError();
    Driver->glBindTexture((GLenum)GL_TEXTURE_2D, GLTexture); CheckGLESError();
    Driver->glTexParameteri((GLenum)GL_TEXTURE_2D, (GLenum)GL_TEXTURE_MIN_FILTER, (GLint)GL_LINEAR); CheckGLESError();
    Driver->glTexParameteri((GLenum)GL_TEXTURE_2D, (GLenum)GL_TEXTURE_MAG_FILTER, (GLint)GL_LINEAR); CheckGLESError();

    Driver->glTexImage2D((GLenum)GL_TEXTURE_2D, (GLint)0, GL_RGBA, (GLsizei)sdfTexture->w, (GLsizei)sdfTexture->h, (GLint)0, (GLenum)GL_RGBA, (GLenum)GL_UNSIGNED_BYTE, sdfTexture->pixels); CheckGLESError();
#endif
    return true;
}

bool SDFGLTexture::Draw(bool Outline, GLsizei Count, AkkordColor& FontColor, AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border, int Spread)
{
#ifndef __CODEBLOCKS
    GLint oldProgramId;
    auto shaderProgram = sdfProgram.GetShaderProgram(Outline);

    auto Driver = GLESDriver::GetInstance();

    Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
    Driver->glUseProgram(shaderProgram->shaderProgram); CheckGLESError(); PrintGLESProgamLog(shaderProgram->shaderProgram);

    Driver->glBindTexture(GL_TEXTURE_2D, GLTexture); CheckGLESError();

    Driver->glEnableVertexAttribArray(SDFProgram::Attributes::SDF_ATTRIB_POSITION); CheckGLESError();
    Driver->glVertexAttribPointer(SDFProgram::Attributes::SDF_ATTRIB_POSITION, (GLint)2, (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, (GLsizei)0, squareVertices); CheckGLESError();

    Driver->glEnableVertexAttribArray(SDFProgram::Attributes::SDF_ATTRIB_UV); CheckGLESError();
    Driver->glVertexAttribPointer(SDFProgram::Attributes::SDF_ATTRIB_UV, (GLint)2, (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, (GLsizei)0, UV); CheckGLESError();

    Driver->glUniform4f(shaderProgram->font_color, GLfloat(FontColor.GetR()) / 255, GLfloat(FontColor.GetG()) / 255, GLfloat(FontColor.GetB()) / 255, GLfloat(FontColor.GetA()) / 255); CheckGLESError();

    GLfloat smoothness = std::min(0.3f, 0.25f / (GLfloat)Spread / Scale * 1.5f) * 850.0f / 255.f / 3.333f;

    //0.25f / (float)Spread / Scale*/*smoothfact*/1.5)*850.0f

    Driver->glUniform1f(shaderProgram->smooth, smoothness); CheckGLESError();

    if (Outline)
    {
        if (shaderProgram->sdf_outline_color >= 0)
        {
            Driver->glUniform4f(shaderProgram->sdf_outline_color, GLfloat(OutlineColor.GetR()) / 255, GLfloat(OutlineColor.GetG()) / 255, GLfloat(OutlineColor.GetB()) / 255, GLfloat(OutlineColor.GetA()) / 255); CheckGLESError();
        }
        else
        {
            logError("shaderProgram->sdf_outline_color error %d", shaderProgram->sdf_outline_color);
        }

        if (shaderProgram->border >= 0)
        {
            Driver->glUniform1f(shaderProgram->border, Border / 6.666f); CheckGLESError();
        }
        else
        {
            logError("shaderProgram->border error %d", shaderProgram->border);
        }
    }
    //logDebug("Before glDrawElements");
    // Мой самсунг Galaxy S2 работает только с GL_UNSIGNED_SHORT
    Driver->glDrawElements((GLenum)GL_TRIANGLES, Count, (GLenum)GL_UNSIGNED_SHORT, Indices); CheckGLESError();
    //logDebug("After glDrawElements");

    Driver->glDisableVertexAttribArray(SDFProgram::Attributes::SDF_ATTRIB_POSITION); CheckGLESError();
    Driver->glDisableVertexAttribArray(SDFProgram::Attributes::SDF_ATTRIB_UV); CheckGLESError();

    Driver->glUseProgram(oldProgramId); CheckGLESError(); CheckGLESError();
#endif
    return true;
};

AkkordPoint SDFGLTexture::GetSize()
{
    return AkkordPoint(sdfTexture->w, sdfTexture->h);
};

SDFGLTexture::~SDFGLTexture()
{
    Clear();
}

class SDFTexture
{
    SDFGLTexture Texture;
    AkkordColor Color;
    int Spread;

    std::vector<GLfloat>UV;
    std::vector<GLfloat>squareVertices;
    std::vector<GLushort>Indices;

    float Scale;
    bool AutoFlush = false;

public:
    bool Load(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority, int Spread);
    void SetColor(const AkkordColor& Color) { this->Color = Color; };
    bool Draw(const AkkordRect& DestRect, const AkkordRect* SourceRect = nullptr);
    void Clear();
    bool Flush();
    void SetAutoFlush(bool AutoFlush) { this->AutoFlush = AutoFlush; };
    ~SDFTexture();
};

void SDFTexture::Clear()
{
    UV.clear();
    squareVertices.clear();
    Indices.clear();
}

SDFTexture::~SDFTexture()
{
    Clear();
    Texture.Clear();
};

bool SDFTexture::Load(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority, int Spread)
{
    this->Spread = Spread;
    Texture.Load(FileNamePNG, SearchPriority);
    return true;
}

bool SDFTexture::Draw(const AkkordRect& DestRect, const AkkordRect* SourceRect)
{
    auto ScreenSize = BWrapper::GetScreenSize();
    float ScrenW = static_cast<decltype(ScrenW)>(ScreenSize.x);
    float ScrenH = static_cast<decltype(ScrenH)>(ScreenSize.y);

    auto Size = Texture.GetSize();
    auto atlasW = Size.x;
    auto atlasH = Size.y;
    auto Rect = AkkordRect(0,0, atlasW, atlasH);

    if (SourceRect != nullptr)
    {
        Rect = AkkordRect(*SourceRect);
    }

    UV.push_back(float(Rect.x) / atlasW);              UV.push_back(float(Rect.y + Rect.h - 1) / atlasH);
    UV.push_back(float(Rect.x + Rect.w - 1) / atlasW); UV.push_back(float(Rect.y + Rect.h - 1) / atlasH);
    UV.push_back(float(Rect.x) / atlasW);              UV.push_back(float(Rect.y) / atlasH);
    UV.push_back(float(Rect.x + Rect.w - 1) / atlasW); UV.push_back(float(Rect.y) / atlasH);
   
    squareVertices.push_back(2 * (float)(DestRect.x / ScrenW) - 1.0f);                           squareVertices.push_back(2 * (ScrenH - DestRect.y - (DestRect.h)) / ScrenH - 1.0f);
    squareVertices.push_back(2 * (float)(DestRect.x + (float)(DestRect.w - 1)) / ScrenW - 1.0f); squareVertices.push_back(2 * (ScrenH - DestRect.y - (DestRect.h)) / ScrenH - 1.0f);
    squareVertices.push_back(2 * (float)(DestRect.x / ScrenW) - 1.0f);                           squareVertices.push_back(2 * (ScrenH - DestRect.y) / ScrenH - 1.0f);
    squareVertices.push_back(2 * (float)(DestRect.x + (float)(DestRect.w - 1)) / ScrenW - 1.0f); squareVertices.push_back(2 * (ScrenH - DestRect.y) / ScrenH - 1.0f);
       
    decltype(Indices)::value_type PointsCnt = Indices.size() / 6 * 4;
    Indices.push_back(PointsCnt + 0); Indices.push_back(PointsCnt + 1); Indices.push_back(PointsCnt + 2);
    Indices.push_back(PointsCnt + 1); Indices.push_back(PointsCnt + 2); Indices.push_back(PointsCnt + 3);

    Scale = std::max(static_cast<float>(DestRect.w) / Rect.w, static_cast<float>(DestRect.h) / Rect.h);

    if (this->AutoFlush)
        Flush();

    return true;
}

bool SDFTexture::Flush()
{
    if (Indices.size() > 0)
    {
        Texture.Draw(false, (GLsizei)Indices.size(), this->Color, this->Color, &UV.front(), &squareVertices.front(), &Indices.front(), Scale, 0.0f, Spread);
    }
    Clear();
    return true;
};

struct SDFCharInfo
{
    unsigned int id, x, y, w, h;
    int xoffset, yoffset, xadvance;
};

class SDFFont
{
    SDFGLTexture FontAtlas;
    unsigned int ScaleW, ScaleH, LineHeight, Spread;   
    std::map<unsigned, SDFCharInfo> CharsMap;

    bool ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority)
    {
        /*        
        http://www.angelcode.com/products/bmfont/doc/file_format.html
        http://www.angelcode.com/products/bmfont/doc/export_options.html
        https://www.gamedev.net/forums/topic/284560-bmfont-and-how-to-interpret-the-fnt-file/
        */

        FileReader fr;
        std::string line;

        decltype(line.find(',')) lpos;
        decltype(lpos)           rpos;
        
        SDFCharInfo sd;

        if (fr.Open(FNTFile, SearchPriority))
        {
            while (fr.ReadLine(line))
                if (line.size() > 0)
                {
                    if (line.find("char id", 0) != std::string::npos)
                    {
                        lpos = 0;
                        rpos = 0;

                        rpos = line.find("id=", lpos) + 3;
                        if (line[rpos] == '\"') ++rpos;
                        auto id = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        lpos = rpos;
                        rpos = line.find("x=", lpos) + 2;
                        if (line[rpos] == '\"') ++rpos;
                        auto x = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        lpos = rpos;
                        rpos = line.find("y=", lpos) + 2;
                        if (line[rpos] == '\"') ++rpos;
                        auto y = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        lpos = rpos;
                        rpos = line.find("width=", lpos) + 6;
                        if (line[rpos] == '\"') ++rpos;
                        auto w = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        lpos = rpos;
                        rpos = line.find("height=", lpos) + 7;
                        if (line[rpos] == '\"') ++rpos;
                        auto h = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        lpos = rpos;
                        rpos = line.find("xoffset=", lpos) + 8;
                        if (line[rpos] == '\"') ++rpos;
                        auto dx = std::stoi(std::string(line, rpos));                        
                        
                        lpos = rpos;
                        rpos = line.find("yoffset=", lpos) + 8;
                        if (line[rpos] == '\"') ++rpos;
                        auto dy = std::stoi(std::string(line, rpos));

                        lpos = rpos;
                        rpos = line.find("xadvance=", lpos) + 9;
                        if (line[rpos] == '\"') ++rpos;
                        auto xa =std::stoi(std::string(line, rpos));

                        sd.x = x; sd.y = y; sd.w = w; sd.h = h; sd.xoffset = dx; sd.yoffset = dy; sd.xadvance = xa;
                        CharsMap.emplace(id, sd);

                        //logDebug("dx=%d, dy=%d, xa=%d", dx, dy, xa);
                    } 
                    else if (line.find("chars", 0) != std::string::npos)
                    {
                        //auto cnt = BWrapper::Str2Num(std::string(line, line.find("\"", 0) + 1).c_str());
                        //CharsVector.reserve(cnt);                        
                    }
                    else if (line.find("common", 0) != std::string::npos)
                    {

                        rpos = line.find("lineHeight=", 0) + 11;
                        if (line[rpos] == '\"') ++rpos;
                        LineHeight = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        rpos = line.find("scaleW=", 0) + 7;
                        if (line[rpos] == '\"') ++rpos;
                        ScaleW = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        rpos = line.find("scaleH=", 0) + 7;
                        if (line[rpos] == '\"') ++rpos;
                        ScaleH = BWrapper::Str2Num(std::string(line, rpos).c_str());

                        //logDebug("ScaleW = %d, ScaleH = %d", ScaleW, ScaleH);                        
                    };
                };
        };

        //for (auto v : CharsMap) logDebug("id=%d, x=%d, y=%d, w=%d, h=%d", v.first, v.second.x, v.second.y, v.second.w, v.second.h);

        fr.Close();

        return true;
    }

    void Clear()
    {        
        CharsMap.clear();
        FontAtlas.Clear();
    };

public:
    enum struct AlignV : unsigned char { Top, Center, Bottom };
    enum struct AlignH : unsigned char { Left, Center, Right };

    ~SDFFont()
    {
        Clear();
    }

    unsigned int GetAtlasW(){ return ScaleW; }
    unsigned int GetAtlasH(){ return ScaleH; }

    bool Load(const char* FileNameFNT, const char* FileNamePNG,  BWrapper::FileSearchPriority SearchPriority, int Spread)
    {
        this->Clear();
        this->Spread = Spread;
        FontAtlas.Load(FileNamePNG, SearchPriority);
        ParseFNTFile(FileNameFNT, BWrapper::FileSearchPriority::Assets);
        return true;
    };

	bool Draw(bool Outline, GLsizei Count, AkkordColor& FontColor, AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border)
    {
        FontAtlas.Draw(Outline, Count, FontColor, OutlineColor, UV, squareVertices, Indices, Scale, Border, Spread);
        return true;
    };

    bool GetCharInfo(unsigned Code, SDFCharInfo& ci)
    {
        auto res = CharsMap.find(Code);
        if(res != CharsMap.end())
        {
            ci = res->second;
            return true;
        }
        logError("Char with id=%u not found", Code);    
        return false;
    };

    unsigned GetLineHeight() { return LineHeight; }
};

// Для рисования всегда указывать левую верхнюю точку (удобно для разгаданных слов в "составь слова")

class SDFFontBuffer
{
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float Border = 0.0f;

    SDFFont* sdfFont = nullptr;
    int rectW = -1, rectH = -1;

    bool outline = false;

    SDFFont::AlignH alignH = SDFFont::AlignH::Center;
    SDFFont::AlignV alignV = SDFFont::AlignV::Center;    

    AkkordColor color, outlineColor;

	std::vector<GLfloat>UV;
	std::vector<GLfloat>squareVertices;
	std::vector<GLushort>Indices;

    //float offset, contrast, outlineOffset, outlineContrast;

    AkkordPoint GetTextSizeByLine(const char* Text, std::vector<unsigned>& VecSize)
    {
        // VecSize - вектор строк (в надписи может быть несколько строк, нужно считать длину каждой строки отдельно - для выравнивания)
        
        AkkordPoint pt, localpoint;

        unsigned int i = 0;
        unsigned int a = 0;
        unsigned len = std::string(Text).size();

        if (len == 0)
            logWarning("Zero-length text");

        pt.x = 0;
        pt.y = 0;

        localpoint.x = 0;
        localpoint.y = 0;

        SDFCharInfo charParams;

        while (i < len)
        {
            a = UTF2Unicode(Text, i);

            if (a == 10) // Если это переход строки
            {
                pt.y += static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight()); // надо учесть общую высоту строки
                VecSize.push_back(localpoint.x);
                pt.x = std::max(pt.x, localpoint.x);
                localpoint.x = 0;
            }            
            else if (a != 13) // Если это не переход строки       
            {
                sdfFont->GetCharInfo(a, charParams);

                localpoint.x += static_cast<decltype(localpoint.x)>(scaleX * charParams.xoffset);
				localpoint.x += static_cast<decltype(localpoint.x)>(scaleX * (a == 32 ? charParams.xadvance : charParams.w));
            }
        };

        VecSize.push_back(localpoint.x);
        pt.x = std::max(pt.x, localpoint.x);

        //pt.y += localpoint.y; // надо учесть общую высоту строки
        pt.y = static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight() * VecSize.size());

        return pt;
    }
public:
    SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, AkkordColor Color)
    {
        this->Clear();
        sdfFont = Font;        
        color = Color;
        Reserve(DigitsCount);
    }

    void SetFont(SDFFont* Font){ sdfFont = Font; };    
    void SetScale(float Scale){ scaleX = scaleY = Scale; }
    void SetScale(float ScaleX, float ScaleY){ scaleX = ScaleX; scaleY = ScaleY; }

    void SetColor(const AkkordColor Color) { color = Color; };
    void SetOutline(bool Outline){ outline = Outline; }
    void SetOutlineColor(const AkkordColor OutlineColor) { outlineColor = OutlineColor; };
    void SetBorder(float BordeWidth){ this->Border = BordeWidth; }

    float GetScaleX(){ return scaleX; }
    float GetScaleY(){ return scaleY; }    

    void SetRect(int W, int H) { rectW = W; rectH = H; }

    void SetAlignment(SDFFont::AlignH AlignH, SDFFont::AlignV AlignV){ alignH = AlignH; alignV = AlignV; }    
    void SetAlignmentH(SDFFont::AlignH AlignH){ alignH = AlignH; }
    void SetAlignmentV(SDFFont::AlignV AlignV){ alignV = AlignV; }

    SDFFont::AlignH GetAlignH() { return alignH; }
    SDFFont::AlignV GetAlignV() { return alignV; }

    void Reserve(unsigned Count) 
    {
        UV.reserve(Count * 4);
        squareVertices.reserve(Count * 4);
        Indices.reserve(Count * 6);
    }

    void Clear()
    {
        UV.clear();
        squareVertices.clear();
        Indices.clear();                
    };

    void Flush()
    {        
        if (Indices.size() > 0)
        {
			sdfFont->Draw(this->outline, (GLsizei)Indices.size(), this->color, this->outlineColor, &UV.front(), &squareVertices.front(), &Indices.front(), (GLfloat)this->scaleX, (GLfloat)this->Border);
        }
        Clear();
    };    
    
    ~SDFFontBuffer()
    {
        Clear();
        sdfFont = nullptr;
    };    

    // сейчас это int, возможно для этой функции сделать отдельный тип со float
    AkkordPoint GetTextSize(const char* Text)
    {
        std::vector<unsigned> VecSize;
        AkkordPoint pt(1, 1);
        pt = GetTextSizeByLine(Text, VecSize);
        return pt;
    };

    AkkordPoint DrawText(int X, int Y, const char* Text)
    {        
        std::vector<unsigned> VecSize;
        AkkordPoint pt, size;
        pt = size = GetTextSizeByLine(Text, VecSize);        
#ifndef __CODEBLOCKS
        decltype(X) x_start, x_current;
        pt = AkkordPoint(0, 0);

        unsigned int i = 0;
        unsigned int a = 0;
        unsigned len = std::string(Text).size();

        auto atlasW = sdfFont->GetAtlasW();
        auto atlasH = sdfFont->GetAtlasH();

        auto ScreenSize = BWrapper::GetScreenSize();    
        
        float ScrenW = static_cast<decltype(ScrenW)>(ScreenSize.x);
        float ScrenH = static_cast<decltype(ScrenH)>(ScreenSize.y);

        SDFCharInfo charParams;
        
        unsigned line = 0;
        decltype (Indices)::value_type PointsCnt = static_cast<decltype (Indices)::value_type>(UV.size() / 2); // Разделив на 2, получаем количество вершин

        switch (alignV)
        {
            case SDFFont::AlignV::Center:
                Y = Y + (rectH - size.y) / 2;
                break;
            case SDFFont::AlignV::Bottom:
                Y = Y + (rectH - size.y);
                break;
            default: // в остальных случаях ничего не делаем, координату Y не меняем
                break;
        };

        check_h_align:
        // Выбираем начальную точку в зависимости от выравнивания
        switch (alignH)
        {
            case SDFFont::AlignH::Center:
                x_start = X + (rectW - VecSize[line]) / 2;                
                break;
            case SDFFont::AlignH::Right:
                x_start = X + (rectW - VecSize[line]);                
                break;
            default:
                x_start = X;                
                break;
        };        

        x_current = x_start;

        while (i < len)
        {
            a = UTF2Unicode(Text, i);

            if (a == 10)
            {
                ++line;
                Y += static_cast<decltype(Y)>(scaleY * sdfFont->GetLineHeight());

                pt.x = std::max(pt.x, x_current - x_start + 1);
                goto check_h_align;                                                        
            }
            else if (a == 13) {} // ничего не делаем
            else
            {
                sdfFont->GetCharInfo(a, charParams);

                x_current += static_cast<decltype(x_current)>(scaleX * charParams.xoffset);

                UV.push_back(float(charParams.x) / atlasW);                    UV.push_back(float(charParams.y + charParams.h - 1) / atlasH);
                UV.push_back(float(charParams.x + charParams.w - 1) / atlasW); UV.push_back(float(charParams.y + charParams.h - 1) / atlasH);
                UV.push_back(float(charParams.x) / atlasW);                    UV.push_back(float(charParams.y) / atlasH);
                UV.push_back(float(charParams.x + charParams.w - 1) / atlasW); UV.push_back(float(charParams.y) / atlasH);

                squareVertices.push_back(2 * (float)(x_current / ScrenW) - 1.0f);                                      squareVertices.push_back(2 * (ScrenH - Y - scaleY * (charParams.h + charParams.yoffset)) / ScrenH - 1.0f);
                squareVertices.push_back(2 * (float)(x_current + (float)scaleX * (charParams.w - 1)) / ScrenW - 1.0f); squareVertices.push_back(2 * (ScrenH - Y - scaleY * (charParams.h + charParams.yoffset)) / ScrenH - 1.0f);
                squareVertices.push_back(2 * (float)(x_current / ScrenW) - 1.0f);                                      squareVertices.push_back(2 * (ScrenH - Y - scaleY * charParams.yoffset) / ScrenH - 1.0f);
                squareVertices.push_back(2 * (float)(x_current + (float)scaleX * (charParams.w - 1)) / ScrenW - 1.0f); squareVertices.push_back(2 * (ScrenH - Y - scaleY * charParams.yoffset) / ScrenH - 1.0f);

                Indices.push_back(PointsCnt + 0); Indices.push_back(PointsCnt + 1); Indices.push_back(PointsCnt + 2);
                Indices.push_back(PointsCnt + 1); Indices.push_back(PointsCnt + 2); Indices.push_back(PointsCnt + 3);

                //x_current = x_current + (float)scaleX * (charParams.w /*+ charParams.xadvance*/);
				x_current = x_current + static_cast<decltype(x_current)>(scaleX * (a == 32 ? charParams.xadvance : charParams.w));
                PointsCnt += 4;
            }
        };        

        pt.x = std::max(pt.x, x_current - x_start + 1);
        pt.y = static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight() * VecSize.size());

        //logDebug("[%d %d] [%d %d]", size.x, size.y, pt.x, pt.y);
#endif
        return pt;
    };
};

#endif // __AKK0RD_SDFFONT_H__