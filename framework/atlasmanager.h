#pragma once
#ifndef __AKK0RD_ATLAS_MANAGER_H__
#define __AKK0RD_ATLAS_MANAGER_H__

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
    IndexType LoadAtlas (const char* ListFilename, const char* TextureFilename, AtlasManager::AtlasType Type, BWrapper::FileSearchPriority ListSearchPriority = BWrapper::FileSearchPriority::Assets, BWrapper::FileSearchPriority TextureSearchPriority = BWrapper::FileSearchPriority::Assets); // return the AtlasIndex

    // Get Sprite handle
    IndexType GetIndexBySpriteName (IndexType AtlasIndex, const char* SpriteName);

    // Draw Sprite
    void DrawSprite                     (IndexType SpriteIndex, const AkkordRect& Rect, unsigned char Flip = AkkordTexture::Flip::None, double Angle = 0, AkkordPoint* Point = nullptr);

    // Return Sprite postion in Atlas
    AkkordPoint GetSpriteSize           (IndexType SpriteIndex);
    AkkordRect  GetSpriteRect           (IndexType SpriteIndex);
    void        Clear();

	AkkordTexture* GetAtlasBySprite(IndexType SpriteIndex);

    AtlasManager();
    ~AtlasManager();

private:

    // список текстур
    std::vector<std::unique_ptr<AkkordTexture>> AtlasTextureList;

    // структура спрайта
    struct SpriteStruct{
        std::string imageName;
        //int x, y, w, h;
        AkkordRect rect;
        IndexType altasIndex;
    };

    //список спрайтов в текстурах
    std::vector<SpriteStruct> Sprites;

    bool IsValidSpriteIndex(IndexType SpriteIndex);
    bool IsValidAtlasIndex(IndexType AtlasIndex);
    void ParseFile_LeshyLabsText(FileReader& fr, IndexType AtlasIndex);
    void AddTexture();
};

#endif // __AKK0RD_ATLAS_MANAGER_H__
