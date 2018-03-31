#include "SDL.h"
#include "SDL_image.h"
#include "openglesdriver.h"
#include "basewrapper.h"
#include "sdffont.h"

int main(int argc, char *argv[])
{
	BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);
	//if (SDL_Init(SDL_INIT_EVERYTHING /*| SDL_VIDEO_OPENGL*/) < 0)
	if (!BWrapper::Init(SDL_INIT_VIDEO))
	{
		logError("SDL Init error %s", SDL_GetError());
		return 0;
	}
	
	auto window = BWrapper::CreateRenderWindow("SDL2 SDF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (BWrapper::GetDeviceOS() == BWrapper::OS::Windows ? 0 : SDL_WINDOW_BORDERLESS));
	if (!window)
	{
		logError("Window create error %s", SDL_GetError());
		return 0;
	}

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

	auto Renderer = BWrapper::CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!Renderer)
	{
		logError("Renderer create error %s", SDL_GetError());
		return 0;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	auto Driver = GLESDriver::GetInstance();
	Driver->Init();

	std::string assetsDir = "";
	if (BWrapper::GetDeviceOS() != BWrapper::OS::AndroidOS)
		assetsDir = "assets/";

	auto Img1 = IMG_Load((assetsDir + "img/1.png").c_str());
	if (!Img1)
	{
		logError("Image 1.png load error %s", SDL_GetError());
		return 0;
	}
	auto tex1 = SDL_CreateTextureFromSurface(Renderer, Img1);
	SDL_FreeSurface(Img1);

	auto Img2 = IMG_Load((assetsDir + "img/2.png").c_str());
	if (!Img2)
	{
		logError("Image 2.png load error %s", SDL_GetError());
		return 0;
	}
	auto tex2 = SDL_CreateTextureFromSurface(Renderer, Img2);
	SDL_FreeSurface(Img2);

	SDFFont fnt;
	fnt.Load("sdf/font_0.png", BWrapper::FileSearchPriority::Assets);

	bool DrawWithRender = true;
	bool DrawSDF = true;

	SDL_SetRenderDrawColor(Renderer, 100, 100, 100, 100);
	while (1)
	{
		SDL_Event e;
		while (SDL_WaitEvent(&e))
		{
			if (e.type == SDL_QUIT) goto end;
			SDL_RenderClear(Renderer);

			if (DrawWithRender)
			{
				SDL_Rect r;
				r.x = r.y = 10; r.w = r.h = 100;
				SDL_RenderCopy(Renderer, tex1, nullptr, &r);

				SDL_GetWindowSize(window, &r.x, nullptr);
				r.y = 10; r.w = r.h = 100;
				r.x = r.x - r.w - 10;
				SDL_RenderCopy(Renderer, tex2, nullptr, &r);
				//SDL_RenderPresent(Renderer);
				//SDL_GL_SwapWindow(window);
			}

			if (DrawSDF)
			{
				SDFFontBuffer FontBuffer(&fnt, 2, AkkordColor(0, 0, 255));
				FontBuffer.Flush();
			}

			SDL_RenderPresent(Renderer);
		}
	}

end:
	SDL_DestroyTexture(tex1);
	SDL_DestroyTexture(tex2);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(Renderer);
	SDL_Quit();

	return 0;
}