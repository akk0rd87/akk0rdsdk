#include "ttfwrapper.h"

bool TTFWrapper::loadFont(const char* Filename, int ptSize, BWrapper::FileSearchPriority FPriority) {
    fontSize = ptSize;
    font = std::unique_ptr<TTF_Font, void(*)(TTF_Font*)>(TTF_OpenFont(Filename, fontSize), TTF_CloseFont);
    return font ? true : false;
}

const AkkordTexture* TTFWrapper::createTexture(const std::string& key, SDL_Surface* surface) {
    std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> SDLSurface(surface, SDL_FreeSurface);
    AkkordTexture at;
    at.CreateFromSurface(surface);
    auto insertResult = cachedTextures.insert({ key, std::move(at) });
    logDebug("cache size %d", cachedTextures.size());
    return &insertResult.first->second;
};

const AkkordTexture* TTFWrapper::getCachedTextTextureBlended(const char* UTF8text, const AkkordColor& Color) {
    const auto key = getKey(UTF8text, Color, textureType::Blended);
    const auto it = cachedTextures.find(key);
    if (it != std::end(cachedTextures)) {
        return &it->second;
    }

    return createTexture(key, TTF_RenderUTF8_Blended(font.get(), UTF8text, getTTFcolor(Color)));
}

const AkkordTexture* TTFWrapper::getCachedTextTextureSolid(const char* UTF8text, const AkkordColor& Color) {
    const auto key = getKey(UTF8text, Color, textureType::Solid);
    const auto it = cachedTextures.find(key);
    if (it != std::end(cachedTextures)) {
        return &it->second;
    }

    return createTexture(key, TTF_RenderUTF8_Solid(font.get(), UTF8text, getTTFcolor(Color)));
}

const AkkordTexture* TTFWrapper::getCachedTextTextureShaded(const char* UTF8text, const AkkordColor& Color, const AkkordColor& BackColor) {
    const auto key = getKey(UTF8text, Color, textureType::Shaded);
    const auto it = cachedTextures.find(key);
    if (it != std::end(cachedTextures)) {
        return &it->second;
    }

    return createTexture(key, TTF_RenderUTF8_Shaded(font.get(), UTF8text, getTTFcolor(Color), getTTFcolor(BackColor)));
}

int TTFWrapper::calculateFontSize(const char* UTF8text, int W, int H) {
    const auto curSize = getTextSize(UTF8text);
    return std::max(1, std::min(W * fontSize / curSize.x, H * fontSize / curSize.y));
};