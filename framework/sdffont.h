#pragma once
#ifndef __AKK0RD_SDFFONT_H__
#define __AKK0RD_SDFFONT_H__

#include "openglesdriver.h"
#include "basewrapper.h"

static const GLchar* SDF_vertexSource =
"varying lowp vec4 result_color; \n\
varying mediump vec2 result_uv; \n\
uniform lowp vec4 sdf_outline_color; \n\
uniform mediump vec4 sdf_params; \n\
uniform mediump mat4 mat; \n\
uniform vec4 font_color; \n\
attribute vec2 position; \n\
attribute vec2 uv; \n\
void main()  \n\
{\n\
    gl_Position = mat * vec4(position, 0.0, 1.0);  \n\
    result_color = font_color; \n\
    result_uv = uv; \n\
}\n";

static const GLchar* SDF_fragmentSource =
"varying lowp vec4 result_color; \n\
varying mediump vec2 result_uv; \n\
uniform lowp vec4 sdf_outline_color; \n\
uniform mediump vec4 sdf_params; \n\
uniform lowp sampler2D base_texture; \n\
lowp vec4 get_base_sdf() \n\
{\n\
    lowp float tx = texture2D(base_texture, result_uv).r; \n\
#ifdef SDF_OUTLINE \n\
    lowp float b =   min((tx - sdf_params.z) * sdf_params.w, 1.0); \n\
    lowp float a = clamp((tx - sdf_params.x) * sdf_params.y, 0.0, 1.0); \n\
    lowp vec4 res = (sdf_outline_color + (result_color - sdf_outline_color)*a) * b; \n\
#else \n\
    lowp float a = min((tx - sdf_params.x) * sdf_params.y, 1.0); \n\
    lowp vec4 res = result_color * a; \n\
#endif \n\
    return res;\n\
} \n\
void main() \n\
{	\n\
	gl_FragColor = get_base_sdf();\n\
}\n";

static const GLfloat SDF_Mat[] =
{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
};

enum {
	SDF_ATTRIB_POSITION = 10, // Начинаем не с нуля, чтобы индексы не пересеклись с другими программами
	//ATTRIB_COLOR,
	SDF_ATTRIB_UV,
	SDF_NUM_ATTRIBUTES
};

struct ShaderProgramStruct
{
	GLuint shaderProgram;
	GLint sdf_outline_color, sdf_params, font_color;
};

// https://habrahabr.ru/post/282191/
static inline unsigned int UTF2Unicode(const /*unsigned*/ char *txt, unsigned int &i){
	unsigned int a = txt[i++];
	if ((a & 0x80) == 0)return a;
	if ((a & 0xE0) == 0xC0){
		a = (a & 0x1F) << 6;
		a |= txt[i++] & 0x3F;
	}
	else if ((a & 0xF0) == 0xE0){
		a = (a & 0xF) << 12;
		a |= (txt[i++] & 0x3F) << 6;
		a |= txt[i++] & 0x3F;
	}
	else if ((a & 0xF8) == 0xF0){
		a = (a & 0x7) << 18;
		a |= (a & 0x3F) << 12;
		a |= (txt[i++] & 0x3F) << 6;
		a |= txt[i++] & 0x3F;
	}
	return a;
};

class SDFProgram
{
	bool Inited = false;
	ShaderProgramStruct ShaderProgram, ShaderProgramOutline;

	bool CompileProgram(ShaderProgramStruct* Program, const char* FragmentShader)
	{
		GLint oldProgramId;

		auto Driver = GLESDriver::GetInstance();

		// Create and compile the vertex shader
		GLuint vertexShader = Driver->glCreateShader(GL_VERTEX_SHADER); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);
		Driver->glShaderSource(vertexShader, 1, &SDF_vertexSource, NULL); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);
		Driver->glCompileShader(vertexShader); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);

		// Create and compile the fragment shader
		GLuint fragmentShader = Driver->glCreateShader(GL_FRAGMENT_SHADER); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);
		Driver->glShaderSource(fragmentShader, 1, &FragmentShader, NULL); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);
		Driver->glCompileShader(fragmentShader); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);

		// Link the vertex and fragment shader into a shader program
		Program->shaderProgram = Driver->glCreateProgram(); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);
		Driver->glAttachShader(Program->shaderProgram, vertexShader); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);
		Driver->glAttachShader(Program->shaderProgram, fragmentShader); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);

		Driver->glBindAttribLocation(Program->shaderProgram, SDF_ATTRIB_POSITION, "position"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);		
		Driver->glBindAttribLocation(Program->shaderProgram, SDF_ATTRIB_UV, "uv"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);

		Driver->glLinkProgram(Program->shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);

		Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
		Driver->glUseProgram(Program->shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);

		auto mat = Driver->glGetUniformLocation(Program->shaderProgram, "mat"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);
		auto base_texture = Driver->glGetUniformLocation(Program->shaderProgram, "base_texture"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);
		Program->sdf_outline_color = Driver->glGetUniformLocation(Program->shaderProgram, "sdf_outline_color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);
		Program->sdf_params = Driver->glGetUniformLocation(Program->shaderProgram, "sdf_params"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);
		Program->font_color = Driver->glGetUniformLocation(Program->shaderProgram, "font_color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(Program->shaderProgram, __LINE__);

		Driver->glUniformMatrix4fv(mat, 1, GL_FALSE, SDF_Mat);    Driver->CheckError(__LINE__);
		Driver->glUniform1i(base_texture, 0);

		Driver->glUseProgram(oldProgramId);

		logDebug("sdf_outline_color = %d; sdf_params = %d; mat = %d, base_texture = %d, font_color = %d", Program->sdf_outline_color, Program->sdf_params, mat, base_texture, Program->font_color);
		return true;
	}
	
public :
	bool Init()
	{		
		if (!Inited)
		{
			if (this->CompileProgram(&ShaderProgram, SDF_fragmentSource) && this->CompileProgram(&ShaderProgram, (std::string("#define SDF_OUTLINE \n") + SDF_fragmentSource).c_str()))
			{
				Inited = true;
				return true;
			}
			return false;
		};
		return true;
	};

	ShaderProgramStruct* GetShaderProgram(bool Outline)
	{
		if (Outline)
			return &ShaderProgramOutline;

		return &ShaderProgram;
	};
};

SDFProgram sdfProgram;

struct SDFCharInfo
{
	unsigned int id, x, y, w, h;
};

class SDFFont
{
	GLuint texture;
	SDL_Surface* fontAtlas;
	unsigned int ScaleW, ScaleH;

	std::vector<SDFCharInfo> CharsVector;

	bool ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority)
	{
		FileReader fr;
		std::string line;

		decltype(line.find(',')) lpos;
		decltype(lpos)           rpos;

		if (fr.Open(FNTFile, SearchPriority))
		{
			while (fr.ReadLine(line))
				if (line.size() > 0)
				{
					if (line.find("<char id", 0) != std::string::npos)
					{
						lpos = 0;
						rpos = 0;

						rpos = line.find("id=", lpos) + 4;
						auto id = BWrapper::Str2Num(std::string(line, rpos).c_str());

						lpos = rpos;
						rpos = line.find("x=", lpos) + 3;
						auto x = BWrapper::Str2Num(std::string(line, rpos).c_str());

						lpos = rpos;
						rpos = line.find("y=", lpos) + 3;
						auto y = BWrapper::Str2Num(std::string(line, rpos).c_str());

						lpos = rpos;
						rpos = line.find("width=", lpos) + 7;
						auto w = BWrapper::Str2Num(std::string(line, rpos).c_str());

						lpos = rpos;
						rpos = line.find("height=", lpos) + 8;
						auto h = BWrapper::Str2Num(std::string(line, rpos).c_str());
						CharsVector.push_back({ id, x, y, w, h });

						goto next_iteration;
					};

					if (line.find("<chars", 0) != std::string::npos)
					{
						auto cnt = BWrapper::Str2Num(std::string(line, line.find("\"", 0) + 1).c_str());
						CharsVector.reserve(cnt);
						goto next_iteration;
					};

					if (line.find("<common", 0) != std::string::npos)
					{
						rpos = line.find("scaleW=", 0) + 8;
						ScaleW = BWrapper::Str2Num(std::string(line, rpos).c_str());

						rpos = line.find("scaleH=", 0) + 8;
						ScaleH = BWrapper::Str2Num(std::string(line, rpos).c_str());

						logDebug("ScaleW = %d, ScaleH = %d", ScaleW, ScaleH);

						goto next_iteration;
					};

				next_iteration:;
				};
		};

		//for (auto v : CharsVector) logDebug("id=%d, x=%d, y=%d, w=%d, h=%d", v.id, v.x, v.y, v.w, v.h);

		fr.Close();

		return true;
	}

	void Clear()
	{
		CharsVector.clear();
		if (fontAtlas)
		{
			SDL_FreeSurface(fontAtlas);
			fontAtlas = nullptr;
		};
	};


public:
	enum struct AlignV : unsigned char { Top, Center, Bottom };
	enum struct AlignH : unsigned char { Left, Center, Right };

	~SDFFont()
	{
		Clear();
	}

	unsigned int GetAtlasW(){ return ScaleW; }
	unsigned int GetAtlasH(){ return ScaleH; }

	bool Load(const char* FileName, BWrapper::FileSearchPriority SearchPriority)
	{
		sdfProgram.Init();
		auto Driver = GLESDriver::GetInstance();
		
		unsigned Size;
		auto buffer = BWrapper::File2Buffer(FileName, SearchPriority, Size);
		auto io = SDL_RWFromMem(buffer, Size);		
		fontAtlas = IMG_LoadPNG_RW(io);
		BWrapper::CloseBuffer(buffer);
		SDL_RWclose(io);		

		if (!fontAtlas)
		{
			logError("fontAtlas create error %s", SDL_GetError());
			return false;
		}

		Driver->glGenTextures(1, &texture); Driver->CheckError(__LINE__);
		Driver->glBindTexture(GL_TEXTURE_2D, texture); Driver->CheckError(__LINE__);
		Driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); Driver->CheckError(__LINE__);
		Driver->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); Driver->CheckError(__LINE__);

		Driver->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fontAtlas->w, fontAtlas->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontAtlas->pixels); Driver->CheckError(__LINE__);

		ParseFNTFile("sdf/font.fnt", BWrapper::FileSearchPriority::Assets);

		return true;
	};

	bool Draw(bool Outline, unsigned Count, AkkordColor& FontColor, AkkordColor& OutlineColor, const float* UV, const float* squareVertices, unsigned short* Indices)
	{
		GLint oldProgramId;
		auto shaderProgram = sdfProgram.GetShaderProgram(Outline);

		auto Driver = GLESDriver::GetInstance();

		Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
		Driver->glUseProgram(shaderProgram->shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram->shaderProgram, __LINE__);

		Driver->glBindTexture(GL_TEXTURE_2D, texture); Driver->CheckError(__LINE__);

		Driver->glEnableVertexAttribArray(SDF_ATTRIB_POSITION); Driver->CheckError(__LINE__);
		Driver->glVertexAttribPointer(SDF_ATTRIB_POSITION, 2, GL_FLOAT, 0, 0, squareVertices); Driver->CheckError(__LINE__);

		Driver->glEnableVertexAttribArray(SDF_ATTRIB_UV); Driver->CheckError(__LINE__);
		Driver->glVertexAttribPointer(SDF_ATTRIB_UV, 2, GL_FLOAT, 0, 0, UV); Driver->CheckError(__LINE__);

		Driver->glUniform4f(shaderProgram->font_color, float(FontColor.GetR()) / 255, float(FontColor.GetG()) / 255, float(FontColor.GetB()) / 255, 1.0f); Driver->CheckError(__LINE__);
		Driver->glUniform4f(shaderProgram->sdf_params, 0.5f, 11.0f, 0.4f, 11.0f); Driver->CheckError(__LINE__);

		if (Outline) 
			if (shaderProgram->sdf_outline_color >= 0)
			{
				Driver->glUniform4f(shaderProgram->sdf_outline_color, float(OutlineColor.GetR()) / 255, float(OutlineColor.GetG()) / 255, float(OutlineColor.GetB()) / 255, 1.0f); Driver->CheckError(__LINE__);
			}
			else
			{
				logError("shaderProgram->sdf_outline_color error %d", shaderProgram->sdf_outline_color);
			}

		Driver->glDrawElements(GL_TRIANGLES, Count, GL_UNSIGNED_SHORT, Indices); Driver->CheckError(__LINE__);

		Driver->glUseProgram(oldProgramId); Driver->CheckError(__LINE__); Driver->CheckError(__LINE__);

		return true;
	};

	bool GetCharInfo(unsigned Code, SDFCharInfo& ci)
	{
		for (auto v : CharsVector)
			if (Code == v.id)
			{
				ci = v;
				return true;
			}
				

		logError("Char with id=%u not found", Code);
		return false;
	};
};

// Для рисования всегда указывать левую верхнюю точку (удобно для разгаданных слов в "составь слова")

class SDFFontBuffer
{
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	SDFFont* sdfFont = nullptr;
	int rectW = -1, rectH = -1;

	SDFFont::AlignH alignH = SDFFont::AlignH::Center;
	SDFFont::AlignV alignV = SDFFont::AlignV::Center;	

	AkkordColor color;

	std::vector<float>UV;
	std::vector<float>squareVertices;
	std::vector<unsigned short>Indices;	
public:
	SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, AkkordColor Color)
	{
		this->Clear();
		sdfFont = Font;
		DigitsCount;		
		color = Color;

		UV.reserve(DigitsCount * 4);
		squareVertices.reserve(DigitsCount * 4);
		Indices.reserve(DigitsCount * 6);

		/*
		unsigned int AtlasW = 8192;
		unsigned int AtlasH = 4096;

		unsigned LetterX1 = 3003;
		unsigned LetterY1 = 1256;
		unsigned LetterW1 = 398;
		unsigned LetterH1 = 387;

		unsigned LetterX2 = 4566;
		unsigned LetterY2 = 1250;
		unsigned LetterW2 = 369;
		unsigned LetterH2 = 387;

		UV = { float(LetterX1) / AtlasW, float(LetterY1 + LetterH1) / AtlasH,
			float(LetterX1 + LetterW1) / AtlasW, float(LetterY1 + LetterH1) / AtlasH,
			float(LetterX1) / AtlasW, float(LetterY1) / AtlasH,
			float(LetterX1 + LetterW1) / AtlasW, float(LetterY1) / AtlasH,

			float(LetterX2) / AtlasW, float(LetterY2 + LetterH2) / AtlasH,
			float(LetterX2 + LetterW2) / AtlasW, float(LetterY2 + LetterH2) / AtlasH,
			float(LetterX2) / AtlasW, float(LetterY2) / AtlasH,
			float(LetterX2 + LetterW2) / AtlasW, float(LetterY2) / AtlasH
		};

		squareVertices = {
			-0.8f, -1.0f,
			0.0f, -1.0f,
			-0.8f, 1.0f,
			0.0f, 1.0f,

			-0.0f, -1.0f,
			0.8f, -1.0f,
			-0.0f, 1.0f,
			0.8f, 1.0f
		};

		Indices = { 0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7 };
		*/
	}

	void SetScale(float Scale){ scaleX = scaleY = Scale; }
	void SetScale(float ScaleX, float ScaleY){ scaleX = ScaleX; scaleY = ScaleY; }

	float GetScaleX(){ return scaleX; }
	float GetScaleY(){ return scaleY; }

	void SetRect(int W, int H)
	{ 
		rectW = W;
		rectH = H; 
	}

	void SetAlignment(SDFFont::AlignH AlignH, SDFFont::AlignV AlignV){ alignH = AlignH; alignV = AlignV; }	
	void SetAlignmentH(SDFFont::AlignH AlignH){ alignH = AlignH; }
	void SetAlignmentV(SDFFont::AlignV AlignV){ alignV = AlignV; }

	SDFFont::AlignH GetAlignH() { return alignH; }
	SDFFont::AlignV GetAlignV() { return alignV; }

	void Clear()
	{
		UV.clear();
		squareVertices.clear();
		Indices.clear();				
	};

	void Flush()
	{
		sdfFont->Draw(false, Indices.size(), color, color, &UV.front(), &squareVertices.front(), &Indices.front());
		Clear();
	};	
	
	~SDFFontBuffer()
	{
		Clear();
		sdfFont = nullptr;
	};	

	// сейчас это int, возможно для этой функции сделать отдельный тип со float
	AkkordPoint GetTextSize(const char* Text)
	{
		AkkordPoint pt;

		unsigned int i = 0;
		unsigned int a = 0;
		unsigned len = strlen(Text);

		pt.x = 0;
		pt.y = 0;
		
		SDFCharInfo charParams;

		while (i < len)
		{
			a = UTF2Unicode(Text, i);
			
			sdfFont->GetCharInfo(a, charParams);
			pt.x += charParams.w;

			if (pt.y < charParams.h)
				pt.y = charParams.h;
		};

		pt.x *= scaleX;
		pt.y *= scaleY;
		//logDebug("text=%s, width=%d, height=%d", Text, pt.x, pt.y);

		return pt;
	};

	void DrawText(int X, int Y, const char* Text)
	{
		auto size = GetTextSize(Text);

		// Выбираем начальную точку в зависимости от выравнивания
		switch (alignH)
		{		
			case SDFFont::AlignH::Center:				
				X = X + (rectW - size.x) / 2;
				break;
			case SDFFont::AlignH::Right:
				X = X + (rectW - size.x);
				break;				
			default: // в остальных случаях ничего не делаем, координату X не меняем
				break;
		};
		
		switch (alignV)
		{
			case SDFFont::AlignV::Center:
				Y = Y + (rectH - size.y) / 2;
				break;
			case SDFFont::AlignV::Bottom:
				Y = Y + (rectH - size.y);
				break;
			default: // в остальных случаях ничего не делаем, координату Y не меняем
				break;
		};

		unsigned int i = 0;
		unsigned int a = 0;
		unsigned len = strlen(Text);

		auto atlasW = sdfFont->GetAtlasW();
		auto atlasH = sdfFont->GetAtlasH();

		auto ScreenSize = BWrapper::GetScreenSize();	
		
		float ScrenW = ScreenSize.x;
		float ScrenH = ScreenSize.y;

		float f;

		SDFCharInfo charParams;
		
		auto UVsize = UV.size();
		while (i < len)
		{			
			a = UTF2Unicode(Text, i);
			sdfFont->GetCharInfo(a, charParams);
			
			UV.push_back(float(charParams.x               ) / atlasW); UV.push_back(float(charParams.y + charParams.h) / atlasH);
			UV.push_back(float(charParams.x + charParams.w) / atlasW); UV.push_back(float(charParams.y + charParams.h) / atlasH);
			UV.push_back(float(charParams.x               ) / atlasW); UV.push_back(float(charParams.y               ) / atlasH);
			UV.push_back(float(charParams.x + charParams.w) / atlasW); UV.push_back(float(charParams.y               ) / atlasH);

			Indices.push_back(UVsize + 0); Indices.push_back(UVsize + 1); Indices.push_back(UVsize + 2);
			Indices.push_back(UVsize + 1); Indices.push_back(UVsize + 2); Indices.push_back(UVsize + 3);

			squareVertices.push_back(2 * (float)(X / ScrenW) - 1.0f); squareVertices.push_back(-1.0f);
			squareVertices.push_back(2 * (float)(X + (float)scaleX * charParams.w) / ScrenW - 1.0f); squareVertices.push_back(-1.0f);
			squareVertices.push_back(2 * (float)(X / ScrenW) - 1.0f); squareVertices.push_back(1.0f);
			squareVertices.push_back(2 * (float)(X + (float)scaleX * charParams.w) / ScrenW - 1.0f); squareVertices.push_back(1.0f);
			
			X = X + (float)scaleX * charParams.w;
			UVsize += 4;
		};		
	};
};

#endif // __AKK0RD_SDFFONT_H__