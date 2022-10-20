#ifndef __AKK0RD_SDK_TTF_WRAPPER__
#define __AKK0RD_SDK_TTF_WRAPPER__

#include <unordered_map>
#include "basewrapper.h"
#include "SDL_ttf.h"

class TTFWrapper {
public:
    enum struct AlignH : unsigned char { Left, Center, Right };
    enum struct AlignV : unsigned char { Top, Center, Bottom };

    TTFWrapper() : font(nullptr, nullptr), fontSize(0) {}
    bool loadFont(const char* Filename, int ptSize, BWrapper::FileSearchPriority FPriority);
    const AkkordTexture* getCachedTextTextureBlended(const char* UTF8text, const AkkordColor& Color);
    const AkkordTexture* getCachedTextTextureSolid(const char* UTF8text, const AkkordColor& Color);
    const AkkordTexture* getCachedTextTextureShaded(const char* UTF8text, const AkkordColor& Color, const AkkordColor& BackColor);

    AkkordPoint getTextSize(const char* UTF8text) {
        AkkordPoint pt(0, 0);
        TTF_SizeUTF8(font.get(), UTF8text, &pt.x, &pt.y);
        return pt;
    };

    int getSize() const { return fontSize; }
    bool changeSize(int ptSize) {
        if (fontSize != ptSize) {
            fontSize = ptSize;
            TTF_SetFontSize(font.get(), fontSize);
            return true;
        }
        return false;
    };

    int calculateFontSize(const char* UTF8text, int W, int H);
    void setRect(int W, int H) { textRect.x = W; textRect.y = H; }
    void drawText(const AkkordTexture* texture, int X, int Y) {
        if (texture) {
            drawText(*texture, X, Y);
        }
    }

    void drawText(const AkkordTexture& texture, int X, int Y) {
        const auto size = texture.GetSize();

        switch (alignH)
        {
        case AlignH::Center:
            X += (textRect.GetX() - size.GetX()) / 2;
            break;
        case AlignH::Right:
            X += (textRect.GetX() - size.GetX());
            break;
        default: // в остальных случаях ничего не делаем, координату не меняем
            break;
        }

        switch (alignV)
        {
        case AlignV::Center:
            Y += ((textRect.GetY() - size.GetY()) / 2);
            break;
        case AlignV::Bottom:
            Y += (textRect.GetY() - size.GetY());
            break;
        default: // в остальных случаях ничего не делаем, координату не меняем
            break;
        }

        texture.Draw(AkkordRect(X, Y, size.GetX(), size.GetY()));
    }

    void SetAlignment(AlignH AlignH, AlignV AlignV) { this->alignH = AlignH; this->alignV = AlignV; };
    void SetAlignmentH(AlignH AlignH) { this->alignH = AlignH; };
    void SetAlignmentV(AlignV AlignV) { this->alignV = AlignV; };

    AlignH GetAlignH() const { return this->alignH; };
    AlignV GetAlignV() const { return this->alignV; };
private:
    std::unordered_map<std::string, AkkordTexture> cachedTextures;
    std::unique_ptr<TTF_Font, void(*)(TTF_Font*)> font;
    AkkordPoint textRect;
    int fontSize;
    AlignH alignH = AlignH::Center;
    AlignV alignV = AlignV::Center;

    enum struct textureType : unsigned { Blended, Solid, Shaded };

    std::string getKey(const char* text, const AkkordColor& Color, textureType tType) { return std::to_string(fontSize) + '-' + std::to_string(static_cast<int>(tType)) + '-' + text; }
    SDL_Color getTTFcolor(const AkkordColor& Color) { return SDL_Color{ Color.GetR(), Color.GetG(), Color.GetB(), 0 }; }
    const AkkordTexture* createTexture(const std::string& key, SDL_Surface* surface);
};
#endif