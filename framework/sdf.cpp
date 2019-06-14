#include "sdf.h"
#include "SDL_image.h"
#include"openglesdriver.h"

/*
https://github.com/libgdx/libgdx/wiki/Hiero
"java -cp gdx.jar;gdx-natives.jar;gdx-backend-lwjgl.jar;gdx-backend-lwjgl-natives.jar;extensions\gdx-freetype\gdx-freetype.jar;extensions\gdx-freetype\gdx-freetype-natives.jar;extensions\gdx-tools\gdx-tools.jar com.badlogic.gdx.tools.hiero.Hiero"
*/


static const GLchar* SDF_vertexSource =
"varying highp vec4 result_color; \n\
varying highp vec2 result_uv; \n\
uniform highp mat4 u_projection; \n\
uniform highp vec4 font_color; \n\
uniform highp float smooth_param; \n\
attribute highp vec2 a_position; \n\
attribute highp vec2 a_texCoord; \n\
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
    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);  \n\
    result_color = font_color; \n\
    result_uv = a_texCoord; \n\
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

// https://habrahabr.ru/post/282191/
static inline unsigned int UTF2Unicode(const /*unsigned*/ char *txt, unsigned &i) {
    unsigned int a = txt[i++];
    if ((a & 0x80) == 0)return a;
    if ((a & 0xE0) == 0xC0) {
        a = (a & 0x1F) << 6;
        a |= txt[i++] & 0x3F;
    }
    else if ((a & 0xF0) == 0xE0) {
        a = (a & 0xF) << 12;
        a |= (txt[i++] & 0x3F) << 6;
        a |= txt[i++] & 0x3F;
    }
    else if ((a & 0xF8) == 0xF0) {
        a = (a & 0x7) << 18;
        a |= (a & 0x3F) << 12;
        a |= (txt[i++] & 0x3F) << 6;
        a |= txt[i++] & 0x3F;
    }
    return a;
};

static const GLfloat SDF_Mat[] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

SDFProgram& SDFProgram::GetInstance()
{
    static SDFProgram sdfProgram;
    return sdfProgram;
};

    bool SDFProgram::CompileProgram(ShaderProgramStruct* Program, const char* VertextShader, const char* FragmentShader)
    {
        GLint oldProgramId;

        auto Driver = &GLESDriver::GetInstance();        

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

		Driver->glBindAttribLocation(Program->shaderProgram, SDFProgram::Attributes::SDF_ATTRIB_POSITION, "a_position"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
		Driver->glBindAttribLocation(Program->shaderProgram, SDFProgram::Attributes::SDF_ATTRIB_UV, "a_texCoord"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

        Driver->glLinkProgram(Program->shaderProgram); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

        Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
        Driver->glUseProgram(Program->shaderProgram); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

        auto mat = Driver->glGetUniformLocation(Program->shaderProgram, "u_projection"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
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

    void SDFProgram::Clear()
    {

    }

    bool SDFProgram::Init()
    {        
        if (this->CompileProgram(&ShaderProgram, SDF_vertexSource, SDF_fragmentSource) && this->CompileProgram(&ShaderProgramOutline, (std::string("#define SDF_OUTLINE \n") + SDF_vertexSource).c_str(), (std::string("#define SDF_OUTLINE \n") + SDF_fragmentSource).c_str()))
        {
            return true;
        }
        return false;
    };

    ShaderProgramStruct* SDFProgram::GetShaderProgram(bool Outline)
    {
        if (Outline)
            return &ShaderProgramOutline;

        return &ShaderProgram;
    };
    
    SDFProgram::SDFProgram()
    {
        //logDebug("SDFProgram constructor");
    }

    SDFProgram::~SDFProgram()
    {
        logDebug("SDFProgram destructor");
        Clear();
    }

/*
class SDFGLTexture
{
    AkkordTexture akkordTexture;
public:
    void Clear();
    bool Load(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority);
    bool Draw(bool Outline, GLsizei Count, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border, int Spread);
    AkkordPoint GetSize();
    ~SDFGLTexture();
};
*/

void SDFGLTexture::Clear()
{

}

bool SDFGLTexture::Load(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority)
{
    this->Clear();
    akkordTexture.LoadFromFile(FileNamePNG, AkkordTexture::TextureType::PNG, SearchPriority);
    return true;
}

bool SDFGLTexture::LoadFromMemory(const char* Buffer, int Size)
{
	this->Clear();	
	akkordTexture.LoadFromMemory(Buffer, Size, AkkordTexture::TextureType::PNG);
	return true;
};

bool SDFGLTexture::Draw(bool Outline, GLsizei Count, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border, int Spread)
{
    GLint oldProgramId;

    struct VertextAttrParamsStruct
    {
        GLint attr_0_enabled, attr_1_enabled, attr_2_enabled, attr_3_enabled;
    } VertexParams;
    VertexParams.attr_0_enabled = VertexParams.attr_1_enabled = VertexParams.attr_2_enabled = VertexParams.attr_3_enabled = GL_FALSE;

    auto shaderProgram = SDFProgram::GetInstance().GetShaderProgram(Outline);

    auto Driver = GLESDriver::GetInstance();

    Driver.glGetIntegerv((GLenum)GL_CURRENT_PROGRAM, &oldProgramId); CheckGLESError();
    
    {
        Driver.glGetVertexAttribiv((GLuint)0, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_0_enabled); CheckGLESError();
        Driver.glGetVertexAttribiv((GLuint)1, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_1_enabled); CheckGLESError();
        Driver.glGetVertexAttribiv((GLuint)2, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_2_enabled); CheckGLESError();
        Driver.glGetVertexAttribiv((GLuint)3, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_3_enabled); CheckGLESError();

        // эти два атрибута нужно включить, если они выключены
        if (VertexParams.attr_0_enabled == GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)0); CheckGLESError(); }
        if (VertexParams.attr_1_enabled == GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)1); CheckGLESError(); }

        // эти атрибуты выключаем всегда, если они включены
        if (VertexParams.attr_2_enabled != GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)2); CheckGLESError(); }
        if (VertexParams.attr_3_enabled != GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)3); CheckGLESError(); }
    }
    
    if (oldProgramId != shaderProgram->shaderProgram)
    {
        Driver.glUseProgram(shaderProgram->shaderProgram); CheckGLESError(); PrintGLESProgamLog(shaderProgram->shaderProgram);
    }

    //Driver->glBindTexture((GLenum)GL_TEXTURE_2D, GLTexture); CheckGLESError();
    SDL_GL_BindTexture(akkordTexture.GetTexture(), nullptr, nullptr);

    Driver.glVertexAttribPointer(SDFProgram::Attributes::SDF_ATTRIB_POSITION, (GLint)2, (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, (GLsizei)0, squareVertices); CheckGLESError();
    Driver.glVertexAttribPointer(SDFProgram::Attributes::SDF_ATTRIB_UV, (GLint)2, (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, (GLsizei)0, UV); CheckGLESError();

    Driver.glUniform4f(shaderProgram->font_color, GLfloat(FontColor.GetR()) / 255.0f, GLfloat(FontColor.GetG()) / 255.0f, GLfloat(FontColor.GetB()) / 255.0f, GLfloat(FontColor.GetA()) / 255.0f); CheckGLESError();

    GLfloat smoothness = std::min(0.3f, 0.25f / (GLfloat)Spread / Scale * 1.5f) * 850.0f / 255.f / 3.333f;

    //0.25f / (float)Spread / Scale*/*smoothfact*/1.5)*850.0f

    Driver.glUniform1f(shaderProgram->smooth, smoothness); CheckGLESError();

    if (Outline)
    {
        if (shaderProgram->sdf_outline_color >= 0)
        {
            Driver.glUniform4f(shaderProgram->sdf_outline_color, GLfloat(OutlineColor.GetR()) / 255.0f, GLfloat(OutlineColor.GetG()) / 255.0f, GLfloat(OutlineColor.GetB()) / 255.0f, GLfloat(OutlineColor.GetA()) / 255.0f); CheckGLESError();
        }
        else
        {
            logError("shaderProgram->sdf_outline_color error %d", shaderProgram->sdf_outline_color);
        }

        if (shaderProgram->border >= 0)
        {
            Driver.glUniform1f(shaderProgram->border, Border / 6.666f); CheckGLESError();
        }
        else
        {
            logError("shaderProgram->border error %d", shaderProgram->border);
        }
    }
        
    Driver.glDrawElements((GLenum)GL_TRIANGLES, Count, (GLenum)GL_UNSIGNED_SHORT, Indices); CheckGLESError();    

    // unbind texture
    SDL_GL_UnbindTexture(akkordTexture.GetTexture());
    
    if (shaderProgram->shaderProgram != oldProgramId && oldProgramId > 0)
    {
        Driver.glUseProgram(oldProgramId); CheckGLESError();
    }
    
    { // возвращаем все исходное состояние
        
        // в рамках обработки SDF мы включили эти атрибуты. Возвращаем их в исходное состояние
        if (VertexParams.attr_0_enabled == GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)0); CheckGLESError(); }
        if (VertexParams.attr_1_enabled == GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)1); CheckGLESError(); }
                                                             
        if (VertexParams.attr_2_enabled != GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)2); CheckGLESError(); }
        if (VertexParams.attr_3_enabled != GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)3); CheckGLESError(); }
    }    

    return true;
};

AkkordPoint SDFGLTexture::GetSize()
{    
    return akkordTexture.GetSize();
};

SDFGLTexture::~SDFGLTexture()
{
    Clear();
}


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

bool SDFTexture::LoadFromMemory(const char* Buffer, int Size, int Spread)
{
	this->Spread = Spread;
	Texture.LoadFromMemory(Buffer, Size);
	return true;
};

bool SDFTexture::Draw(const AkkordRect& DestRect, const AkkordRect* SourceRect)
{
    auto ScreenSize = BWrapper::GetScreenSize();

    // если целевое размещение не попадает на экран, не рисуем его
    if (DestRect.x > ScreenSize.x || DestRect.x + DestRect.w < 0 || DestRect.y > ScreenSize.y || DestRect.y + DestRect.h < 0)
    {
        return false;
    }

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
        Texture.Draw(Outline, (GLsizei)Indices.size(), this->Color, this->OutlineColor, &UV.front(), &squareVertices.front(), &Indices.front(), Scale, (GLfloat)Border, Spread);
    }
    Clear();
    return true;
};


    bool SDFFont::ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority)
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

    void SDFFont::Clear()
    {        
        CharsMap.clear();
        FontAtlas.Clear();
    };



    SDFFont::~SDFFont()
    {
        Clear();
    }

    unsigned int SDFFont::GetAtlasW(){ return ScaleW; }
    unsigned int SDFFont::GetAtlasH(){ return ScaleH; }

    bool SDFFont::Load(const char* FileNameFNT, const char* FileNamePNG,  BWrapper::FileSearchPriority SearchPriority, int Spread)
    {
        this->Clear();
        this->Spread = Spread;
        FontAtlas.Load(FileNamePNG, SearchPriority);
        ParseFNTFile(FileNameFNT, BWrapper::FileSearchPriority::Assets);
        return true;
    };

	bool SDFFont::Draw(bool Outline, GLsizei Count, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border)
    {
        FontAtlas.Draw(Outline, Count, FontColor, OutlineColor, UV, squareVertices, Indices, Scale, Border, Spread);
        return true;
    };

    bool SDFFont::GetCharInfo(unsigned Code, SDFCharInfo& ci)
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

    unsigned SDFFont::GetLineHeight() { return LineHeight; }


// Для рисования всегда указывать левую верхнюю точку (удобно для разгаданных слов в "составь слова")



    AkkordPoint SDFFontBuffer::GetTextSizeByLine(const char* Text, std::vector<unsigned>& VecSize)
    {
        // VecSize - вектор строк (в надписи может быть несколько строк, нужно считать длину каждой строки отдельно - для выравнивания)
        
        AkkordPoint pt, localpoint;
        pt = localpoint = AkkordPoint(0, 0);

        unsigned int a = 0;
        unsigned int i = 0;

        SDFCharInfo charParams;

        if (Text != nullptr)
        {
            do
            {
                a = UTF2Unicode(Text, i);

                if (a == 0)
                {
                    break;
                }
                else if (a == 10) // Если это переход строки
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
                    localpoint.x += static_cast<decltype(localpoint.x)>(scaleX * charParams.xadvance);
                }
            } while (true);
        }

        VecSize.push_back(localpoint.x);
        pt.x = std::max(pt.x, localpoint.x);

        //pt.y += localpoint.y; // надо учесть общую высоту строки
        pt.y = static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight() * VecSize.size());

        return pt;
    }

    SDFFontBuffer::SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, const AkkordColor& Color)
    {
        this->Clear();
        sdfFont = Font;        
        color = Color;
        Reserve(DigitsCount);
    }

    void SDFFontBuffer::SetFont(SDFFont* Font){ sdfFont = Font; };    
    void SDFFontBuffer::SetScale(float Scale){ scaleX = scaleY = Scale; }
    void SDFFontBuffer::SetScale(float ScaleX, float ScaleY){ scaleX = ScaleX; scaleY = ScaleY; }

    void SDFFontBuffer::SetColor(const AkkordColor& Color) { color = Color; };
    void SDFFontBuffer::SetOutline(bool Outline){ outline = Outline; }
    void SDFFontBuffer::SetOutlineColor(const AkkordColor& OutlineColor) { outlineColor = OutlineColor; };
    void SDFFontBuffer::SetBorder(float BorderWidth){ this->Border = BorderWidth; }

    float SDFFontBuffer::GetScaleX(){ return scaleX; }
    float SDFFontBuffer::GetScaleY(){ return scaleY; }    

    void SDFFontBuffer::SetRect(int W, int H) { rectW = W; rectH = H; }

    void SDFFontBuffer::SetAlignment(SDFFont::AlignH AlignH, SDFFont::AlignV AlignV){ alignH = AlignH; alignV = AlignV; }    
    void SDFFontBuffer::SetAlignmentH(SDFFont::AlignH AlignH){ alignH = AlignH; }
    void SDFFontBuffer::SetAlignmentV(SDFFont::AlignV AlignV){ alignV = AlignV; }

    SDFFont::AlignH SDFFontBuffer::GetAlignH() { return alignH; }
    SDFFont::AlignV SDFFontBuffer::GetAlignV() { return alignV; }

    void SDFFontBuffer::Reserve(unsigned Count) 
    {
        UV.reserve(Count * 4);
        squareVertices.reserve(Count * 4);
        Indices.reserve(Count * 6);
    }

    void SDFFontBuffer::Clear()
    {
        UV.clear();
        squareVertices.clear();
        Indices.clear();                
    };

    void SDFFontBuffer::Flush()
    {        
        if (Indices.size() > 0)
        {
			sdfFont->Draw(this->outline, (GLsizei)Indices.size(), this->color, this->outlineColor, &UV.front(), &squareVertices.front(), &Indices.front(), (GLfloat)this->scaleX, (GLfloat)this->Border);
        }
        Clear();
    };    
    
    SDFFontBuffer::~SDFFontBuffer()
    {
        Clear();
        sdfFont = nullptr;
    };    

    // сейчас это int, возможно для этой функции сделать отдельный тип со float
    AkkordPoint SDFFontBuffer::GetTextSize(const char* Text)
    {
        std::vector<unsigned> VecSize;
        AkkordPoint pt(1, 1);
        pt = GetTextSizeByLine(Text, VecSize);
        return pt;
    };

    AkkordPoint SDFFontBuffer::DrawText(int X, int Y, const char* Text)
    {        
        AkkordPoint pt = AkkordPoint(0,0);
        if (Text != nullptr)
        {
            std::vector<unsigned> VecSize;
            AkkordPoint size;
            pt = size = GetTextSizeByLine(Text, VecSize);
            decltype(X) x_start, x_current;
            pt = AkkordPoint(0, 0);

            unsigned int a = 0;

            unsigned i = 0;

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

            do
            {
                a = UTF2Unicode(Text, i);

                if (a == 0)
                {
                    break;
                }
                else if (a == 10)
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

                    const decltype(charParams.w) minus = 0;

                    UV.push_back(float(charParams.x) / atlasW);                        UV.push_back(float(charParams.y + charParams.h - minus) / atlasH);
                    UV.push_back(float(charParams.x + charParams.w - minus) / atlasW); UV.push_back(float(charParams.y + charParams.h - minus) / atlasH);
                    UV.push_back(float(charParams.x) / atlasW);                        UV.push_back(float(charParams.y) / atlasH);
                    UV.push_back(float(charParams.x + charParams.w - minus) / atlasW); UV.push_back(float(charParams.y) / atlasH);

                    squareVertices.push_back(2 * (float)(x_current / ScrenW) - 1.0f);                                      squareVertices.push_back(2 * (ScrenH - Y - scaleY * (charParams.h + charParams.yoffset)) / ScrenH - 1.0f);
                    squareVertices.push_back(2 * (float)(x_current + (float)scaleX * (charParams.w - 1)) / ScrenW - 1.0f); squareVertices.push_back(2 * (ScrenH - Y - scaleY * (charParams.h + charParams.yoffset)) / ScrenH - 1.0f);
                    squareVertices.push_back(2 * (float)(x_current / ScrenW) - 1.0f);                                      squareVertices.push_back(2 * (ScrenH - Y - scaleY * charParams.yoffset) / ScrenH - 1.0f);
                    squareVertices.push_back(2 * (float)(x_current + (float)scaleX * (charParams.w - 1)) / ScrenW - 1.0f); squareVertices.push_back(2 * (ScrenH - Y - scaleY * charParams.yoffset) / ScrenH - 1.0f);

                    Indices.push_back(PointsCnt + 0); Indices.push_back(PointsCnt + 1); Indices.push_back(PointsCnt + 2);
                    Indices.push_back(PointsCnt + 1); Indices.push_back(PointsCnt + 2); Indices.push_back(PointsCnt + 3);

                    //x_current = x_current + (float)scaleX * (charParams.w /*+ charParams.xadvance*/);
                    x_current = x_current + static_cast<decltype(x_current)>(scaleX * charParams.xadvance);
                    PointsCnt += 4;
                }
            } while (true);

            pt.x = std::max(pt.x, x_current - x_start + 1);
            pt.y = static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight() * VecSize.size());

            //logDebug("[%d %d] [%d %d]", size.x, size.y, pt.x, pt.y);
        }
        return pt;
    };
