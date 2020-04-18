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

public:
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
    ShaderProgramStruct* GetShaderProgram(bool Outline) { return (Outline ? &ShaderProgramOutline : &ShaderProgram); };
    SDFProgram() {};
    ~SDFProgram() { Clear(); };

    static SDFProgram& GetInstance();

    //Запрещаем создавать экземпляр класса SDFProgram
    SDFProgram(SDFProgram& rhs) = delete; // Копирующий: конструктор
    SDFProgram(SDFProgram&& rhs) = delete; // Перемещающий: конструктор
    SDFProgram& operator= (SDFProgram&& rhs) = delete; // Оператор перемещающего присваивания
};

class SDFGLTexture
{
    AkkordTexture akkordTexture;
public:
    void Clear() {};
    bool LoadFromFile(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority) {
        this->Clear();
        akkordTexture.LoadFromFile(FileNamePNG, AkkordTexture::TextureType::PNG, SearchPriority);
        return true;
    };

    bool LoadFromMemory(const char* Buffer, int Size) {
        this->Clear();
        akkordTexture.LoadFromMemory(Buffer, Size, AkkordTexture::TextureType::PNG);
        return true;
    };
    bool Draw(bool Outline, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const std::vector<GLfloat>& UV, const std::vector<GLfloat>& squareVertices, const std::vector <GLushort>& Indices, GLfloat Scale, GLfloat Border, int Spread);
    AkkordPoint GetSize() { return akkordTexture.GetSize(); };
    ~SDFGLTexture() { Clear(); };
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
    void InitAtlasWH() {
        auto size = Texture.GetSize();
        atlasW = static_cast<float>(size.x);
        atlasH = static_cast<float>(size.y);
    };
public:
    bool LoadFromFile(const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority, int Spread) {
        this->Spread = Spread;
        Texture.LoadFromFile(FileNamePNG, SearchPriority);
        InitAtlasWH();
        return true;
    };
    bool LoadFromMemory(const char* Buffer, int Size, int Spread) {
        this->Spread = Spread;
        Texture.LoadFromMemory(Buffer, Size);
        InitAtlasWH();
        return true;
    };
    void SetColor(const AkkordColor& Color) { this->Color = Color; };
    void SetOutlineColor(const AkkordColor& OutlineColor) { this->OutlineColor = OutlineColor; };
    bool Draw(const AkkordRect& DestRect, const AkkordRect* SourceRect = nullptr);
    bool Flush();
    void SetAutoFlush(bool AutoFlush) { this->AutoFlush = AutoFlush; };
    void SetOutline(bool Outline) { this->Outline = Outline; };
    void SetBorder(float Border) { this->Border = Border; };
    void Clear() {
        UV.clear();
        squareVertices.clear();
        Indices.clear();
    };
    ~SDFTexture() {
        Clear();
        Texture.Clear();
    };
};

class SDFFont
{
    SDFGLTexture FontAtlas;
    unsigned int ScaleW, ScaleH, LineHeight, Spread;
    std::map<unsigned, SDFCharInfo> CharsMap;

    template <class fntStream>
    bool ParseFontMap(fntStream& fonsStream);

    bool ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority);
    void Clear() {
        CharsMap.clear();
        FontAtlas.Clear();
    };
public:
    enum struct AlignV : unsigned char { Top, Center, Bottom };
    enum struct AlignH : unsigned char { Left, Center, Right };

    ~SDFFont() { Clear(); };

    unsigned int GetAtlasW() { return ScaleW; };
    unsigned int GetAtlasH() { return ScaleH; };

    bool Load(const char* FileNameFNT, const char* FileNamePNG, BWrapper::FileSearchPriority SearchPriority, int Spread) {
        this->Clear();
        this->Spread = Spread;
        FontAtlas.LoadFromFile(FileNamePNG, SearchPriority);
        ParseFNTFile(FileNameFNT, BWrapper::FileSearchPriority::Assets);
        return true;
    };

    bool LoadAtlasFromMemory(int Spread, const char* Buffer, int Size) {
        this->Spread = Spread;
        return FontAtlas.LoadFromMemory(Buffer, Size);
    };

    bool LoadCharMapFromMemory(const char* Buffer, int Size);

    bool Draw(bool Outline, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const std::vector<GLfloat>& UV, const std::vector<GLfloat>& squareVertices, const std::vector <GLushort>& Indices, GLfloat Scale, GLfloat Border);
    bool GetCharInfo(unsigned Code, SDFCharInfo& ci) {
        auto res = CharsMap.find(Code);
        if (res != CharsMap.end())
        {
            ci = res->second;
            return true;
        }
        logError("Char with id=%u not found", Code);
        return false;
    };
    unsigned GetLineHeight() { return LineHeight; };
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

    AkkordPoint GetTextSizeByLine(const char* Text, std::vector<int>& VecSize);
public:
    SDFFontBuffer() : sdfFont{ nullptr } {};
    SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, const AkkordColor& Color) {
        this->Clear();
        sdfFont = Font;
        color = Color;
        Reserve(DigitsCount);
    };

    void SetFont(SDFFont* Font) { this->sdfFont = Font; };
    void SetScale(float Scale) { this->scaleX = this->scaleY = Scale; };
    void SetScale(float ScaleX, float ScaleY) { this->scaleX = ScaleX; this->scaleY = ScaleY; };

    void SetColor(const AkkordColor& Color) { this->color = Color; };
    void SetOutline(bool Outline) { this->outline = Outline; };
    void SetOutlineColor(const AkkordColor& OutlineColor) { this->outlineColor = OutlineColor; };
    void SetBorder(float BorderWidth) { this->Border = BorderWidth; };

    float GetScaleX() { return this->scaleX; };
    float GetScaleY() { return this->scaleY; };

    void SetRect(int W, int H) { this->rectW = W; this->rectH = H; };

    void SetAlignment(SDFFont::AlignH AlignH, SDFFont::AlignV AlignV) { this->alignH = AlignH; this->alignV = AlignV; };
    void SetAlignmentH(SDFFont::AlignH AlignH) { this->alignH = AlignH; };
    void SetAlignmentV(SDFFont::AlignV AlignV) { this->alignV = AlignV; };

    SDFFont::AlignH GetAlignH() { return this->alignH; };
    SDFFont::AlignV GetAlignV() { return this->alignV; };

    void Reserve(unsigned Count) {
        UV.reserve(Count * 4);
        squareVertices.reserve(Count * 4);
        Indices.reserve(Count * 6);
    };

    void Clear() {
        UV.clear();
        squareVertices.clear();
        Indices.clear();
    };
    void Flush();
    ~SDFFontBuffer() {
        Clear();
        sdfFont = nullptr;
    };
    // сейчас это int, возможно для этой функции сделать отдельный тип со float
    AkkordPoint GetTextSize(const char* Text) {
        std::vector<int> VecSize;
        return GetTextSizeByLine(Text, VecSize);
    };
    void        WrapText(const char* Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size);
    AkkordPoint DrawText(int X, int Y, const char* Text);
};

#endif // __AKK0RD_SDFFONT_H__
