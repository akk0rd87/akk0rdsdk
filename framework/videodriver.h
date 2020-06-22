#ifndef __AKK0RD_VIDEODRIVER_H__
#define __AKK0RD_VIDEODRIVER_H__

#include <unordered_map>
#include "basewrapper.h"
#include"openglesdriver.h"

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

    SDFGLTexture() {};
    SDFGLTexture(const SDFGLTexture& rhs) = delete; // Копирующий: конструктор
    SDFGLTexture(SDFGLTexture&& rhs) = delete; // Перемещающий: конструктор
    SDFGLTexture& operator= (const SDFGLTexture& rhs) = delete; // Оператор копирующего присваивания
    SDFGLTexture& operator= (SDFGLTexture&& rhs) = delete; // Оператор перемещающего присваивания
};

class SDFTexture
{
    SDFGLTexture Texture;
    AkkordColor Color, OutlineColor;
    int Spread = 0;

    std::vector<GLfloat>UV;
    std::vector<GLfloat>squareVertices;
    std::vector<GLushort>Indices;

    bool AutoFlush = false, Outline = false;
    float atlasW = 0.0f, atlasH = 0.0f, Scale = 0.0f, Border = 0.0f;
    void InitAtlasWH() {
        const auto size = Texture.GetSize();
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

    SDFTexture() {};
    SDFTexture(const SDFTexture& rhs) = delete; // Копирующий: конструктор
    SDFTexture(SDFTexture&& rhs) = delete; // Перемещающий: конструктор
    SDFTexture& operator= (const SDFTexture& rhs) = delete; // Оператор копирующего присваивания
    SDFTexture& operator= (SDFTexture&& rhs) = delete; // Оператор перемещающего присваивания
};

class SDFFont
{
public:
    enum struct AlignV : unsigned char { Top, Center, Bottom };
    enum struct AlignH : unsigned char { Left, Center, Right };
    unsigned int GetAtlasW() { return ScaleW; };
    unsigned int GetAtlasH() { return ScaleH; };

    struct SDFCharInfo {
        unsigned int id, x, y, w, h;
        int xoffset, yoffset, xadvance;
    };

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
    bool GetCharInfo(unsigned Code, SDFFont::SDFCharInfo& ci) {
        const auto res = CharsMap.find(Code);
        if (res != CharsMap.end()) {
            ci = res->second;
            return true;
        }
        logError("Char with id=%u not found", Code);
        return false;
    };
    unsigned GetLineHeight() { return LineHeight; };

    SDFFont() {};
    ~SDFFont() { Clear(); };
    SDFFont(const SDFFont& rhs) = delete; // Копирующий: конструктор
    SDFFont(SDFFont&& rhs) = delete; // Перемещающий: конструктор
    SDFFont& operator= (const SDFFont& rhs) = delete; // Оператор копирующего присваивания
    SDFFont& operator= (SDFFont&& rhs) = delete; // Оператор перемещающего присваивания
private:
    SDFGLTexture FontAtlas;
    unsigned int ScaleW = 0, ScaleH = 0, LineHeight = 0, Spread = 0;
    std::unordered_map<unsigned, SDFFont::SDFCharInfo> CharsMap;

    template <class fntStream>
    bool ParseFontMap(fntStream& fonsStream);

    bool ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority);
    void Clear() {
        CharsMap.clear();
        FontAtlas.Clear();
    };
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

    AkkordPoint GetTextSizeByLine(const char* Text, std::vector<int>* VecSize);
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
        return GetTextSizeByLine(Text, nullptr);
    };
    void        WrapText(const char* Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size);
    AkkordPoint DrawText(int X, int Y, const char* Text);

    SDFFontBuffer(const SDFFontBuffer& rhs) = delete; // Копирующий: конструктор
    SDFFontBuffer(SDFFontBuffer&& rhs) = default; // Перемещающий: конструктор
    SDFFontBuffer& operator= (const SDFFontBuffer& rhs) = delete; // Оператор копирующего присваивания
    SDFFontBuffer& operator= (SDFFontBuffer&& rhs) = delete; // Оператор перемещающего присваивания
};

class VideoDriver {
public:
    enum struct Feature : Uint8 { SDF = 1, SDF_Outline = 2, Gradient = 4 };
    static bool Init(const VideoDriver::Feature Features);
    static bool DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1);
};

inline VideoDriver::Feature operator | (VideoDriver::Feature a, VideoDriver::Feature b) {
    return static_cast<VideoDriver::Feature>(static_cast<Uint8>(a) | static_cast<Uint8>(b));
}

inline VideoDriver::Feature operator & (VideoDriver::Feature a, VideoDriver::Feature b) {
    return static_cast<VideoDriver::Feature>(static_cast<Uint8>(a) & static_cast<Uint8>(b));
}

inline VideoDriver::Feature& operator |= (VideoDriver::Feature& a, VideoDriver::Feature b) {
    a = a | b;
    return a;
}

inline bool operator!(VideoDriver::Feature a) {
    return (static_cast<Uint8>(a) == 0);
}

#endif // __AKK0RD_VIDEODRIVER_H__
