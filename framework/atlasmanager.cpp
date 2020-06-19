#include "atlasmanager.h"

void AtlasManager::ParseFile_LeshyLabsText(FileReader& fr, IndexType AtlasIndex)
{
    std::string line;
    while (fr.ReadLine(line)) {
        if (!line.empty()) {
            const char* p = line.c_str();
            while (*p && *p != ',') {
                ++p;
            }
            Sprites.emplace_back(AtlasIndex, std::string(line.c_str(), p - line.c_str()));
            auto& s = Sprites.back();

            const auto getIntValue = [](const char*& p) {
                int v{ 0 };
                for (++p; ('0' <= (*p) && (*p) <= '9'); ++p) {
                    v *= 10;
                    v += static_cast<decltype(v)>(*p - '0');
                }
                return v;
            };

            s.rect.x = getIntValue(p);
            s.rect.y = getIntValue(p);
            s.rect.w = getIntValue(p);
            s.rect.h = getIntValue(p);
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