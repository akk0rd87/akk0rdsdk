#pragma once
#ifndef __AKK0RD_ADRANDOMIZER_H__
#define __AKK0RD_ADRANDOMIZER_H__

#include "basewrapper.h"
class AdRandomizer
{
public:
    enum struct Apps : unsigned { CFCross, FCross, JCross, JDraw, WordsRu1, WordsRu2, WordsRu8, WordsUs/*, CJCross*/};
    static const int ImageWidth  = 100;
    static const int ImageHeight = 100;

    void           ExcludeApp(Apps AppId);
    unsigned       Randomize(unsigned Count);
    int            DrawImageByIndex(unsigned Index, const AkkordRect &Rect);
    void           OpenURLByIndex(unsigned Index);
    static void    OpenURL(BWrapper::OS OSCode, Apps AppId);

    AdRandomizer();
    ~AdRandomizer();

private:
    const char* AdRandomizerDir = "adrandomizer";

    class AppInfoStruct
    {
    public:
        AdRandomizer::Apps AppCode;
        std::string Path;
        std::vector<std::string> ImageFiles;
        AkkordTexture AdTexture;
        ~AppInfoStruct()
        {
            Path.clear();
            ImageFiles.clear();
            //logVerbose("AdRandomizer:: AppInfoStruct Destroy");
        }
    };

    void Clear();
    void InitRootPath();
    bool IsValidIndex(unsigned Index);

    std::string RootPath = std::string(AdRandomizerDir);
    void InitApps();
    bool LoadApp(AppInfoStruct& App, unsigned ImageIndex);
    std::vector<AppInfoStruct> CurApps;
};

/////////////////////////////////////////////////
////////// REALIZATION
/////////////////////////////////////////////////

/*
На винде, если нет папки assets/adrandomizer, лезем в дефолтный путь по SDK
На андроиде ищем adrandomizer в assets
На ios ищем в локальной папке adrandomizer
*/

// Для винды устанавливаем корневой путь
void AdRandomizer::InitRootPath()
{
#ifdef __WIN32__
    RootPath = std::string("assets/") + std::string(AdRandomizerDir);
    if (!BWrapper::DirExists(RootPath.c_str()))
        RootPath = BWrapper::GetEnvVariable("AKKORD_SDK_HOME") + "/resources/adimages/" + AdRandomizerDir;
#endif
}

void AdRandomizer::Clear()
{
    CurApps.clear();
}

AdRandomizer::AdRandomizer()
{
    Clear();
    InitRootPath();
    InitApps();
}

AdRandomizer::~AdRandomizer()
{
    Clear();
}

// Загрузка изображения для приложения
bool AdRandomizer::LoadApp(AppInfoStruct& App, unsigned ImageIndex)
{
    std::string ImagePath;
    ImagePath = RootPath + "/" + App.Path + "/" + App.ImageFiles[ImageIndex];

#ifdef __ANDROID__
    auto res = App.AdTexture.LoadFromFile(ImagePath.c_str(), AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);
#else
    auto res = App.AdTexture.LoadFromFile(ImagePath.c_str(), AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::FileSystem);
#endif

    return res;
}


bool AdRandomizer::IsValidIndex(unsigned Index)
{
    if (Index < CurApps.size()) return true;

    logError("AdRandomizer::IsValidIndex: Index %d does not exist CurApps with size %d", Index, CurApps.size());
    return false;
}

void AdRandomizer::ExcludeApp(Apps AppId)
{
    // Erase element from Vector
    for (unsigned i = 0; i < CurApps.size(); i++)
        if (AppId == CurApps[i].AppCode)
        {
            CurApps.erase(CurApps.begin() + i);
            break;
        }
}

unsigned AdRandomizer::Randomize(unsigned Count)
{
    BWrapper::RandomInit();

    auto Size = CurApps.size();
    if (Count > Size) Count = Size;

    while (Size > Count)
    {
        CurApps.erase(CurApps.begin() + (BWrapper::Random() % Size));
        --Size;
    }

    // грузим по ним картинки
    for (auto &v : CurApps)
    {
        LoadApp(v, BWrapper::Random() % v.ImageFiles.size());
    }

    return CurApps.size();
}

int AdRandomizer::DrawImageByIndex(unsigned Index, const AkkordRect &Rect)
{
    if (IsValidIndex(Index)) return CurApps[Index].AdTexture.Draw(Rect);
    return 1;
}



void AdRandomizer::OpenURLByIndex(unsigned Index)
{
    if (!IsValidIndex(Index)) return;

    auto DeviceOs = BWrapper::GetDeviceOS();
    switch (DeviceOs)
    {
        // на Windows открываем ссылки по всем платформам
        case BWrapper::OS::Windows:
            this->OpenURL(BWrapper::OS::AndroidOS, CurApps[Index].AppCode);
            this->OpenURL(BWrapper::OS::iOS      , CurApps[Index].AppCode);
            break;
        // на остальных платформах открываем согласно текущей платформе
        default:
            this->OpenURL(DeviceOs, CurApps[Index].AppCode);
            break;
    }
}

///////////////////////////////
///////////////////////////////
///////////////////////////////

void AdRandomizer::OpenURL(BWrapper::OS OSCode, Apps AppId)
{
    logVerbose("Open URL OSCode=%d, AppId", OSCode, AppId);

    auto Language = BWrapper::GetDeviceLanguage();

    switch (AppId)
    {
    case Apps::FCross:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1091290034");
                else                                        BWrapper::OpenURL("https://itunes.apple.com/en/app/id1091290034");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.fcross");
                break;
        }
        break;


    case Apps::JDraw:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1113501306");
                else                                        BWrapper::OpenURL("https://itunes.apple.com/en/app/id1113501306");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.jdraw");
                break;
        }
        break;

    case Apps::JCross:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/app/id1013587052");
                else                                        BWrapper::OpenURL("https://itunes.apple.com/app/id1013587052");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.jc");
                break;
        }
        break;

    case Apps::WordsRu1:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                BWrapper::OpenURL("https://itunes.apple.com/ru/app/id960409308");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=com.popapp.WordsRu");
                break;
        }
        break;

    case Apps::WordsRu2:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1080796090");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.WordsRu2");
                break;
        }
        break;

    case Apps::WordsRu8:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1112942939");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.sostav_slova_iz_bukv");
                break;
        }
        break;

    case Apps::WordsUs:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                BWrapper::OpenURL("https://itunes.apple.com/us/app/id969837546");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.WordsUS");
                break;
        }
        break;

    case Apps::CFCross:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1147212126");
                else                                        BWrapper::OpenURL("https://itunes.apple.com/app/id1147212126");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.color_fcross");
                break;
        }
        break;
    }
}

void AdRandomizer::InitApps()
{
    AppInfoStruct str;

    CurApps.push_back(str);
    auto size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::CFCross;
    CurApps[size].Path = "cfcross";
    CurApps[size].ImageFiles = { "ad_cfcross_1.png", "ad_cfcross_2.png", "ad_cfcross_3.png", "ad_cfcross_4.png", "ad_cfcross_5.png", "ad_cfcross_6.png" };

    CurApps.push_back(str);
    size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::FCross;
    CurApps[size].Path = "fcross";
    CurApps[size].ImageFiles = { "ad_fcross_1.png", "ad_fcross_2.png", "ad_fcross_3.png" };

    CurApps.push_back(str);
    size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::JCross;
    CurApps[size].Path = "jcross";
    CurApps[size].ImageFiles = { "ad_jcross_1.png", "ad_jcross_2.png", "ad_jcross_3.png" };

    CurApps.push_back(str);
    size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::JDraw;
    CurApps[size].Path = "jdraw";
    CurApps[size].ImageFiles = { "ad_jdraw_1.png", "ad_jdraw_2.png" };

    CurApps.push_back(str);
    size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::WordsRu1;
    CurApps[size].Path = "wordsru1";
    CurApps[size].ImageFiles = { "ad_words1_ru_1.png" };

    CurApps.push_back(str);
    size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::WordsRu2;
    CurApps[size].Path = "wordsru2";
    CurApps[size].ImageFiles = { "ad_words2_ru_1.png" };

    CurApps.push_back(str);
    size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::WordsRu8;
    CurApps[size].Path = "wordsru8";
    CurApps[size].ImageFiles = { "ad_words8_ru_1.png" };

    CurApps.push_back(str);
    size = CurApps.size() - 1;
    CurApps[size].AppCode = AdRandomizer::Apps::WordsUs;
    CurApps[size].Path = "wordsus";
    CurApps[size].ImageFiles = { "words_us_100.png" };


#if defined(_DEBUG) && defined(__WIN32__) // На винде в дебаге всегда проверяем, не протерялись ли ad-картинки
    for (auto &v : CurApps)
    {
        auto d = v.ImageFiles.size();
        for (decltype(d) j = 0; j < v.ImageFiles.size(); j++)
            LoadApp(v, j);
    }
#endif
}

#endif // __AKK0RD_ADRANDOMIZER_H__
