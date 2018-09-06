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
    void DrawSprite                     (IndexType SpriteIndex, AkkordRect Rect, unsigned char Flip = AkkordTexture::Flip::None, double Angle = 0, AkkordPoint* Point = nullptr);

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

/////////////////////////////////////////////////
////////// REALIZATION
/////////////////////////////////////////////////

void AtlasManager::ParseFile_LeshyLabsText(FileReader& fr, IndexType AtlasIndex)
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

AtlasManager::IndexType AtlasManager::LoadAtlas(const char* ListFilename, const char* TextureFilename, AtlasManager::AtlasType Type, BWrapper::FileSearchPriority ListSearchPriority, BWrapper::FileSearchPriority TextureSearchPriority)
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

bool AtlasManager::IsValidSpriteIndex(IndexType SpriteIndex)
{
    if (SpriteIndex < Sprites.size())
        return true;

    logError("AtlasManager: SpriteIndex %u doesn't exists", SpriteIndex);
    return false;
}

bool AtlasManager::IsValidAtlasIndex(IndexType AtlasIndex)
{
    if (AtlasIndex < AtlasTextureList.size())
        return true;

    logError("AtlasManager: AtlasIndex %u doesn't exists", AtlasIndex);
    return false;
}

AtlasManager::IndexType AtlasManager::GetIndexBySpriteName(IndexType AtlasIndex, const char* Imagename)
{
    if (IsValidAtlasIndex(AtlasIndex))
    {        
        for (decltype(Sprites.size()) i = 0; i < Sprites.size(); ++i)     
            if (Sprites[i].altasIndex == AtlasIndex && Imagename == Sprites[i].imageName)
                return i;        

        logError("AtlasManager::GetIndexBySpriteName Sprite '%s' with AtlasIndex = %u not found", Imagename, AtlasIndex);
    }

    return (std::numeric_limits<IndexType>::max)();
}

AkkordTexture* AtlasManager::GetAtlasBySprite(IndexType SpriteIndex)
{
	if (IsValidSpriteIndex(SpriteIndex))
		return AtlasTextureList[Sprites[SpriteIndex].altasIndex].get();

	return nullptr;
};

void AtlasManager::DrawSprite(IndexType SpriteIndex, AkkordRect Rect, unsigned char Flip, double Angle, AkkordPoint* Point)
{
    if (IsValidSpriteIndex(SpriteIndex))    
        AtlasTextureList[Sprites[SpriteIndex].altasIndex]->Draw(Rect, &Sprites[SpriteIndex].rect, Flip, Angle, Point);    
}

AkkordPoint AtlasManager::GetSpriteSize(IndexType SpriteIndex)
{
    if (IsValidSpriteIndex(SpriteIndex))
    {
        AkkordRect* rect = &Sprites[SpriteIndex].rect;
        return AkkordPoint(rect->x, rect->y);
    }
    return AkkordPoint(-1, -1);
}

AkkordRect AtlasManager::GetSpriteRect(IndexType SpriteIndex)
{
    if (IsValidSpriteIndex(SpriteIndex))    
        return Sprites[SpriteIndex].rect;
    
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
    //AtlasTextureList.push_back(std::move(std::unique_ptr<AkkordTexture>(new AkkordTexture())));
    AtlasTextureList.push_back(std::unique_ptr<AkkordTexture>(new AkkordTexture()));
}

#endif // __AKK0RD_ATLAS_MANAGER_H__
