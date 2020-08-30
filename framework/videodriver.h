#ifndef __AKK0RD_SDK_VIDEODRIVER_H__
#define __AKK0RD_SDK_VIDEODRIVER_H__

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

    SDFGLTexture() = default;
    SDFGLTexture(const SDFGLTexture& rhs) = delete; // Копирующий: конструктор
    SDFGLTexture& operator= (const SDFGLTexture& rhs) = delete; // Оператор копирующего присваивания
    SDFGLTexture(SDFGLTexture&& rhs) = default; // Перемещающий: конструктор
    SDFGLTexture& operator= (SDFGLTexture&& rhs) = default; // Оператор перемещающего присваивания
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
    void SetColor(Uint8 R, Uint8 G, Uint8 B) { this->Color = AkkordColor(R, G, B); };
    void SetColor(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { this->Color = AkkordColor(R, G, B, A); };
    void SetOutlineColor(const AkkordColor& OutlineColor) { this->OutlineColor = OutlineColor; };
    void SetOutlineColor(Uint8 R, Uint8 G, Uint8 B) { this->OutlineColor = AkkordColor(R, G, B); };
    void SetOutlineColor(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { this->OutlineColor = AkkordColor(R, G, B, A); };
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

    SDFTexture() = default;
    SDFTexture(const SDFTexture& rhs) = delete; // Копирующий: конструктор
    SDFTexture& operator= (const SDFTexture& rhs) = delete; // Оператор копирующего присваивания
    SDFTexture(SDFTexture&& rhs) = default; // Перемещающий: конструктор
    SDFTexture& operator= (SDFTexture&& rhs) = default; // Оператор перемещающего присваивания
};

class SDFFont
{
public:
    friend class SDFFontBuffer;
    enum struct AlignV : unsigned char { Top, Center, Bottom };
    enum struct AlignH : unsigned char { Left, Center, Right };

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

    void Clear() {
        CharsMap.clear();
        FontAtlas.Clear();
    };

    SDFFont() = default;
    SDFFont(const SDFFont& rhs) = delete; // Копирующий: конструктор
    SDFFont& operator= (const SDFFont& rhs) = delete; // Оператор копирующего присваивания
    SDFFont(SDFFont&& rhs) = default; // Перемещающий: конструктор
    SDFFont& operator= (SDFFont&& rhs) = default; // Оператор перемещающего присваивания
private:
    mutable SDFGLTexture FontAtlas;
    struct SDFCharInfo {
        float x, y, w, h, xoffset, yoffset, xadvance;
    };
    float LineHeight = 0.0F, ScaleW = 0.0F, ScaleH = 0.0F;
    unsigned int Spread = 0;
    std::unordered_map<unsigned, SDFFont::SDFCharInfo> CharsMap;

    template <class fntStream>
    bool ParseFontMap(fntStream& fonsStream);

    float GetAtlasW() const { return ScaleW; };
    float GetAtlasH() const { return ScaleH; };

    bool GetCharInfo(unsigned Code, SDFFont::SDFCharInfo& ci) const {
        const auto res = CharsMap.find(Code);
        if (res != CharsMap.end()) {
            ci = res->second;
            return true;
        }
        logError("Char with id=%u not found", Code);
        return false;
    };
    float GetLineHeight() const { return LineHeight; };

    bool Draw(bool Outline, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const std::vector<GLfloat>& UV, const std::vector<GLfloat>& squareVertices, const std::vector <GLushort>& Indices, GLfloat Scale, GLfloat Border) const {
        FontAtlas.Draw(Outline, FontColor, OutlineColor, UV, squareVertices, Indices, Scale, Border, Spread);
        return true;
    };

    bool ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority);
};

// Для рисования всегда указывать левую верхнюю точку (удобно для разгаданных слов в "составь слова")
class SDFFontBuffer
{
    float scaleX = 1.0F;
    float scaleY = 1.0F;
    float Border = 0.0F;

    SDFFont* sdfFont = nullptr;
    float rectW = -1.0F, rectH = -1.0F;

    bool outline = false;

    SDFFont::AlignH alignH = SDFFont::AlignH::Center;
    SDFFont::AlignV alignV = SDFFont::AlignV::Center;

    AkkordColor color, outlineColor;

    std::vector<GLfloat>UV;
    std::vector<GLfloat>squareVertices;
    std::vector<GLushort>Indices;

    AkkordPoint GetTextSizeByLine(const char* Text, std::vector<float>* VecSize) const;
public:
    SDFFontBuffer() : sdfFont{ nullptr } {};
    SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, const AkkordColor& Color) {
        this->Clear();
        sdfFont = Font;
        color = Color;
        Reserve(DigitsCount);
    };

    void SetFont(SDFFont* Font) { this->sdfFont = Font; };
    void SetFont(SDFFont& Font) { this->sdfFont = &Font; };
    void SetScale(float Scale) { this->scaleX = this->scaleY = Scale; };
    void SetScale(float ScaleX, float ScaleY) { this->scaleX = ScaleX; this->scaleY = ScaleY; };

    void SetColor(const AkkordColor& Color) { this->color = Color; };
    void SetColor(Uint8 R, Uint8 G, Uint8 B) { this->color = AkkordColor(R, G, B); };
    void SetColor(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { this->color = AkkordColor(R, G, B, A); };
    void SetOutline(bool Outline) { this->outline = Outline; };
    void SetOutlineColor(const AkkordColor& OutlineColor) { this->outlineColor = OutlineColor; };
    void SetOutlineColor(Uint8 R, Uint8 G, Uint8 B) { this->outlineColor = AkkordColor(R, G, B); };
    void SetOutlineColor(Uint8 R, Uint8 G, Uint8 B, Uint8 A) { this->outlineColor = AkkordColor(R, G, B, A); };
    void SetBorder(float BorderWidth) { this->Border = BorderWidth; };

    float GetScaleX() const { return this->scaleX; };
    float GetScaleY() const { return this->scaleY; };

    void SetRect(int W, int H) { this->rectW = static_cast<decltype(rectW)>(W); this->rectH = static_cast<decltype(rectH)>(H); };
    void SetRect(const AkkordPoint& Rect) { SetRect(Rect.x, Rect.y); };
    AkkordPoint GetRect() const { return AkkordPoint(static_cast<int>(rectW), static_cast<int>(rectH)); }

    void SetAlignment(SDFFont::AlignH AlignH, SDFFont::AlignV AlignV) { this->alignH = AlignH; this->alignV = AlignV; };
    void SetAlignmentH(SDFFont::AlignH AlignH) { this->alignH = AlignH; };
    void SetAlignmentV(SDFFont::AlignV AlignV) { this->alignV = AlignV; };

    SDFFont::AlignH GetAlignH() const { return this->alignH; };
    SDFFont::AlignV GetAlignV() const { return this->alignV; };

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

    void Flush() {
        if (Indices.size() > 0) {
            sdfFont->Draw(this->outline, this->color, this->outlineColor, UV, squareVertices, Indices, (GLfloat)this->scaleX, (GLfloat)this->Border);
        }
        Clear();
    };

    // сейчас это int, возможно для этой функции сделать отдельный тип со float
    AkkordPoint GetTextSize(const char* Text) const { return GetTextSizeByLine(Text, nullptr); };
    AkkordPoint GetTextSize(const std::string& Text) const { return GetTextSizeByLine(Text.c_str(), nullptr); };
    void        WrapText(const char* Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size);
    void        WrapText(const std::string& Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size) { WrapText(Text.c_str(), ScaleMutiplier, ResultString, UsedScale, Size); };
    AkkordPoint DrawText(int X, int Y, const char* Text);
    AkkordPoint DrawText(const AkkordPoint& Position, const char* Text) { return DrawText(Position.x, Position.y, Text); };

    AkkordPoint DrawText(int X, int Y, const std::string& Text) { return DrawText(X, Y, Text.c_str()); };
    AkkordPoint DrawText(const AkkordPoint& Position, const std::string& Text) { return DrawText(Position.x, Position.y, Text.c_str()); };

    SDFFontBuffer(const SDFFontBuffer& rhs) = delete; // Копирующий: конструктор
    SDFFontBuffer& operator= (const SDFFontBuffer& rhs) = delete; // Оператор копирующего присваивания
    SDFFontBuffer(SDFFontBuffer&& rhs) = default; // Перемещающий: конструктор
    SDFFontBuffer& operator= (SDFFontBuffer&& rhs) = default; // Оператор перемещающего присваивания
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

#endif // __AKK0RD_SDK_VIDEODRIVER_H__
