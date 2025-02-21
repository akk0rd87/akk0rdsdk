#include "adrandomizer.h"

void AdRandomizer::OpenURL(BWrapper::OS OSCode, Apps AppId)
{
    logVerbose("Open URL OSCode=%d, %d", (int)OSCode, (int)AppId);

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

    case Apps::Sudoku:
        switch (OSCode)
        {
        case BWrapper::OS::iOS:
            //if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1147212126");
            //else                                   BWrapper::OpenURL("https://itunes.apple.com/app/id1147212126");
            BWrapper::OpenURL("https://itunes.apple.com/app/id1446860030");
            break;

        case BWrapper::OS::AndroidOS:
            BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.sudoku");
            break;

        default:
            break;
        }
        break;

    case Apps::DotLines:
        switch (OSCode)
        {
        case BWrapper::OS::iOS:
            //if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1473846186");
            //else                                   BWrapper::OpenURL("https://itunes.apple.com/app/id1473846186");
            BWrapper::OpenURL("https://itunes.apple.com/app/id1473846186");
            break;

        case BWrapper::OS::AndroidOS:
            BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.akk0rdsdk.dotlines");
            break;

        default:
            break;
        }
        break;

    case Apps::PirateBomb:
        switch (OSCode)
        {
        case BWrapper::OS::iOS:
            //if (Language == Locale::Lang::Russian) BWrapper::OpenURL("https://itunes.apple.com/ru/app/id1473846186");
            //else                                   BWrapper::OpenURL("https://itunes.apple.com/app/id1473846186");
            BWrapper::OpenURL("https://itunes.apple.com/app/id1482437026");
            break;

        case BWrapper::OS::AndroidOS:
            BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.akk0rdsdk.PirateBomb");
            break;

        default:
            break;
        }
        break;

    case Apps::CJCross:
        switch (OSCode)
        {
        case BWrapper::OS::iOS:
            BWrapper::OpenURL("https://itunes.apple.com/app/id1587839728");
            break;

        case BWrapper::OS::AndroidOS:
            BWrapper::OpenURL("https://play.google.com/store/apps/details?id=org.popapp.colornonograms");
            break;

        default:
            break;
        }
        break;

    case Apps::CJCrossRuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/org.popapp.colornonograms");
        break;

    case Apps::JCrossRuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/org.popapp.jc");
        break;

    case Apps::JDrawRuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/org.popapp.jdraw");
        break;

    case Apps::PirateBombRuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/org.akk0rdsdk.PirateBomb");
        break;

    case Apps::CFCrossRuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/org.popapp.color_fcross");
        break;

    case Apps::FCrossRuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/org.popapp.fcross");
        break;

    case Apps::WordsRu1RuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/com.popapp.WordsRu");
        break;

    case Apps::SudokuRuStore:
        BWrapper::OpenURL("https://www.rustore.ru/catalog/app/org.popapp.sudoku");
        break;
    }
}

void AdRandomizer::OpenURL(Apps AppId)
{
    const auto DeviceOs = BWrapper::GetDeviceOS();
    switch (DeviceOs)
    {
        // на Windows открываем ссылки по всем платформам
    case BWrapper::OS::Windows:
        AdRandomizer::OpenURL(BWrapper::OS::AndroidOS, AppId);
        AdRandomizer::OpenURL(BWrapper::OS::iOS, AppId);
        break;
        // на остальных платформах открываем согласно текущей платформе
    default:
        AdRandomizer::OpenURL(DeviceOs, AppId);
        break;
    }
}

void AdRandomizer::OpenPublisherAppstorePage()
{
    const char* iOS = "https://itunes.apple.com/developer/id945326562";
    const char* AndroidOS = "https://play.google.com/store/apps/developer?id=popapp.org";

    switch (BWrapper::GetDeviceOS()) {
    case BWrapper::OS::iOS:
        BWrapper::OpenURL(iOS);
        break;

    case BWrapper::OS::AndroidOS:
        BWrapper::OpenURL(AndroidOS);
        break;

    default:
        BWrapper::OpenURL(AndroidOS);
        BWrapper::OpenURL(iOS);
        break;
    }
};