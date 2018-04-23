#pragma once
#ifndef __AKK0RD_ATLAS_MANAGER_H__
#define __AKK0RD_ATLAS_MANAGER_H__

#include "basewrapper.h"

class AtlasManager
{
public:
    enum struct AtlasType : unsigned char
    {
        LeshyLabsText /* https://www.leshylabs.com/apps/sstool/ */
    };

    // Load atlas with list file
    unsigned LoadAtlas                  (const char* ListFilename, const char* TextureFilename, AtlasManager::AtlasType Type, BWrapper::FileSearchPriority ListSearchPriority = BWrapper::FileSearchPriority::Assets, BWrapper::FileSearchPriority TextureSearchPriority = BWrapper::FileSearchPriority::Assets); // return the AtlasIndex

    // Get Sprite handle
    unsigned GetIndexBySpriteName       (unsigned AtlasIndex, const char* SpriteName);

    // Draw Sprite
    void DrawSprite                     (unsigned SpriteIndex, AkkordRect Rect, unsigned char Flip = AkkordTexture::Flip::None, double Angle = 0, AkkordPoint* Point = nullptr);

    // Return Sprite postion in Atlas
    AkkordPoint GetSpriteSize           (unsigned SpriteIndex);
    AkkordRect  GetSpriteRect           (unsigned SpriteIndex);
    void        Clear();

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
        unsigned altasIndex;
    };

    //список спрайтов в текстурах
    std::vector<SpriteStruct> Sprites;

    bool IsValidSpriteIndex(unsigned SpriteIndex);
    bool IsValidAtlasIndex(unsigned AtlasIndex);
    void ParseFile_LeshyLabsText(FileReader& fr, unsigned AtlasIndex);
    void AddTexture();
};

/////////////////////////////////////////////////
////////// REALIZATION
/////////////////////////////////////////////////

void AtlasManager::ParseFile_LeshyLabsText(FileReader& fr, unsigned AtlasIndex)
{
    std::string line;
    SpriteStruct sprite;
    while (fr.ReadLine(line))
    {
        if (line.size() > 0)
        {
            Sprites.push_back(sprite);
            auto idx = Sprites.size() - 1;

            decltype(line.find(',')) lpos = 0;
            decltype(line.find(',')) rpos = 0;

            rpos = line.find(',', lpos);
            Sprites[idx].imageName = std::string(line, lpos, rpos);

            lpos = rpos + 1;
            rpos = line.find(',', lpos);
            Sprites[idx].rect.x = BWrapper::Str2Num(std::string(line, lpos, rpos).c_str());

            lpos = rpos + 1;
            rpos = line.find(',', lpos);
            Sprites[idx].rect.y = BWrapper::Str2Num(std::string(line, lpos, rpos).c_str());

            lpos = rpos + 1;
            rpos = line.find(',', lpos);
            Sprites[idx].rect.w = BWrapper::Str2Num(std::string(line, lpos, rpos).c_str());

            lpos = rpos + 1;
            rpos = line.find(',', lpos);
            Sprites[idx].rect.h = BWrapper::Str2Num(std::string(line, lpos).c_str());

            Sprites[idx].altasIndex = AtlasIndex;

            logVerbose("LoadAtlas Parse Line: Image = %s, x=%d, y=%d, w=%d, h=%d", Sprites[idx].imageName.c_str(), Sprites[idx].rect.x, Sprites[idx].rect.y, Sprites[idx].rect.w, Sprites[idx].rect.h);
        }
    }
}

unsigned AtlasManager::LoadAtlas(const char* ListFilename, const char* TextureFilename, AtlasManager::AtlasType Type, BWrapper::FileSearchPriority ListSearchPriority, BWrapper::FileSearchPriority TextureSearchPriority)
{
    AddTexture();
    auto index = AtlasTextureList.size() - 1;
    AtlasTextureList[index]->LoadFromFile(TextureFilename, AkkordTexture::TextureType::PNG, TextureSearchPriority);

    FileReader fr;
    if (fr.Open(ListFilename, ListSearchPriority))
    {
        switch (Type)
        {
            case AtlasManager::AtlasType::LeshyLabsText:
                ParseFile_LeshyLabsText(fr, index);
                break;
            default:
                logError("LoadAtlas File = %s Unknown atlas typ ", ListFilename);
                break;
        }
    }
    fr.Close();

    return index;
};

bool AtlasManager::IsValidSpriteIndex(unsigned SpriteIndex)
{
    if (SpriteIndex < Sprites.size())
        return true;

    logError("AtlasManager: SpriteIndex %u does't exists", SpriteIndex);
    return false;
}

bool AtlasManager::IsValidAtlasIndex(unsigned AtlasIndex)
{
    if (AtlasIndex < AtlasTextureList.size())
        return true;

    logError("AtlasManager: AtlasIndex %u does't exists", AtlasIndex);
    return false;
}

unsigned AtlasManager::GetIndexBySpriteName(unsigned AtlasIndex, const char* Imagename)
{
    if (IsValidAtlasIndex(AtlasIndex))
    {
        auto size = Sprites.size();
        for (decltype(size) i = 0; i < size; i++)
        {
            if (Sprites[i].altasIndex == AtlasIndex && Imagename == Sprites[i].imageName)
                return i;
        }

        logError("AtlasManager::GetIndexBySpriteName Sprite '%s' with AtlasIndex = %u not found", Imagename, AtlasIndex);
    }

    return GConstants::unsigned_max();
}

void AtlasManager::DrawSprite(unsigned SpriteIndex, AkkordRect Rect, unsigned char Flip, double Angle, AkkordPoint* Point)
{
    if (IsValidSpriteIndex(SpriteIndex))
    {
        AtlasTextureList[Sprites[SpriteIndex].altasIndex]->Draw(Rect, &Sprites[SpriteIndex].rect, Flip, Angle, Point);
    }
}

AkkordPoint AtlasManager::GetSpriteSize(unsigned SpriteIndex)
{
    if (IsValidSpriteIndex(SpriteIndex))
    {
        AkkordRect* rect = &Sprites[SpriteIndex].rect;
        return AkkordPoint(rect->x, rect->y);
    }
    return AkkordPoint(-1, -1);
}

AkkordRect AtlasManager::GetSpriteRect(unsigned SpriteIndex)
{
    if (IsValidSpriteIndex(SpriteIndex))
    {
        return Sprites[SpriteIndex].rect;
    }
    return AkkordRect(-1, -1, -1, -1);

}

void AtlasManager::Clear()
{
    Sprites.clear();
    AtlasTextureList.clear();
}

AtlasManager::AtlasManager()
{
    this->Clear();
}

AtlasManager::~AtlasManager()
{
    this->Clear();
}

void AtlasManager::AddTexture()
{
    AtlasTextureList.push_back(std::move(std::unique_ptr<AkkordTexture>(new AkkordTexture())));
}

#endif // __AKK0RD_ATLAS_MANAGER_H__
