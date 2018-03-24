#include "openglesdriver.h"
#include "basewrapper.h"

class SDFFont
{
public:
	enum struct AlignV : unsigned char { Top, Center, Bottom };
	enum struct AlignH : unsigned char { Left, Center, Right };
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
public:
	SDFFontBuffer(SDFFont* Font)
	{
		this->Clear();
		sdfFont = Font;
	}

	void SetScale(float Scale){ scaleX = scaleY = Scale; }
	void SetScale(float ScaleX, float ScaleY){ scaleX = ScaleX; scaleY = ScaleY; }

	float GetScaleX(){ return scaleX; }
	float GetScaleY(){ return scaleY; }

	void SetRect(int W, int H){ rectW = W; rectH = H; }

	void SetAlignment(SDFFont::AlignH AlignH, SDFFont::AlignV AlignV){ alignH = AlignH; alignV = AlignV; }	
	void SetAlignmentH(SDFFont::AlignH AlignH){ alignH = AlignH; }
	void SetAlignmentV(SDFFont::AlignV AlignV){ alignV = AlignV; }

	SDFFont::AlignH GetAlignH() { return alignH; }
	SDFFont::AlignV GetAlignV() { return alignV; }

	void Clear()
	{
		//logDebug("Clear");
	};

	void Flush()
	{
		Clear();
	};	
	
	~SDFFontBuffer()
	{
		Clear();
	};

	

	// сейчас это int, возможно для этой функции сделать отдельный тип со float
	AkkordPoint GetTextSize(const char* Text)
	{
		AkkordPoint pt;	
		return pt;
	}

	
};