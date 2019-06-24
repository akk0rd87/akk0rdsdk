#include "atlasmanager.h"

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

void AtlasManager::DrawSprite(IndexType SpriteIndex, const AkkordRect& Rect, unsigned char Flip, double Angle, AkkordPoint* Point)
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
    AtlasTextureList.emplace_back(std::make_unique<AkkordTexture>());
}
