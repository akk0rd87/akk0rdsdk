#ifndef __AKK0RD_SDFFONT_H__
#define __AKK0RD_SDFFONT_H__

#include <map>
#include "basewrapper.h"
#include "openglesdriver.h"

// https://github.com/libgdx/libgdx/wiki/distance-field-fonts

/*
Call stack:
1) SDFFontBuffer -> SDFFont -> SDFTexture
2) SDFAtlas -> SDFTexture
*/


/*
https://github.com/libgdx/libgdx/wiki/Hiero
"java -cp gdx.jar;gdx-natives.jar;gdx-backend-lwjgl.jar;gdx-backend-lwjgl-natives.jar;extensions\gdx-freetype\gdx-freetype.jar;extensions\gdx-freetype\gdx-freetype-natives.jar;extensions\gdx-tools\gdx-tools.jar com.badlogic.gdx.tools.hiero.Hiero"
*/

struct ShaderProgramStruct
{
    GLuint shaderProgram;
    GLint sdf_outline_color, font_color, smooth, border;
};

struct SDFCharInfo
{
    unsigned int id, x, y, w, h;
    int xoffset, yoffset, xadvance;
};

class SDFProgram
{
    ShaderProgramStruct ShaderProgram, ShaderProgramOutline;

    bool CompileProgram(ShaderProgramStruct* Program, const char* VertextShader, const char* FragmentShader);

    void Clear();

public :
	struct Attributes 
	{
		enum : GLuint 
		{
			SDF_ATTRIB_POSITION = 0, // Начинаем не с нуля, чтобы индексы не пересеклись с другими программами			
			SDF_ATTRIB_UV = 1
			//SDF_NUM_ATTRIBUTES = 7,
			//ATTRIB_COLOR = 8
		};
	};

    bool Init();
    ShaderProgramStruct* GetShaderProgram(bool Outline);    
    SDFProgram();
    ~SDFProgram();

    static SDFProgram& GetInstance();
};

class SDFGLTexture
{
    AkkordTexture akkordTexture;
public:
    void Clear();
    bool LoadFromFile(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority);
    bool LoadFromMemory(const char* Buffer, int Size);
    bool Draw(bool Outline, GLsizei Count, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border, int Spread);
    AkkordPoint GetSize();
    ~SDFGLTexture();
};

class SDFTexture
{
    SDFGLTexture Texture;
    AkkordColor Color, OutlineColor;
    int Spread;

    std::vector<GLfloat>UV;
    std::vector<GLfloat>squareVertices;
    std::vector<GLushort>Indices;

    float Scale, Border;
    bool AutoFlush = false;
    bool Outline = false;
	float atlasW;
	float atlasH;
	void InitAtlasWH();
public:
    bool LoadFromFile(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority, int Spread);
    bool LoadFromMemory(const char* Buffer, int Size, int Spread);
    void SetColor(const AkkordColor& Color) { this->Color = Color; };
    void SetOutlineColor(const AkkordColor& OutlineColor) { this->OutlineColor = OutlineColor; };
    bool Draw(const AkkordRect& DestRect, const AkkordRect* SourceRect = nullptr);    
    void Clear();
    bool Flush();
    void SetAutoFlush(bool AutoFlush) { this->AutoFlush = AutoFlush; };
    void SetOutline(bool Outline) { this->Outline = Outline; };
    void SetBorder(float Border) { this->Border = Border; };
    ~SDFTexture();
};

class SDFFont
{
    SDFGLTexture FontAtlas;
    unsigned int ScaleW, ScaleH, LineHeight, Spread;   
    std::map<unsigned, SDFCharInfo> CharsMap;

    bool ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority);
    void Clear();

public:
    enum struct AlignV : unsigned char { Top, Center, Bottom };
    enum struct AlignH : unsigned char { Left, Center, Right };

    ~SDFFont();

    unsigned int GetAtlasW();
    unsigned int GetAtlasH();

    bool Load(const char* FileNameFNT, const char* FileNamePNG,  BWrapper::FileSearchPriority SearchPriority, int Spread);
	bool Draw(bool Outline, GLsizei Count, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const GLfloat* UV, const GLfloat* squareVertices, const GLushort* Indices, GLfloat Scale, GLfloat Border);
    bool GetCharInfo(unsigned Code, SDFCharInfo& ci);
    unsigned GetLineHeight();
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

    AkkordPoint GetTextSizeByLine(const char* Text, std::vector<unsigned>& VecSize);
public:
    SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, const AkkordColor& Color);

    void SetFont(SDFFont* Font);    
    void SetScale(float Scale);
    void SetScale(float ScaleX, float ScaleY);

    void SetColor(const AkkordColor& Color);
    void SetOutline(bool Outline);
    void SetOutlineColor(const AkkordColor& OutlineColor);
    void SetBorder(float BorderWidth);

    float GetScaleX();
    float GetScaleY();

    void SetRect(int W, int H);

    void SetAlignment(SDFFont::AlignH AlignH, SDFFont::AlignV AlignV);
    void SetAlignmentH(SDFFont::AlignH AlignH);
    void SetAlignmentV(SDFFont::AlignV AlignV);

    SDFFont::AlignH GetAlignH();
    SDFFont::AlignV GetAlignV();

    void Reserve(unsigned Count);

    void Clear();
    void Flush();    
    ~SDFFontBuffer();
    // сейчас это int, возможно для этой функции сделать отдельный тип со float
    AkkordPoint GetTextSize(const char* Text);
    AkkordPoint DrawText(int X, int Y, const char* Text);
};

#endif // __AKK0RD_SDFFONT_H__