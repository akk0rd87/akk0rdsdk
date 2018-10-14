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

    bool           Init();
    void           ExcludeApp(Apps AppId);
    unsigned       Randomize(unsigned Count);
    int            DrawImageByIndex(unsigned Index, const AkkordRect &Rect);
    void           OpenURLByIndex(unsigned Index);
    static void    OpenURL(Apps AppId);
	void           Clear();

    AdRandomizer();
    ~AdRandomizer();

private:
    const char* AdRandomizerDir = "adrandomizer";
	static void OpenURL_private(BWrapper::OS OSCode, Apps AppId);

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

    
    void InitRootPath();
    bool IsValidIndex(unsigned Index);

    std::string RootPath = std::string(AdRandomizerDir);
    void InitApps();
    bool LoadApp(AppInfoStruct& App, unsigned ImageIndex);
    std::vector<std::unique_ptr<AppInfoStruct>> CurApps;
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
}

bool AdRandomizer::Init()
{
    Clear();
    InitRootPath();
    InitApps();
    return true;
};

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
    for (decltype(CurApps.size()) i = 0; i < CurApps.size(); ++i)
        if (AppId == CurApps[i]->AppCode)
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

    for (; Size > Count; --Size)    
        CurApps.erase(CurApps.begin() + (BWrapper::Random() % Size));

    // грузим по ним картинки
    for (auto &v : CurApps)    
        LoadApp(*v, BWrapper::Random() % v->ImageFiles.size());    

    return CurApps.size();
}

int AdRandomizer::DrawImageByIndex(unsigned Index, const AkkordRect &Rect)
{
    if (IsValidIndex(Index)) return CurApps[Index]->AdTexture.Draw(Rect);
    return 1;
}

void AdRandomizer::OpenURLByIndex(unsigned Index)
{
    if (!IsValidIndex(Index)) return;
	AdRandomizer::OpenURL(CurApps[Index]->AppCode);
}

///////////////////////////////
///////////////////////////////
///////////////////////////////

void AdRandomizer::OpenURL_private(BWrapper::OS OSCode, Apps AppId)
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
                else                                        BWrapper::OpenURL("https://itunes.apple.com/app/id1091290034");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.fcross");
                break;

			default:
				break;
        }
        break;


    case Apps::JDraw:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1113501306");
                else                                        BWrapper::OpenURL("https://itunes.apple.com/app/id1113501306");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.jdraw");
                break;

			default:
				break;
        }
        break;

    case Apps::JCross:
        switch (OSCode)
        {
            case BWrapper::OS::iOS:
                if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1013587052");
                else                                        BWrapper::OpenURL("https://itunes.apple.com/app/id1013587052");
                break;

            case BWrapper::OS::AndroidOS:
                BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.jc");
                break;

			default:
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

			default:
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

			default:
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

			default:
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

			default:
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

			default:
				break;
        }
        break;
    }
}

void AdRandomizer::OpenURL(Apps AppId)
{
	auto DeviceOs = BWrapper::GetDeviceOS();
	switch (DeviceOs)
	{
		// на Windows открываем ссылки по всем платформам
		case BWrapper::OS::Windows:
			AdRandomizer::OpenURL_private(BWrapper::OS::AndroidOS, AppId);
			AdRandomizer::OpenURL_private(BWrapper::OS::iOS, AppId);
			break;
			// на остальных платформах открываем согласно текущей платформе
		default:
			AdRandomizer::OpenURL_private(DeviceOs, AppId);
			break;
	}
}

void AdRandomizer::InitApps()
{
    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::CFCross;
    CurApps.back()->Path = "cfcross";
    CurApps.back()->ImageFiles = { "ad_cfcross_1.png", "ad_cfcross_2.png", "ad_cfcross_3.png", "ad_cfcross_4.png", "ad_cfcross_5.png", "ad_cfcross_6.png" };

    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::FCross;
    CurApps.back()->Path = "fcross";
    CurApps.back()->ImageFiles = { "ad_fcross_1.png", "ad_fcross_2.png", "ad_fcross_3.png" };

    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::JCross;
    CurApps.back()->Path = "jcross";
    CurApps.back()->ImageFiles = { "ad_jcross_1.png", "ad_jcross_2.png", "ad_jcross_3.png" };

    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::JDraw;
    CurApps.back()->Path = "jdraw";
    CurApps.back()->ImageFiles = { "ad_jdraw_1.png", "ad_jdraw_2.png" };

    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::WordsRu1;
    CurApps.back()->Path = "wordsru1";
    CurApps.back()->ImageFiles = { "ad_words1_ru_1.png" };

    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::WordsRu2;
    CurApps.back()->Path = "wordsru2";
    CurApps.back()->ImageFiles = { "ad_words2_ru_1.png" };

    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::WordsRu8;
    CurApps.back()->Path = "wordsru8";
    CurApps.back()->ImageFiles = { "ad_words8_ru_1.png" };

    //CurApps.push_back(std::make_unique<AppInfoStruct>());
    CurApps.back()->AppCode = AdRandomizer::Apps::WordsUs;
    CurApps.back()->Path = "wordsus";
    CurApps.back()->ImageFiles = { "words_us_100.png" };


#if defined(_DEBUG) && defined(__WIN32__) // На винде в дебаге всегда проверяем, не протерялись ли ad-картинки
    for (auto &v : CurApps)            
        for (decltype(v->ImageFiles.size()) j = 0; j < v->ImageFiles.size(); j++)
            LoadApp(*v, j);    
#endif
}

#endif // __AKK0RD_ADRANDOMIZER_H__
