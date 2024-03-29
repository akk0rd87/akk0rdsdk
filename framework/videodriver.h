#ifndef __AKK0RD_SDK_VIDEODRIVER_H__
#define __AKK0RD_SDK_VIDEODRIVER_H__

#include <unordered_map>
#include <utility>
#include <cmath>
#include "basewrapper.h"

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

class VideoBuffer;

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
    bool Draw(std::unique_ptr<VideoBuffer>& sdfVideoBuffer, bool Outline, const AkkordColor& FontColor, const AkkordColor& OutlineColor, float Scale, float Border, int Spread);
    AkkordPoint GetSize() const { return akkordTexture.GetSize(); };
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
    std::unique_ptr<VideoBuffer> videoBuffer;

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
    bool Draw(const AkkordRect& DestRect, const AkkordRect* SourceRect = nullptr) { return Draw(AkkordFRect(static_cast<float>(DestRect.x), static_cast<float>(DestRect.y), static_cast<float>(DestRect.w), static_cast<float>(DestRect.h)), SourceRect); };
    bool Draw(const AkkordFRect& DestRect, const AkkordRect* SourceRect = nullptr);
    bool Flush();
    void Clear();
    void SetAutoFlush(bool AutoFlush) { this->AutoFlush = AutoFlush; };
    void SetOutline(bool Outline) { this->Outline = Outline; };
    void SetBorder(float Border) { this->Border = Border; };
    AkkordColor GetColor() const { return Color; };
    AkkordColor GetOutlineColor() const { return OutlineColor; };

    ~SDFTexture();
    SDFTexture();
    SDFTexture(SDFTexture&& rhs); // Перемещающий: конструктор
    SDFTexture(const SDFTexture& rhs) = delete; // Копирующий: конструктор
    SDFTexture& operator= (const SDFTexture& rhs) = delete; // Оператор копирующего присваивания
    SDFTexture& operator= (SDFTexture&& rhs) = delete; // Оператор перемещающего присваивания
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
    std::unique_ptr<VideoBuffer> videoBuffer;

    std::pair<float, float> GetTextSizeByLine(const char* Text, float ParamScaleX, float ParamScaleY, std::vector<float>* VecSize) const;
public:

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

    void Clear();
    void Reserve(unsigned Count);
    void Flush();

    // сейчас это int, возможно для этой функции сделать отдельный тип со float
    AkkordPoint GetTextSize(const char* Text) const {
        const auto fp = GetTextSizeByLine(Text, scaleX, scaleY, nullptr);
        return AkkordPoint(static_cast<decltype(AkkordPoint::x)>(std::ceil(fp.first)), static_cast<decltype(AkkordPoint::y)>(std::ceil(fp.second)));
    };
    AkkordPoint GetTextSize(const std::string& Text) const { return  GetTextSize(Text.c_str()); };
    void        WrapText(const char* Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size);
    void        WrapText(const std::string& Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size) { WrapText(Text.c_str(), ScaleMutiplier, ResultString, UsedScale, Size); };
    AkkordPoint DrawText(float X, float Y, const char* Text);
    AkkordPoint DrawText(const AkkordPoint& Position, const char* Text) { return DrawText(static_cast<float>(Position.x), static_cast<float>(Position.y), Text); };
    AkkordPoint DrawText(const AkkordFPoint& Position, const char* Text) { return DrawText(Position.x, Position.y, Text); };

    AkkordPoint DrawText(float X, float Y, const std::string& Text) { return DrawText(X, Y, Text.c_str()); };
    AkkordPoint DrawText(const AkkordPoint& Position, const std::string& Text) { return DrawText(static_cast<float>(Position.x), static_cast<float>(Position.y), Text); };
    AkkordPoint DrawText(const AkkordFPoint& Position, const std::string& Text) { return DrawText(Position.x, Position.y, Text); };

    float GetTextScale(int W, int H, const char* Text) const {
        const float localScale = 1.0F;
        const auto fp = GetTextSizeByLine(Text, localScale, localScale, nullptr);
        return localScale * std::min(static_cast<float>(W) / fp.first, static_cast<float>(H) / fp.second);
    }
    float GetTextScale(int W, int H, const std::string& Text) const { return GetTextScale(W, H, Text.c_str()); }

    SDFFontBuffer();
    SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, const AkkordColor& Color);
    ~SDFFontBuffer();
    SDFFontBuffer(SDFFontBuffer&& rhs); // Перемещающий: конструктор
    SDFFontBuffer(const SDFFontBuffer& rhs) = delete; // Копирующий: конструктор
    SDFFontBuffer& operator= (const SDFFontBuffer& rhs) = delete; // Оператор копирующего присваивания
    SDFFontBuffer& operator= (SDFFontBuffer&& rhs) = delete; // Оператор перемещающего присваивания
};

class VideoDriver {
public:
    enum struct Feature : Uint8 { SDF = 1, SDF_Outline = 2, Gradient = 4 };
    static bool Init(const VideoDriver::Feature Features);
    static void ShutDown();
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
