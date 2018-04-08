#include "SDL.h"
#include "SDL_image.h"
#include "openglesdriver.h"
#include "basewrapper.h"
#include "sdffont.h"

int main(int argc, char *argv[])
{
	BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);	
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
	BWrapper::SetActiveWindow(window);

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

	auto Renderer = BWrapper::CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!Renderer)
	{
		logError("Renderer create error %s", SDL_GetError());
		return 0;
	}
	BWrapper::SetActiveRenderer(Renderer);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	auto Driver = GLESDriver::GetInstance();
	Driver->Init();

	AkkordTexture Img1, Img2;	
	Img1.LoadFromFile("img/1.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);
	Img2.LoadFromFile("img/2.png", AkkordTexture::TextureType::PNG, BWrapper::FileSearchPriority::Assets);	

	SDFFont fnt;
	fnt.Load("sdf/font_0.png", BWrapper::FileSearchPriority::Assets);
	//fnt.Load("sdf/HieroCalibri.png", BWrapper::FileSearchPriority::Assets);

	bool DrawWithRender = true;
	bool DrawSDF = true;
	
	SDL_SetRenderDrawColor(Renderer, 100, 100, 100, 100);
	while (1)
	{
		SDL_Event e;
		while (SDL_WaitEvent(&e))
		{
			if (e.type == SDL_QUIT) goto end;			
			BWrapper::ClearRenderer();

			if (DrawWithRender)
			{
				SDL_Rect r;
				r.x = r.y = 10; r.w = r.h = 100;				
				Img1.Draw(AkkordRect(r.x, r.y, r.w, r.h), nullptr);

				SDL_GetWindowSize(window, &r.x, nullptr);
				r.y = 10; r.w = r.h = 100;
				r.x = r.x - r.w - 10;
				Img2.Draw(AkkordRect(r.x, r.y, r.w, r.h), nullptr);
			}

			if (DrawSDF)
			{
				SDFFontBuffer FontBuffer(&fnt, 2, AkkordColor(255, 255, 255));
				FontBuffer.SetOutline(true);
				auto size = BWrapper::GetScreenSize();
				//FontBuffer.SetRect(640, 480);
				FontBuffer.SetRect(size.x, size.y);
				FontBuffer.SetScale(0.2f);
				//FontBuffer.SetScale(3.0f);
				FontBuffer.SetAlignment(SDFFont::AlignH::Left, SDFFont::AlignV::Top);
				FontBuffer.DrawText(10, 10, "Who are you?");
				FontBuffer.DrawText(10, 200, "PpFfWw true");
				FontBuffer.Flush();
				//auto size = FontBuffer.GetTextSize("Hello");
			}
			
			BWrapper::RefreshRenderer();
		}
	}

end:	
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(Renderer);
	SDL_Quit();

	return 0;
}