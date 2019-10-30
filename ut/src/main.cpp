#include "ut_defines.h"
#include "testfs.h"
#include "testconfigmanager.h"

Statistic statistic;

bool Init()
{
    if (!BWrapper::Init(SDL_INIT_VIDEO)) return false;

    /*
    auto Window = BWrapper::CreateRenderWindow("Hello World!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_RESIZABLE);

    if (!Window) return false;

    BWrapper::SetActiveWindow(Window);
    BWrapper::SetWindowResizable(true);
    auto Renderer = BWrapper::CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!Renderer) return false;

    BWrapper::SetActiveRenderer(Renderer);
    */

    return true;
}



void ClearAll()
{
    //BWrapper::DestroyRenderer();
    //BWrapper::DestroyWindow();
}


int main(int, char**){

    BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);

    //SDL_Event event;
    if (!Init())
    {
        logError("Init Errors");
        ClearAll();
        return 1;
    }

    /*
    BWrapper::Log(BWrapper::LogPriority::Debug, "Started");
    char buffer[100] = {0};
    FileReader fr;
    unsigned Readed;
    if (fr.Open("data/2.txt", BWrapper::FileSearchPriority::Assets))
    {
        BWrapper::Log(BWrapper::LogPriority::Debug, "File opened");
        std::string line;
        //while (fr.ReadLine(line))
        while (fr.Read(buffer, 10, Readed))
        {
            BWrapper::Log(BWrapper::LogPriority::Debug, "Inside the cycle");
            //BWrapper::Log(BWrapper::LogPriority::Debug, line.c_str());
            BWrapper::Log(BWrapper::LogPriority::Debug, std::string(buffer, Readed).c_str());
        };
    }
    fr.Close();
    */

    {
        TestFS testFS;
        testFS.Run();
    }

    {
        ConfigNamagerTester cfg;
        cfg.Run();
    }
    statistic.PrintResult();

    //SDL_Delay(8000);

    ClearAll();
    BWrapper::Quit();

    return 0;
}
