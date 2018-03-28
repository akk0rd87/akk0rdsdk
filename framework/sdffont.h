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
"#define SDF_OUTLINE \n\
varying lowp vec4 result_color; \n\
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

class SDFProgram
{
	GLuint shaderProgram;
	GLint sdf_outline_color, sdf_params, font_color;
	
public :
	bool Init()
	{
		GLint oldProgramId;

		auto Driver = GLESDriver::GetInstance();		

		// Create and compile the vertex shader
		GLuint vertexShader = Driver->glCreateShader(GL_VERTEX_SHADER); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);
		Driver->glShaderSource(vertexShader, 1, &SDF_vertexSource, NULL); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);
		Driver->glCompileShader(vertexShader); Driver->CheckError(__LINE__); Driver->PrintShaderLog(vertexShader, __LINE__);

		// Create and compile the fragment shader
		GLuint fragmentShader = Driver->glCreateShader(GL_FRAGMENT_SHADER); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);
		Driver->glShaderSource(fragmentShader, 1, &SDF_fragmentSource, NULL); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);
		Driver->glCompileShader(fragmentShader); Driver->CheckError(__LINE__); Driver->PrintShaderLog(fragmentShader, __LINE__);

		// Link the vertex and fragment shader into a shader program
		GLuint shaderProgram = Driver->glCreateProgram(); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		Driver->glAttachShader(shaderProgram, vertexShader); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		Driver->glAttachShader(shaderProgram, fragmentShader); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

		Driver->glBindAttribLocation(shaderProgram, SDF_ATTRIB_POSITION, "position"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		//Driver->glBindAttribLocation(shaderProgram, ATTRIB_COLOR, "color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		Driver->glBindAttribLocation(shaderProgram, SDF_ATTRIB_UV, "uv"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

		Driver->glLinkProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

		Driver->glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
		Driver->glUseProgram(shaderProgram); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		
		auto mat          = Driver->glGetUniformLocation(shaderProgram, "mat"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		auto base_texture = Driver->glGetUniformLocation(shaderProgram, "base_texture"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		sdf_outline_color = Driver->glGetUniformLocation(shaderProgram, "sdf_outline_color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);
		sdf_params = Driver->glGetUniformLocation(shaderProgram, "sdf_params"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);				
		font_color = Driver->glGetUniformLocation(shaderProgram, "font_color"); Driver->CheckError(__LINE__); Driver->PrintProgamLog(shaderProgram, __LINE__);

		Driver->glUniformMatrix4fv(mat, 1, GL_FALSE, SDF_Mat);    Driver->CheckError(__LINE__);
		Driver->glUniform1i(base_texture, 0);

		Driver->glUseProgram(oldProgramId);

		logDebug("sdf_outline_color = %d; sdf_params = %d; mat = %d, base_texture = %d, font_color = %d", sdf_outline_color, sdf_params, mat, base_texture, font_color);
	}
};

class SDFFont
{
    
public:
	enum struct AlignV : unsigned char { Top, Center, Bottom };
	enum struct AlignH : unsigned char { Left, Center, Right };

	bool Load(const char* FileName, BWrapper::FileSearchPriority SearchPriority)
	{

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

	unsigned int digitsCount = 0;

	AkkordColor color;

	std::vector<float>UV;
	std::vector<float>squareVertices;
	std::vector<unsigned short>Indices;	
public:
	SDFFontBuffer(SDFFont* Font, unsigned int DigitsCount, AkkordColor Color)
	{
		this->Clear();
		sdfFont = Font;
		digitsCount = DigitsCount;		
		color = Color;

		UV.reserve(digitsCount * 4);
		squareVertices.reserve(digitsCount * 4);
		Indices.reserve(digitsCount * 6);
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
		UV.clear();
		squareVertices.clear();
		Indices.clear();
		digitsCount = 0;
		SDFFont* sdfFont = nullptr;
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