#include "ttfwrapper.h"

bool TTFWrapper::loadFont(const char* Filename, int ptSize, BWrapper::FileSearchPriority FPriority) {
  fontSize = ptSize;
  font = std::unique_ptr<TTF_Font, void(*)(TTF_Font*)>(TTF_OpenFont(Filename, fontSize), TTF_CloseFont);
  return font ? true : false;
}

AkkordTexture TTFWrapper::createTexture(SDL_Surface* surface) {
    std::unique_ptr<SDL_Surface, void(*)(SDL_Surface*)> SDLSurface (surface, SDL_FreeSurface);
    std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> SDLTexture(SDL_CreateTextureFromSurface(BWrapper::GetActiveRenderer(), surface), SDL_DestroyTexture);
    return AkkordTexture(std::move(SDLTexture));
};

AkkordTexture* TTFWrapper::getCachedTextTextureBlended(const char* UTF8text, const AkkordColor& Color){
    const auto key = getKey(UTF8text, Color, textureType::Blended);
    const auto it = cachedTextures.find(key);
    if(it != std::end(cachedTextures)) {
        return &it->second;
    }

    auto insertResult = cachedTextures.insert({key, createTexture(TTF_RenderUTF8_Blended(font.get(), UTF8text, getTTFcolor(Color)))});
    return &insertResult.first->second;
}

AkkordTexture* TTFWrapper::getCachedTextTextureSolid(const char* UTF8text, const AkkordColor& Color){
    const auto key = getKey(UTF8text, Color, textureType::Solid);
    const auto it = cachedTextures.find(key);
    if(it != std::end(cachedTextures)) {
        return &it->second;
    }

    auto insertResult = cachedTextures.insert({key, createTexture(TTF_RenderUTF8_Solid(font.get(), UTF8text, getTTFcolor(Color)))});
    return &insertResult.first->second;
}


AkkordTexture* TTFWrapper::getCachedTextTextureShaded(const char* UTF8text, const AkkordColor& Color, const AkkordColor& BackColor){
    const auto key = getKey(UTF8text, Color, textureType::Shaded);
    const auto it = cachedTextures.find(key);
    if(it != std::end(cachedTextures)) {
        return &it->second;
    }

    auto insertResult = cachedTextures.insert({key, createTexture(TTF_RenderUTF8_Shaded(font.get(), UTF8text, getTTFcolor(Color), getTTFcolor(BackColor)))});
    return &insertResult.first->second;
}

int TTFWrapper::calculateFontSize(const char* UTF8text, int W, int H) {
    const auto curSize = getTextSize(UTF8text);
    return std::max(1, std::min(W * fontSize / curSize.x, H * fontSize / curSize.y));
};