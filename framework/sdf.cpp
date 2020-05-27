#include "sdf.h"
#include "SDL_image.h"
#include"openglesdriver.h"
#include <array>

static SDFProgram sdfProgram;
GLuint ArrayBuffer, ElementBuffer;

constexpr GLsizei constBufferSize = 8;

static struct {
    std::array<GLuint, constBufferSize> ArrayBufferID;
    std::array<GLuint, constBufferSize> ElementBufferID;

    std::array<GLsizeiptr, constBufferSize> ArrayBufferSize;
    std::array<GLsizeiptr, constBufferSize> ElementBufferSize;

    GLsizei CurentBuffer;
} VBO;

/*
https://github.com/libgdx/libgdx/wiki/Hiero
"java -cp gdx.jar;gdx-natives.jar;gdx-backend-lwjgl.jar;gdx-backend-lwjgl-natives.jar;extensions\gdx-freetype\gdx-freetype.jar;extensions\gdx-freetype\gdx-freetype-natives.jar;extensions\gdx-tools\gdx-tools.jar com.badlogic.gdx.tools.hiero.Hiero"
*/

static const GLchar* SDF_vertexSource =
"varying highp vec4 result_color; \n\
varying highp vec2 result_uv; \n\
uniform highp mat4 u_projection; \n\
uniform highp vec4 font_color; \n\
uniform highp float smooth_param; \n\
attribute highp vec2 a_position; \n\
attribute highp vec2 a_texCoord; \n\
varying highp float SmoothDistance; \n\
varying highp float center; \n\
#ifdef SDF_OUTLINE \n\
    uniform highp vec4  sdf_outline_color; \n\
    uniform highp float border; \n\
    varying highp vec4  outBorderCol; \n\
    varying highp float    outlineMaxValue0; \n\
    varying highp float    outlineMaxValue1; \n\
#endif \n\
void main()  \n\
{\n\
    gl_Position = u_projection * vec4(a_position, 0.0, 1.0);  \n\
    result_color = font_color; \n\
    result_uv = a_texCoord; \n\
    SmoothDistance = smooth_param; \n\
#ifdef SDF_OUTLINE \n\
    outBorderCol = sdf_outline_color; \n\
    outlineMaxValue0 = 0.5 - border; \n\
    outlineMaxValue1 = 0.5 + border; \n\
    center = outlineMaxValue0 - border; \n\
#else \n\
    center = 0.5; \n\
#endif \n\
}\n";

static const GLchar* SDF_fragmentSource =
"varying highp vec4 result_color; \n\
varying highp vec2 result_uv; \n\
uniform highp vec4 sdf_outline_color; \n\
uniform highp sampler2D base_texture; \n\
varying highp float SmoothDistance; \n\
varying highp float   outlineMaxValue0; \n\
varying highp float   outlineMaxValue1; \n\
varying highp float   center; \n\
\n\
varying highp vec4       outBorderCol; \n\
\n\
highp float my_smoothstep(highp float edge0, highp float edge1, highp float x) { \n\
x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); \n\
return x * x * (3.0 - 2.0 * x); \n\
}\n\
void main() \n\
{  \n\
    highp float distAlpha = texture2D(base_texture, result_uv).a; \n\
    highp vec4 rgba = result_color; \n\
#ifdef SDF_OUTLINE \n\
    rgba.xyzw = mix(rgba.xyzw, outBorderCol.xyzw, my_smoothstep(outlineMaxValue1, outlineMaxValue0, distAlpha)); \n\
#endif \n\
    rgba.a *= my_smoothstep(center - SmoothDistance, center + SmoothDistance, distAlpha); \n\
    gl_FragColor = rgba; \n\
}\n";

// https://habrahabr.ru/post/282191/
static inline unsigned int UTF2Unicode(const /*unsigned*/ char* txt, unsigned& i) {
    unsigned int a = txt[i++];
    if ((a & 0x80) == 0)return a;
    if ((a & 0xE0) == 0xC0) {
        a = (a & 0x1F) << 6;
        a |= txt[i++] & 0x3F;
    }
    else if ((a & 0xF0) == 0xE0) {
        a = (a & 0xF) << 12;
        a |= (txt[i++] & 0x3F) << 6;
        a |= txt[i++] & 0x3F;
    }
    else if ((a & 0xF8) == 0xF0) {
        a = (a & 0x7) << 18;
        a |= (a & 0x3F) << 12;
        a |= (txt[i++] & 0x3F) << 6;
        a |= txt[i++] & 0x3F;
    }
    return a;
};

static const GLfloat SDF_Mat[] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

SDFProgram& SDFProgram::GetInstance()
{
    return sdfProgram;
};

bool SDFProgram::CompileProgram(ShaderProgramStruct* Program, const char* VertextShader, const char* FragmentShader)
{
    GLint oldProgramId;

    auto& Driver = GLESDriver::GetInstance();

    // Create and compile the fragment shader
    GLuint vertexShader = Driver.glCreateShader(GL_VERTEX_SHADER); CheckGLESError(); PrintGLESShaderLog(vertexShader);
    Driver.glShaderSource(vertexShader, 1, &VertextShader, NULL); CheckGLESError(); PrintGLESShaderLog(vertexShader);
    Driver.glCompileShader(vertexShader); CheckGLESError(); PrintGLESShaderLog(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = Driver.glCreateShader(GL_FRAGMENT_SHADER); CheckGLESError(); PrintGLESShaderLog(fragmentShader);
    Driver.glShaderSource(fragmentShader, 1, &FragmentShader, NULL); CheckGLESError(); PrintGLESShaderLog(fragmentShader);
    Driver.glCompileShader(fragmentShader); CheckGLESError(); PrintGLESShaderLog(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    Program->shaderProgram = Driver.glCreateProgram(); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    Driver.glAttachShader(Program->shaderProgram, vertexShader); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    Driver.glAttachShader(Program->shaderProgram, fragmentShader); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

    Driver.glBindAttribLocation(Program->shaderProgram, SDFProgram::Attributes::SDF_ATTRIB_POSITION, "a_position"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    Driver.glBindAttribLocation(Program->shaderProgram, SDFProgram::Attributes::SDF_ATTRIB_UV, "a_texCoord"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

    Driver.glLinkProgram(Program->shaderProgram); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

    Driver.glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
    Driver.glUseProgram(Program->shaderProgram); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

    auto mat = Driver.glGetUniformLocation(Program->shaderProgram, "u_projection"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    auto base_texture = Driver.glGetUniformLocation(Program->shaderProgram, "base_texture"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    Program->sdf_outline_color = Driver.glGetUniformLocation(Program->shaderProgram, "sdf_outline_color"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    Program->font_color = Driver.glGetUniformLocation(Program->shaderProgram, "font_color"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    Program->smooth = Driver.glGetUniformLocation(Program->shaderProgram, "smooth_param"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);
    Program->border = Driver.glGetUniformLocation(Program->shaderProgram, "border"); CheckGLESError(); PrintGLESProgamLog(Program->shaderProgram);

    Driver.glUniformMatrix4fv(mat, 1, GL_FALSE, SDF_Mat);    CheckGLESError();
    Driver.glUniform1i(base_texture, 0);

    if (oldProgramId > 0) {
        Driver.glUseProgram(oldProgramId);
    }

    //logDebug("sdf_outline_color = %d; sdf_params = %d; mat = %d, base_texture = %d, font_color = %d", Program->sdf_outline_color, Program->sdf_params, mat, base_texture, Program->font_color);
    return true;
}

void SDFProgram::Clear()
{
}

bool SDFProgram::Init()
{
    auto& Driver = GLESDriver::GetInstance();

    {
        // обнуляем буффера
        VBO.CurentBuffer = 0;
        for (auto& v : VBO.ArrayBufferID) v = 0;
        for (auto& v : VBO.ElementBufferID) v = 0;
        for (auto& v : VBO.ArrayBufferSize) v = 0;
        for (auto& v : VBO.ElementBufferSize) v = 0;
    }

    Driver.glGenBuffers(constBufferSize, &VBO.ArrayBufferID.front()); CheckGLESError();
    Driver.glGenBuffers(constBufferSize, &VBO.ElementBufferID.front()); CheckGLESError();

    const char* Outline = "#define SDF_OUTLINE \n";
    std::string regVertex = SDF_vertexSource;
    std::string regFragment = SDF_fragmentSource;
    std::string outVertex = std::string(Outline) + SDF_vertexSource;
    std::string outFragment = std::string(Outline) + SDF_fragmentSource;

    // на винде работаем на openGL 2.1, поэтому нужно явно указать номер версии OpenGL Shading Language https://en.wikipedia.org/wiki/OpenGL_Shading_Language
    // на всем остальном работаем на openGLES 2.0
    if (BWrapper::GetDeviceOS() == BWrapper::OS::Windows) {
        const char* Version = "#version 130 \n";
        regVertex = std::string(Version) + regVertex;
        regFragment = std::string(Version) + regFragment;
        outVertex = std::string(Version) + outVertex;
        outFragment = std::string(Version) + outFragment;
    }

    if (this->CompileProgram(&ShaderProgram, regVertex.c_str(), regFragment.c_str()) && this->CompileProgram(&ShaderProgramOutline, outVertex.c_str(), outFragment.c_str())) {
        return true;
    }
    return false;
};

bool SDFGLTexture::Draw(bool Outline, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const std::vector<GLfloat>& UV, const std::vector<GLfloat>& squareVertices, const std::vector <GLushort>& Indices, GLfloat Scale, GLfloat Border, int Spread)
{
    // GLsizei Count,
    GLint oldProgramId;

    struct VertextAttrParamsStruct {
        GLint attr_0_enabled, attr_1_enabled, attr_2_enabled, attr_3_enabled;
    } VertexParams;
    VertexParams.attr_0_enabled = VertexParams.attr_1_enabled = VertexParams.attr_2_enabled = VertexParams.attr_3_enabled = GL_FALSE;

    auto shaderProgram = SDFProgram::GetInstance().GetShaderProgram(Outline);
    auto& Driver = GLESDriver::GetInstance();

    // Высчитываем размеры для буфера
    const auto uvSize = static_cast<GLsizeiptr>(UV.size() * sizeof(UV.front()));
    const auto svSize = static_cast<GLsizeiptr>(squareVertices.size() * sizeof(squareVertices.front()));
    const auto bufSize = uvSize + svSize;
    const auto indSize = static_cast<GLsizeiptr>(Indices.size() * sizeof(decltype(Indices.front())));

    // работаем c GL_ARRAY_BUFFER
    Driver.glBindBuffer(GL_ARRAY_BUFFER, VBO.ArrayBufferID[VBO.CurentBuffer]); CheckGLESError();
    if (VBO.ArrayBufferSize[VBO.CurentBuffer] < bufSize) { // размера недостаточно и нужно выделить память
        Driver.glBufferData(GL_ARRAY_BUFFER, bufSize, nullptr, GL_STREAM_DRAW); CheckGLESError();
        VBO.ArrayBufferSize[VBO.CurentBuffer] = bufSize;
    }
    Driver.glBufferSubData(GL_ARRAY_BUFFER, 0, uvSize, &UV.front()); CheckGLESError();
    Driver.glBufferSubData(GL_ARRAY_BUFFER, uvSize, svSize, &squareVertices.front()); CheckGLESError();

    // работаем c GL_ELEMENT_ARRAY_BUFFER
    Driver.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO.ElementBufferID[VBO.CurentBuffer]); CheckGLESError();
    if (VBO.ElementBufferSize[VBO.CurentBuffer] < indSize) { // размера недостаточно и нужно выделить память
        Driver.glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, &Indices.front(), GL_STREAM_DRAW); CheckGLESError();
        VBO.ElementBufferSize[VBO.CurentBuffer] = indSize;
    }
    else { // если размера хватает, заполняем текущее подмножество
        Driver.glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indSize, &Indices.front()); CheckGLESError();
    }

    // переходим к следующему буфферу, который будет использоваться при следующем обращении
    if (++VBO.CurentBuffer >= constBufferSize) {
        VBO.CurentBuffer = 0;
    }

    Driver.glGetIntegerv((GLenum)GL_CURRENT_PROGRAM, &oldProgramId); CheckGLESError();

    {
        Driver.glGetVertexAttribiv((GLuint)0, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_0_enabled); CheckGLESError();
        Driver.glGetVertexAttribiv((GLuint)1, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_1_enabled); CheckGLESError();
        Driver.glGetVertexAttribiv((GLuint)2, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_2_enabled); CheckGLESError();
        Driver.glGetVertexAttribiv((GLuint)3, (GLenum)GL_VERTEX_ATTRIB_ARRAY_ENABLED, &VertexParams.attr_3_enabled); CheckGLESError();

        // эти два атрибута нужно включить, если они выключены
        if (VertexParams.attr_0_enabled == GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)0); CheckGLESError(); }
        if (VertexParams.attr_1_enabled == GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)1); CheckGLESError(); }

        // эти атрибуты выключаем всегда, если они включены
        if (VertexParams.attr_2_enabled != GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)2); CheckGLESError(); }
        if (VertexParams.attr_3_enabled != GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)3); CheckGLESError(); }
    }

    if (oldProgramId != shaderProgram->shaderProgram) {
        Driver.glUseProgram(shaderProgram->shaderProgram); CheckGLESError(); PrintGLESProgamLog(shaderProgram->shaderProgram);
    }

    SDL_GL_BindTexture(akkordTexture.GetTexture(), nullptr, nullptr);

    Driver.glVertexAttribPointer(SDFProgram::Attributes::SDF_ATTRIB_POSITION, (GLint)2, (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, (GLsizei)0, (const GLvoid*)uvSize); CheckGLESError();
    Driver.glVertexAttribPointer(SDFProgram::Attributes::SDF_ATTRIB_UV, (GLint)2, (GLenum)GL_FLOAT, (GLboolean)GL_FALSE, (GLsizei)0, nullptr); CheckGLESError();

    Driver.glUniform4f(shaderProgram->font_color, GLfloat(FontColor.GetR()) / 255.0f, GLfloat(FontColor.GetG()) / 255.0f, GLfloat(FontColor.GetB()) / 255.0f, GLfloat(FontColor.GetA()) / 255.0f); CheckGLESError();

    const GLfloat smoothness = std::min(0.3f, 0.25f / (GLfloat)Spread / Scale * 1.5f) * 850.0f / 255.f / 3.333f;

    Driver.glUniform1f(shaderProgram->smooth, smoothness); CheckGLESError();

    if (Outline) {
        if (shaderProgram->sdf_outline_color >= 0) {
            Driver.glUniform4f(shaderProgram->sdf_outline_color, GLfloat(OutlineColor.GetR()) / 255.0f, GLfloat(OutlineColor.GetG()) / 255.0f, GLfloat(OutlineColor.GetB()) / 255.0f, GLfloat(OutlineColor.GetA()) / 255.0f); CheckGLESError();
        }
        else {
            logError("shaderProgram->sdf_outline_color error %d", shaderProgram->sdf_outline_color);
        }

        if (shaderProgram->border >= 0) {
            Driver.glUniform1f(shaderProgram->border, Border / 6.666f); CheckGLESError();
        }
        else {
            logError("shaderProgram->border error %d", shaderProgram->border);
        }
    }

    Driver.glDrawElements((GLenum)GL_TRIANGLES, static_cast<GLsizei>(Indices.size()), (GLenum)GL_UNSIGNED_SHORT, nullptr); CheckGLESError();

    // unbind texture
    SDL_GL_UnbindTexture(akkordTexture.GetTexture());

    if (shaderProgram->shaderProgram != oldProgramId && oldProgramId > 0) {
        Driver.glUseProgram(oldProgramId); CheckGLESError();
    }

    { // возвращаем все исходное состояние
        // в рамках обработки SDF мы включили эти атрибуты. Возвращаем их в исходное состояние
        if (VertexParams.attr_0_enabled == GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)0); CheckGLESError(); }
        if (VertexParams.attr_1_enabled == GL_FALSE) { Driver.glDisableVertexAttribArray((GLuint)1); CheckGLESError(); }

        if (VertexParams.attr_2_enabled != GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)2); CheckGLESError(); }
        if (VertexParams.attr_3_enabled != GL_FALSE) { Driver.glEnableVertexAttribArray((GLuint)3); CheckGLESError(); }
    }

    return true;
};

bool SDFTexture::Draw(const AkkordRect& DestRect, const AkkordRect* SourceRect)
{
    auto ScreenSize = BWrapper::GetScreenSize();

    // если целевое размещение не попадает на экран, не рисуем его
    if (DestRect.x > ScreenSize.x || DestRect.x + DestRect.w < 0 || DestRect.y > ScreenSize.y || DestRect.y + DestRect.h < 0) {
        return false;
    }

    float ScrenW = static_cast<decltype(ScrenW)>(ScreenSize.x);
    float ScrenH = static_cast<decltype(ScrenH)>(ScreenSize.y);

    struct FloatRect { float x, y, w, h; };
    FloatRect Src, Dest;

    if (SourceRect != nullptr) {
        Src.x = static_cast<float>(SourceRect->x);
        Src.y = static_cast<float>(SourceRect->y);
        Src.w = static_cast<float>(SourceRect->w);
        Src.h = static_cast<float>(SourceRect->h);
    }
    else {
        Src.x = Src.y = 0.0f;
        Src.w = atlasW;
        Src.h = atlasH;
    }

    Dest.x = Src.x / atlasW;                     //px1
    Dest.y = (Src.x + Src.w - 1.0f) / atlasW;     //px2
    Dest.w = (Src.y + Src.h - 1.0f) / atlasH;     //py1
    Dest.h = Src.y / atlasH;                   //py2

    float& px1 = Dest.x;
    float& px2 = Dest.y;
    float& py1 = Dest.w;
    float& py2 = Dest.h;

    UV.insert(UV.cend(),
        {
            px1, py1,
            px2, py1,
            px1, py2,
            px2, py2
        });

    Dest.x = static_cast<float>(DestRect.x);
    Dest.y = static_cast<float>(DestRect.y);
    Dest.w = static_cast<float>(DestRect.w);
    Dest.h = static_cast<float>(DestRect.h);

    squareVertices.insert(squareVertices.cend(),
        {
            2.0f * (Dest.x / ScrenW) - 1.0f                  , 2.0f * (ScrenH - Dest.y - (Dest.h)) / ScrenH - 1.0f,
            2.0f * (Dest.x + (Dest.w - 1.0f)) / ScrenW - 1.0f, 2.0f * (ScrenH - Dest.y - (Dest.h)) / ScrenH - 1.0f,
            2.0f * (Dest.x / ScrenW) - 1.0f                  , 2.0f * (ScrenH - Dest.y) / ScrenH - 1.0f,
            2.0f * (Dest.x + (Dest.w - 1.0f)) / ScrenW - 1.0f, 2.0f * (ScrenH - Dest.y) / ScrenH - 1.0f
        });

    decltype(Indices)::value_type PointsCnt0 = Indices.size() / 6 * 4;
    decltype(PointsCnt0) PointsCnt1 = PointsCnt0 + 1;
    decltype(PointsCnt0) PointsCnt2 = PointsCnt0 + 2;
    decltype(PointsCnt0) PointsCnt3 = PointsCnt0 + 3;

    Indices.insert(Indices.cend(),
        {
            PointsCnt0, PointsCnt1, PointsCnt2,
            PointsCnt1, PointsCnt2, PointsCnt3
        });

    Scale = std::max((Dest.w) / Src.w, (Dest.h) / Src.h);

    if (this->AutoFlush)
        Flush();

    return true;
}

bool SDFTexture::Flush()
{
    if (Indices.size() > 0) {
        Texture.Draw(Outline, this->Color, this->OutlineColor, UV, squareVertices, Indices, Scale, (GLfloat)Border, Spread);
    }
    Clear();
    return true;
};

class MyStream {
private:
    const char* pointer;
    const char* end;
    std::string line;
public:
    MyStream(const char* Pointer, int Size) : pointer(Pointer), end(Pointer + Size) {};
    bool ReadLine(std::string& line) {
        constexpr char c13 = 13;
        constexpr char c10 = 10;
        line.clear();
        decltype(pointer) start = nullptr;
        while (pointer != end)
        {
            if (c13 == *pointer || c10 == *pointer) {
                if (start) {
                    break;
                }
            }
            else if (!start) {
                start = pointer;
            }
            ++pointer;
        }

        if (start) {
            line = std::string(start, pointer - start);
            return true;
        }
        return false;
    };
};

template <class myStream>
bool SDFFont::ParseFontMap(myStream& fonsStream)
{
    /*
    http://www.angelcode.com/products/bmfont/doc/file_format.html
    http://www.angelcode.com/products/bmfont/doc/export_options.html
    https://www.gamedev.net/forums/topic/284560-bmfont-and-how-to-interpret-the-fnt-file/
    */

    std::string line;
    decltype(line.find(',')) lpos;
    decltype(lpos)           rpos;
    SDFCharInfo sd;

    while (fonsStream.ReadLine(line))
        if (!line.empty()) {
            auto getUnsignedValue = [](const char* p) {
                unsigned Value{ 0 };
                for (; '0' <= *p && *p <= '9'; ++p) {
                    Value = Value * 10 + static_cast<decltype(Value)>(*p - '0');
                }
                return Value;
            };

            auto getSignedValue = [](const char* p) {
                int Value{ 0 }, Sign{ 1 };
                if ('-' == *p) {
                    Sign = -1;
                    ++p;
                }
                for (; '0' <= *p && *p <= '9'; ++p) {
                    Value = Value * 10 + static_cast<decltype(Value)>(*p - '0');
                }
                return Value * Sign;
            };

            if (std::strncmp(line.c_str(), "char id", 7) == 0) {
                lpos = 0;
                rpos = 0;

                rpos = line.find("id=", lpos) + 3;
                if (line[rpos] == '\"') ++rpos;
                auto id = getUnsignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("x=", lpos) + 2;
                if (line[rpos] == '\"') ++rpos;
                sd.x = getUnsignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("y=", lpos) + 2;
                if (line[rpos] == '\"') ++rpos;
                sd.y = getUnsignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("width=", lpos) + 6;
                if (line[rpos] == '\"') ++rpos;
                sd.w = getUnsignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("height=", lpos) + 7;
                if (line[rpos] == '\"') ++rpos;
                sd.h = getUnsignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("xoffset=", lpos) + 8;
                if (line[rpos] == '\"') ++rpos;
                sd.xoffset = getSignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("yoffset=", lpos) + 8;
                if (line[rpos] == '\"') ++rpos;
                sd.yoffset = getSignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("xadvance=", lpos) + 9;
                if (line[rpos] == '\"') ++rpos;
                sd.xadvance = getSignedValue(line.c_str() + rpos);

                CharsMap.emplace(id, sd);

                //logDebug("dx=%d, dy=%d, xa=%d", dx, dy, xa);
            }
            //else if (line.find("chars", 0) != std::string::npos)
            //{
            //    auto cnt = BWrapper::Str2Num(std::string(line, line.find("\"", 0) + 1).c_str());
            //    CharsVector.reserve(cnt);
            //}
            if (std::strncmp(line.c_str(), "common", 6) == 0) {
                rpos = line.find("lineHeight=", 0) + 11;
                if (line[rpos] == '\"') ++rpos;
                LineHeight = getUnsignedValue(line.c_str() + rpos);

                rpos = line.find("scaleW=", 0) + 7;
                if (line[rpos] == '\"') ++rpos;
                ScaleW = getUnsignedValue(line.c_str() + rpos);

                rpos = line.find("scaleH=", 0) + 7;
                if (line[rpos] == '\"') ++rpos;
                ScaleH = getUnsignedValue(line.c_str() + rpos);

                //logDebug("ScaleW = %d, ScaleH = %d", ScaleW, ScaleH);
            };
        };
    return true;
};

bool SDFFont::LoadCharMapFromMemory(const char* Buffer, int Size)
{
    CharsMap.clear();
    std::string line;
    MyStream ms(Buffer, Size);
    return ParseFontMap(ms);
};

bool SDFFont::ParseFNTFile(const char* FNTFile, BWrapper::FileSearchPriority SearchPriority)
{
    CharsMap.clear();
    FileReader fr;
    if (fr.Open(FNTFile, SearchPriority))
    {
        ParseFontMap(fr);
        fr.Close();
        return true;
    }
    return false;
}

bool SDFFont::Draw(bool Outline, const AkkordColor& FontColor, const AkkordColor& OutlineColor, const std::vector<GLfloat>& UV, const std::vector<GLfloat>& squareVertices, const std::vector <GLushort>& Indices, GLfloat Scale, GLfloat Border)
{
    FontAtlas.Draw(Outline, FontColor, OutlineColor, UV, squareVertices, Indices, Scale, Border, Spread);
    return true;
};

// Для рисования всегда указывать левую верхнюю точку (удобно для разгаданных слов в "составь слова")

AkkordPoint SDFFontBuffer::GetTextSizeByLine(const char* Text, std::vector<int>& VecSize)
{
    // VecSize - вектор строк (в надписи может быть несколько строк, нужно считать длину каждой строки отдельно - для выравнивания)

    AkkordPoint pt, localpoint;
    pt = localpoint = AkkordPoint(0, 0);

    unsigned int a = 0;
    unsigned int i = 0;

    SDFCharInfo charParams;

    if (Text != nullptr)
    {
        do
        {
            a = UTF2Unicode(Text, i);

            if (a == 0)
            {
                break;
            }
            else if (a == 10) // Если это переход строки
            {
                pt.y += static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight()); // надо учесть общую высоту строки
                VecSize.emplace_back(localpoint.x);
                pt.x = std::max(pt.x, localpoint.x);
                localpoint.x = 0;
            }
            else if (a != 13) // Если это не переход строки
            {
                sdfFont->GetCharInfo(a, charParams);

                localpoint.x += static_cast<decltype(localpoint.x)>(scaleX * charParams.xoffset);
                localpoint.x += static_cast<decltype(localpoint.x)>(scaleX * charParams.xadvance);
            }
        } while (true);
    }

    VecSize.emplace_back(localpoint.x);
    pt.x = std::max(pt.x, localpoint.x);

    //pt.y += localpoint.y; // надо учесть общую высоту строки
    pt.y = static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight() * VecSize.size());
    return pt;
}

void SDFFontBuffer::Flush()
{
    if (Indices.size() > 0) {
        sdfFont->Draw(this->outline, this->color, this->outlineColor, UV, squareVertices, Indices, (GLfloat)this->scaleX, (GLfloat)this->Border);
    }
    Clear();
};

AkkordPoint SDFFontBuffer::DrawText(int X, int Y, const char* Text)
{
    AkkordPoint pt = AkkordPoint(0, 0);
    float px1, px2, py1, py2;
    if (Text != nullptr)
    {
        std::vector<int> VecSize;
        AkkordPoint size;
        pt = size = GetTextSizeByLine(Text, VecSize);
        decltype(X) x_start, x_current;
        pt = AkkordPoint(0, 0);

        unsigned int a = 0;

        unsigned i = 0;

        auto atlasW = static_cast<float>(sdfFont->GetAtlasW());
        auto atlasH = static_cast<float>(sdfFont->GetAtlasH());

        auto ScreenSize = BWrapper::GetScreenSize();

        float ScrenW = static_cast<decltype(ScrenW)>(ScreenSize.x);
        float ScrenH = static_cast<decltype(ScrenH)>(ScreenSize.y);

        SDFCharInfo charParams;

        unsigned line = 0;
        decltype (Indices)::value_type PointsCnt = static_cast<decltype (Indices)::value_type>(UV.size() / 2); // Разделив на 2, получаем количество вершин

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

    check_h_align:
        // Выбираем начальную точку в зависимости от выравнивания
        switch (alignH)
        {
        case SDFFont::AlignH::Center:
            x_start = X + (rectW - VecSize[line]) / 2;
            break;
        case SDFFont::AlignH::Right:
            x_start = X + (rectW - VecSize[line]);
            break;
        default:
            x_start = X;
            break;
        };

        x_current = x_start;

        do
        {
            a = UTF2Unicode(Text, i);

            if (a == 0)
            {
                break;
            }
            else if (a == 10)
            {
                ++line;
                Y += static_cast<decltype(Y)>(scaleY * sdfFont->GetLineHeight());

                pt.x = std::max(pt.x, x_current - x_start + 1);
                goto check_h_align;
            }
            else if (a == 13) {} // ничего не делаем
            else
            {
                sdfFont->GetCharInfo(a, charParams);

                x_current += static_cast<decltype(x_current)>(scaleX * charParams.xoffset);

                //const decltype(charParams.w) minus = 0;

                px1 = float(charParams.x) / atlasW;
                px2 = float(charParams.x + charParams.w /*- minus */) / atlasW;
                py1 = float(charParams.y + charParams.h /*- minus */) / atlasH;
                py2 = float(charParams.y) / atlasH;

                UV.insert(UV.cend(),
                    {
                        px1, py1,
                        px2, py1,
                        px1, py2,
                        px2, py2
                    });

                px1 = 2 * ((float)x_current / ScrenW) - 1.0f;
                px2 = 2 * ((float)x_current + scaleX * (charParams.w - 1.0f)) / ScrenW - 1.0f;
                py1 = 2 * (ScrenH - Y - scaleY * (charParams.h + charParams.yoffset)) / ScrenH - 1.0f;
                py2 = 2 * (ScrenH - Y - scaleY * charParams.yoffset) / ScrenH - 1.0f;

                squareVertices.insert(squareVertices.cend(),
                    {
                        px1, py1,
                        px2, py1,
                        px1, py2,
                        px2, py2
                    });

                {
                    auto& PointsCnt0 = PointsCnt;
                    decltype(PointsCnt) PointsCnt1 = PointsCnt + 1;
                    decltype(PointsCnt) PointsCnt2 = PointsCnt + 2;
                    decltype(PointsCnt) PointsCnt3 = PointsCnt + 3;

                    Indices.insert(Indices.cend(),
                        {
                            PointsCnt0, PointsCnt1, PointsCnt2,
                            PointsCnt1, PointsCnt2, PointsCnt3
                        });
                }
                x_current = x_current + static_cast<decltype(x_current)>(scaleX * charParams.xadvance);
                PointsCnt += 4;
            }
        } while (true);

        pt.x = std::max(pt.x, x_current - x_start + 1);
        pt.y = static_cast<decltype(pt.y)>(scaleY * sdfFont->GetLineHeight() * VecSize.size());

        //logDebug("[%d %d] [%d %d]", size.x, size.y, pt.x, pt.y);
    }
    return pt;
};

void SDFFontBuffer::WrapText(const char* Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size)
{
    UsedScale = scaleX;
    auto font_line_height = sdfFont->GetLineHeight();
    const char* textPtr;

    // лямбда для поиска нового слова
    auto GetNextWord = [&textPtr]() {
        std::string res;
        for (; textPtr && (*textPtr) && (*textPtr) != ' ' && (*textPtr) != '\n'; ++textPtr)
            res += (*textPtr);
        return res;
    };

    // лямбда для определения размера слова в единицах шрифта
    auto GetWordSize = [](SDFFont* sdfFont, const char* Word, float uScale)
    {
        SDFCharInfo charParams;
        int xSize = 0;
        unsigned int i = 0, a = 0;
        while (1) {
            a = UTF2Unicode(Word, i);
            if (!a)
                break;
            sdfFont->GetCharInfo(a, charParams);
            xSize += static_cast<decltype(xSize)>(uScale * charParams.xadvance);
            xSize += static_cast<decltype(xSize)>(uScale * charParams.xoffset);
        };
        return xSize;
    };

    unsigned lines_cnt = 0;
    int x_pos = 0, max_line_len = 0, space_len = 0;
    SDFCharInfo charParams;
    std::string word;

    // сначала пробегаем по всем словам, и делаем так, чтобы каждое слово было меньше ширины выделенного под текст прямоугольника
    textPtr = Text;
    while (1) {
        const auto word = GetNextWord();
        while (1) {
            int xSize = GetWordSize(sdfFont, word.c_str(), UsedScale);
            if (xSize >= rectW)
            {
                UsedScale *= ScaleMutiplier;
            }
            else
            {
                break;
            }
        }

        // пропускаем ненужные пробелы и переходы на новой строку
        while (' ' == *textPtr || '\n' == *textPtr)
            ++textPtr;

        // если конец строки, выходим
        if ('\0' == *textPtr)
            break;
    };

repeat_again:
    ResultString.clear();
    textPtr = Text;
    lines_cnt = max_line_len = x_pos = 0;
    sdfFont->GetCharInfo(32 /* space */, charParams);
    space_len = static_cast<decltype(space_len)>(UsedScale * charParams.xoffset) + static_cast<decltype(space_len)>(UsedScale * charParams.xadvance);
    while (1)
    {
        const auto word = GetNextWord();
        const auto xSize = GetWordSize(sdfFont, word.c_str(), UsedScale);

        // если в строке уже есть слово
        if (x_pos != 0)
        {
            // если новое еще помещается в текущую строку
            if (x_pos + space_len + xSize <= rectW)
            {
                x_pos += (space_len + xSize);
                ResultString += " ";
                ResultString += word;
                max_line_len = std::max(max_line_len, x_pos);
            }
            else // если новое уже не помещается в текущую строку
            {
                ++lines_cnt;

                // проверить, не вышли ли за диапазон по высоте
                if (static_cast<int>(UsedScale * font_line_height * (lines_cnt + 1) > rectH))
                {
                    UsedScale *= ScaleMutiplier;
                    goto repeat_again;
                }

                ResultString += '\n';
                ResultString += word;
                x_pos = xSize;
                max_line_len = std::max(max_line_len, x_pos);
            }
        }
        else // 0 == x_pos
        {
            x_pos += xSize;
            ResultString += word;
            max_line_len = std::max(max_line_len, x_pos);
        }

        // пропускаем ненужные пробелы
        while (' ' == *textPtr)
            ++textPtr;

        // если конец строки, выходим
        if ('\0' == *textPtr)
            break;

        while ('\n' == *textPtr)
        {
            // переход к след строке
            ++lines_cnt;
            // проверить, не вышли ли за диапазон по высоте
            if (static_cast<int>(UsedScale * font_line_height * (lines_cnt + 1) > rectH))
            {
                UsedScale *= ScaleMutiplier;
                goto repeat_again;
            }
            ++textPtr;
            x_pos = 0;
            ResultString += '\n';
        }
    };

    Size = AkkordPoint(max_line_len, static_cast<int>(UsedScale * font_line_height * (lines_cnt + 1)));
};