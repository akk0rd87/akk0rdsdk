#include "adrandomizer.h"

void AdRandomizer::OpenURL_private(BWrapper::OS OSCode, Apps AppId)
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

void AdRandomizer::OpenPublisherAppstorePage()
{
    if (BWrapper::GetDeviceOS() == BWrapper::OS::iOS)
        BWrapper::OpenURL("https://itunes.apple.com/developer/id945326562");
    else
        BWrapper::OpenURL("https://play.google.com/store/apps/developer?id=popapp.org");
};