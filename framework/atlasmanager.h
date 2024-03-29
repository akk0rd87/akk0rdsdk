#pragma once
#ifndef __AKK0RD_SDK_ATLAS_MANAGER_H__
#define __AKK0RD_SDK_ATLAS_MANAGER_H__

#include "basewrapper.h"

class AtlasManager
{
public:
    using IndexType = std::vector<int>::size_type;

    enum struct AtlasType : unsigned char
    {
        LeshyLabsText /* https://www.leshylabs.com/apps/sstool/ */
    };

    // Load atlas with list file
    IndexType LoadAtlas(const char* ListFilename, const char* TextureFilename, AtlasManager::AtlasType Type, BWrapper::FileSearchPriority ListSearchPriority = BWrapper::FileSearchPriority::Assets, BWrapper::FileSearchPriority TextureSearchPriority = BWrapper::FileSearchPriority::Assets); // return the AtlasIndex

    // Get Sprite handle
    IndexType GetIndexBySpriteName(IndexType AtlasIndex, const char* SpriteName);

    // Draw Sprite
    void DrawSprite(IndexType SpriteIndex, const AkkordRect& Rect, AkkordTexture::Flip Flip = AkkordTexture::Flip::None, double Angle = 0, AkkordPoint* Point = nullptr);

    // Return Sprite postion in Atlas
    AkkordPoint GetSpriteSize(IndexType SpriteIndex);
    AkkordRect  GetSpriteRect(IndexType SpriteIndex);
    void        Clear() { Sprites.clear(); AtlasTextureList.clear(); };
    void        ReserveSprites(std::size_t Count) {
        Sprites.reserve(Count);
    };

    void        ReserveAtlases(std::size_t Count) { AtlasTextureList.reserve(Count); }

    AkkordTexture* GetAtlasBySprite(IndexType SpriteIndex);

    AtlasManager() = default;
    ~AtlasManager() = default;
    AtlasManager(AtlasManager&& rhs) = default; // Перемещающий: конструктор
    AtlasManager& operator= (AtlasManager&& rhs) = default; // Оператор перемещающего присваивания

    AtlasManager(const AtlasManager& rhs) = delete; // Копирующий: конструктор
    AtlasManager& operator= (const AtlasManager& rhs) = delete; // Оператор копирующего присваивания

private:

    // список текстур
    std::vector<AkkordTexture> AtlasTextureList;

    // структура спрайта
    struct SpriteStruct {
        AkkordRect rect;
        IndexType altasIndex;
        std::string imageName;
        SpriteStruct(IndexType AltasIndex, std::string&& Name) : rect(0, 0, 0, 0), altasIndex{ AltasIndex }, imageName{ std::move(Name) } {}
    };

    //список спрайтов в текстурах
    std::vector<SpriteStruct> Sprites;

    bool IsValidSpriteIndex(IndexType SpriteIndex);
    bool IsValidAtlasIndex(IndexType AtlasIndex);
    void ParseFile_LeshyLabsText(FileReader& fr, IndexType AtlasIndex);
};

#endif // __AKK0RD_SDK_ATLAS_MANAGER_H__
