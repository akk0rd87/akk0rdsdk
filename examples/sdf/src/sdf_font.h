#include "SDL.h"
#include "SDL_image.h"
#include "openglesdriver.h"
#include "basewrapper.h"
#include "sdffont.h"

//extern int GLES2_ActivateRenderer(SDL_Renderer * renderer);

int main(int argc, char *argv[])
{
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");

	BWrapper::SetLogPriority(BWrapper::LogPriority::Debug);	
	if (!BWrapper::Init(SDL_INIT_VIDEO))
	{
		logError("SDL Init error %s", SDL_GetError());
		return 0;
	}
	
	auto window = BWrapper::CreateRenderWindow("SDL2 SDF", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 900, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (BWrapper::GetDeviceOS() == BWrapper::OS::Windows ? 0 : SDL_WINDOW_BORDERLESS));
	if (!window)
	{
		logError("Window create error %s", SDL_GetError());		
		return 0;
	}
	BWrapper::SetActiveWindow(window);
	
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
	
	SDL_SetRenderDrawBlendMode(BWrapper::GetActiveRenderer(), SDL_BLENDMODE_BLEND);		

	while (1)
	{
		SDL_Event e;
		while (SDL_WaitEvent(&e))
		{
			if (e.type == SDL_QUIT) goto end;			
			BWrapper::SetCurrentColor(AkkordColor(100, 100, 100, 100));
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

				BWrapper::SetCurrentColor(AkkordColor(255, 255, 0));
				//BWrapper::DrawRect(AkkordRect(10 - 1, 10 - 1, 150, 150));
				BWrapper::DrawRect(AkkordRect(50, 50, 180, 180));

				BWrapper::SetCurrentColor(AkkordColor(255, 0, 255));

				BWrapper::FillRect(AkkordRect(150, 150, 150, 150));
			}

			if (DrawSDF)
			{
				SDFFontBuffer FontBuffer(&fnt, 2, AkkordColor(255, 255, 255));
				FontBuffer.SetOutlineColor(AkkordColor(0, 0, 0));
				FontBuffer.SetOutline(true);
				FontBuffer.SetSDFParams(0.5f, 40.0f, 0.4f, 40.0f);
				auto size = BWrapper::GetScreenSize();
				//FontBuffer.SetRect(640, 480);
				FontBuffer.SetRect(size.x, size.y);
				FontBuffer.SetScale(0.15f);
				//FontBuffer.SetScale(3.0f);
				FontBuffer.SetAlignment(SDFFont::AlignH::Left, SDFFont::AlignV::Top);
				auto FontSize = FontBuffer.DrawText(10, 10, "Who are you?");
				
				//BWrapper::SetCurrentColor(AkkordColor(255, 0, 0));
				//BWrapper::DrawRect(AkkordRect(10, 10, FontSize.x, FontSize.y));

				auto TextSize = FontBuffer.GetTextSize("PpFfWw true");
				FontSize = FontBuffer.DrawText(10, 200, "PpFfWw true");		


				//SDL_BlendMode bmd;
				//SDL_GetRenderDrawBlendMode(BWrapper::GetActiveRenderer(), &bmd);				
				//SDL_SetRenderDrawBlendMode(BWrapper::GetActiveRenderer(), SDL_BLENDMODE_BLEND);
				FontBuffer.Flush();				

				//SDL_SetRenderDrawBlendMode(BWrapper::GetActiveRenderer(), bmd);

				BWrapper::SetCurrentColor(AkkordColor(255, 0, 0));
				BWrapper::DrawRect(AkkordRect(10, 200, FontSize.x, FontSize.y));
				BWrapper::DrawRect(AkkordRect(10 - 1, 200 - 1, FontSize.x + 2, FontSize.y + 2));
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