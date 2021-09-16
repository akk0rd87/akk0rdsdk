#include "videodriver.h"
#include "core/videoadapter/video_interface.h"

//static std::unique_ptr<VideoAdapter> videoAdapter = nullptr;
static VideoAdapter* videoAdapter = nullptr;

/*
https://github.com/libgdx/libgdx/wiki/Hiero
"java -cp gdx.jar;gdx-natives.jar;gdx-backend-lwjgl.jar;gdx-backend-lwjgl-natives.jar;extensions\gdx-freetype\gdx-freetype.jar;extensions\gdx-freetype\gdx-freetype-natives.jar;extensions\gdx-tools\gdx-tools.jar com.badlogic.gdx.tools.hiero.Hiero"
*/

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

static bool InitVideoBuffer(std::unique_ptr<VideoBuffer>& videoBuffer) {
    if (videoAdapter) {
        videoBuffer = videoAdapter->CreateVideoBuffer();
        if (!videoBuffer) {
            logError("videoBuffer create error");
            return false;
        }
    }
    else {
        logError("videoAdapter is not initialized!");
        return false;
    }
    return true;
}

static struct { // разделяемый буффер, который используется эксклюзивно только в рамках одного вызова в потоке рисования
    std::string strObject;
    std::vector<float> floatVector;
} SharedPool;

bool SDFGLTexture::Draw(std::unique_ptr<VideoBuffer>& sdfVideoBuffer, bool Outline, const AkkordColor& FontColor, const AkkordColor& OutlineColor, float Scale, float Border, int Spread)
{
    if (sdfVideoBuffer) {
        VideoSDFBufferDrawParams SDFParams;
        SDFParams.Texture = akkordTexture.GetTexture();
        SDFParams.Color = &FontColor;
        SDFParams.Outline = Outline;
        SDFParams.OutlineColor = &OutlineColor;
        SDFParams.Border = Border;
        SDFParams.Scale = Scale;
        SDFParams.Spread = Spread;

        sdfVideoBuffer->DrawSDF(SDFParams);
    }
    else {
        logError("sdfVideoBuffer is empty!");
        return false;
    }

    return true;
};

void SDFTexture::Clear() {
    if (this->videoBuffer) {
        this->videoBuffer->Clear();
    }
}

bool SDFTexture::Draw(const AkkordFRect& DestRect, const AkkordRect* SourceRect) {
    const auto ScreenSize = BWrapper::GetScreenSize();
    // если целевое размещение не попадает на экран, не рисуем его
    if (DestRect.x > ScreenSize.x || DestRect.x + DestRect.w < 0 || DestRect.y > ScreenSize.y || DestRect.y + DestRect.h < 0) {
        return false;
    }

    if (!videoBuffer) {
        if (!InitVideoBuffer(videoBuffer)) {
            return false;
        }
    }

    // не забыть посчитать Scale и выполнить AutoFlush
    VideoFloatRect srcRect;

    VideoBufferAppendParams appendParams;
    appendParams.DestRect = &DestRect;
    appendParams.SrcRect = &srcRect;
    appendParams.TextureW = atlasW;
    appendParams.TextureH = atlasH;
    appendParams.ScrenW = static_cast<decltype(appendParams.ScrenW)>(ScreenSize.x);
    appendParams.ScrenH = static_cast<decltype(appendParams.ScrenH)>(ScreenSize.y);

    if (SourceRect) {
        srcRect.x = static_cast<decltype(srcRect.x)>(SourceRect->x);
        srcRect.y = static_cast<decltype(srcRect.y)>(SourceRect->y);
        srcRect.w = static_cast<decltype(srcRect.w)>(SourceRect->w);
        srcRect.h = static_cast<decltype(srcRect.h)>(SourceRect->h);
    }
    else {
        srcRect.x = 0.0F;
        srcRect.y = 0.0F;
        srcRect.w = atlasW;
        srcRect.h = atlasH;
    }
    videoBuffer->Append(appendParams);
    this->Scale = std::max(DestRect.w / srcRect.w, DestRect.h / srcRect.h);

    if (this->AutoFlush) {
        Flush();
    }

    return true;
}

bool SDFTexture::Flush()
{
    if (this->videoBuffer) {
        Texture.Draw(this->videoBuffer, Outline, this->Color, this->OutlineColor, Scale, Border, Spread);
    }
    Clear();
    return true;
};

class MyStream {
private:
    const char* pointer;
    const char* end;
public:
    MyStream(const char* Pointer, int Size) : pointer(Pointer), end(Pointer + Size) {};
    bool ReadLine(std::string& line) {
        constexpr char c13{ 13 };
        constexpr char c10{ 10 };
        line.clear();
        decltype(pointer) start = nullptr;
        while (pointer != end) {
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
            line.assign(start, pointer - start);
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

    auto& line = SharedPool.strObject;
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
                const auto id = getUnsignedValue(line.c_str() + rpos);

                lpos = rpos;
                rpos = line.find("x=", lpos) + 2;
                if (line[rpos] == '\"') ++rpos;
                sd.x = static_cast<decltype(sd.x)>(getUnsignedValue(line.c_str() + rpos));

                lpos = rpos;
                rpos = line.find("y=", lpos) + 2;
                if (line[rpos] == '\"') ++rpos;
                sd.y = static_cast<decltype(sd.y)>(getUnsignedValue(line.c_str() + rpos));

                lpos = rpos;
                rpos = line.find("width=", lpos) + 6;
                if (line[rpos] == '\"') ++rpos;
                sd.w = static_cast<decltype(sd.w)>(getUnsignedValue(line.c_str() + rpos));

                lpos = rpos;
                rpos = line.find("height=", lpos) + 7;
                if (line[rpos] == '\"') ++rpos;
                sd.h = static_cast<decltype(sd.h)>(getUnsignedValue(line.c_str() + rpos));

                lpos = rpos;
                rpos = line.find("xoffset=", lpos) + 8;
                if (line[rpos] == '\"') ++rpos;
                sd.xoffset = static_cast<decltype(sd.xoffset)>(getSignedValue(line.c_str() + rpos));

                lpos = rpos;
                rpos = line.find("yoffset=", lpos) + 8;
                if (line[rpos] == '\"') ++rpos;
                sd.yoffset = static_cast<decltype(sd.yoffset)>(getSignedValue(line.c_str() + rpos));

                lpos = rpos;
                rpos = line.find("xadvance=", lpos) + 9;
                if (line[rpos] == '\"') ++rpos;
                sd.xadvance = static_cast<decltype(sd.xadvance)>(getSignedValue(line.c_str() + rpos));

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
                LineHeight = static_cast<decltype(LineHeight)>(getUnsignedValue(line.c_str() + rpos));

                rpos = line.find("scaleW=", 0) + 7;
                if (line[rpos] == '\"') ++rpos;
                ScaleW = static_cast<decltype(ScaleW)>(getUnsignedValue(line.c_str() + rpos));

                rpos = line.find("scaleH=", 0) + 7;
                if (line[rpos] == '\"') ++rpos;
                ScaleH = static_cast<decltype(ScaleH)>(getUnsignedValue(line.c_str() + rpos));
            };
        };
    return true;
};

bool SDFFont::LoadCharMapFromMemory(const char* Buffer, int Size)
{
    CharsMap.clear();
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

void SDFFontBuffer::Clear() {
    if (this->videoBuffer) {
        this->videoBuffer->Clear();
    }
}

void SDFFontBuffer::Reserve(unsigned Count) {
    if (!videoBuffer) {
        if (!InitVideoBuffer(videoBuffer)) {
            return;
        }
    }
    this->videoBuffer->Reserve(Count);
}

void SDFFontBuffer::Flush() {
    sdfFont->FontAtlas.Draw(videoBuffer, outline, color, outlineColor, scaleX, Border, sdfFont->Spread);
    Clear();
}

std::pair<float, float> SDFFontBuffer::GetTextSizeByLine(const char* Text, float ParamScaleX, float ParamScaleY, std::vector<float>* VecSize) const
{
    // VecSize - вектор строк (в надписи может быть несколько строк, нужно считать длину каждой строки отдельно - для выравнивания)
    std::pair<float, float> pt(0.0F, 0.0F); // вместо точки с X и Y
    if (Text != nullptr) {
        unsigned i{ 0 }, linesCount{ 0 };;
        float localPointX{ 0.0F };
        SDFFont::SDFCharInfo charParams;
        while (true) {
            const auto a = UTF2Unicode(Text, i);
            switch (a) {
            case 0: // конец строки
                goto after_while;
                break;

            case 10: // Если это переход строки
                ++linesCount;
                if (VecSize) {
                    VecSize->push_back(localPointX);
                }
                pt.first = std::max(pt.first, localPointX);
                localPointX = 0.0F;
                break;

            case 13: // Ничего не делаем
                break;

            default:
                sdfFont->GetCharInfo(a, charParams);
                localPointX += ParamScaleX * (charParams.xoffset + charParams.xadvance);
                break;
            }
        }

    after_while:
        ++linesCount;
        if (VecSize) {
            VecSize->push_back(localPointX);
        }
        pt.first = std::max(pt.first, localPointX);
        // надо учесть общую высоту строки
        pt.second = ParamScaleY * sdfFont->GetLineHeight() * static_cast<decltype(ParamScaleY)>(linesCount);
    }
    return pt;
}

AkkordPoint SDFFontBuffer::DrawText(int X, int Y, const char* Text) {
    AkkordPoint pt(0, 0);
    if (!videoBuffer) {
        if (!InitVideoBuffer(videoBuffer)) {
            return pt;
        }
    }

    if (Text != nullptr) {
        const auto ScreenSize = BWrapper::GetScreenSize();
        // не забыть посчитать Scale и выполнить AutoFlush
        VideoFloatRect destRect, srcRect;

        VideoBufferAppendParams appendParams;
        appendParams.DestRect = &destRect;
        appendParams.SrcRect = &srcRect;
        appendParams.TextureW = sdfFont->GetAtlasW();
        appendParams.TextureH = sdfFont->GetAtlasH();
        appendParams.ScrenW = static_cast<decltype(appendParams.ScrenW)>(ScreenSize.x);
        appendParams.ScrenH = static_cast<decltype(appendParams.ScrenH)>(ScreenSize.y);

        SharedPool.floatVector.clear();
        const auto size = GetTextSizeByLine(Text, scaleX, scaleY, &SharedPool.floatVector);
        float x_start, x_current, y_current{ static_cast<float>(Y) };
        unsigned i{ 0 }, line{ 0 };

        SDFFont::SDFCharInfo charParams;

        switch (alignV)
        {
        case SDFFont::AlignV::Center:
            y_current += (rectH - size.second) / 2;
            break;
        case SDFFont::AlignV::Bottom:
            y_current += (rectH - size.second);
            break;
        default: // в остальных случаях ничего не делаем, координату Y не меняем
            break;
        };

    check_h_align:
        // Выбираем начальную точку в зависимости от выравнивания
        x_start = static_cast<decltype(x_start)>(X);
        switch (alignH)
        {
        case SDFFont::AlignH::Center:
            x_start += (rectW - SharedPool.floatVector[line]) / 2.0F;
            break;
        case SDFFont::AlignH::Right:
            x_start += (rectW - SharedPool.floatVector[line]);
            break;
        default:
            break;
        };

        x_current = x_start;

        while (true) {
            const auto a = UTF2Unicode(Text, i);
            switch (a)
            {
            case 0: // выйти
                goto after_cycle;
                break;

            case 10: // переход строки
                ++line;
                y_current += scaleY * sdfFont->GetLineHeight();
                pt.x = std::max(pt.x, static_cast<decltype(pt.x)>(x_current - x_start + 1.0F));
                goto check_h_align;
                break;

            case 13: // ничего не делаем
                break;

            default:
                sdfFont->GetCharInfo(a, charParams);
                x_current += scaleX * charParams.xoffset;

                srcRect.x = charParams.x;
                srcRect.y = charParams.y;
                srcRect.w = charParams.w;
                srcRect.h = charParams.h;

                destRect.x = x_current;
                destRect.y = y_current + scaleY * charParams.yoffset;
                destRect.w = scaleX * charParams.w;
                destRect.h = scaleY * charParams.h;

                videoBuffer->Append(appendParams);

                x_current += scaleX * charParams.xadvance;
                break;
            }
        }

    after_cycle:
        pt.x = std::max(pt.x, static_cast<decltype(pt.x)>(std::ceil(x_current - x_start + 1.0F)));
        pt.y = static_cast<decltype(pt.y)>(std::ceil(scaleY * sdfFont->GetLineHeight() * SharedPool.floatVector.size()));
    }
    return pt;
};

void SDFFontBuffer::WrapText(const char* Text, float ScaleMutiplier, std::string& ResultString, float& UsedScale, AkkordPoint& Size)
{
    UsedScale = scaleX;
    const auto font_line_height = sdfFont->GetLineHeight();
    const char* textPtr{ Text };

    // лямбда для поиска нового слова
    auto GetNextWord = [&textPtr](std::string& Word) {
        const auto pBegin = textPtr;
        while (textPtr && (*textPtr) && (*textPtr) != ' ' && (*textPtr) != '\n') {
            ++textPtr;
        }
        if (textPtr > pBegin) {
            Word.assign(pBegin, textPtr - pBegin);
        }
        else {
            Word.clear();
        }
    };

    // лямбда для определения размера слова в единицах шрифта
    auto GetWordSize = [](SDFFont* sdfFont, const char* Word) {
        SDFFont::SDFCharInfo charParams;
        float xSize{ 0.0F };
        unsigned int i{ 0 };
        while (1) {
            const auto a = UTF2Unicode(Word, i);
            if (!a) {
                break;
            }
            sdfFont->GetCharInfo(a, charParams);
            xSize += charParams.xadvance;
            xSize += charParams.xoffset;
        };
        return xSize;
    };

    unsigned lines_cnt = 0;
    float x_pos = 0, max_line_len = 0;
    SDFFont::SDFCharInfo charParams;

    // сначала пробегаем по всем словам, и делаем так, чтобы каждое слово было меньше ширины выделенного под текст прямоугольника
    while (1) {
        GetNextWord(SharedPool.strObject);
        while (1) {
            const auto xSize = UsedScale * GetWordSize(sdfFont, SharedPool.strObject.c_str());
            if (xSize >= rectW) {
                UsedScale *= ScaleMutiplier;
            }
            else {
                break;
            }
        }

        // пропускаем ненужные пробелы и переходы на новой строку
        while (' ' == *textPtr || '\n' == *textPtr || '\r' == *textPtr) {
            ++textPtr;
        }

        // если конец строки, выходим
        if ('\0' == *textPtr)
            break;
    };

repeat_again:
    ResultString.clear();
    textPtr = Text;
    lines_cnt = 0;
    max_line_len = x_pos = 0.0F;
    sdfFont->GetCharInfo(32 /* space */, charParams);
    const auto space_len = UsedScale * (charParams.xoffset + charParams.xadvance);
    while (1) {
        GetNextWord(SharedPool.strObject);
        const auto xSize = UsedScale * GetWordSize(sdfFont, SharedPool.strObject.c_str());

        // если в строке уже есть слово
        if (static_cast<int>(x_pos) != 0) {
            // если новое еще помещается в текущую строку
            if (x_pos + space_len + xSize <= rectW) {
                x_pos += (space_len + xSize);
                ResultString += " ";
                ResultString += SharedPool.strObject;
                max_line_len = std::max(max_line_len, x_pos);
            }
            else { // если новое уже не помещается в текущую строку
                ++lines_cnt;

                // проверить, не вышли ли за диапазон по высоте
                if (UsedScale * font_line_height * (lines_cnt + 1) > rectH) {
                    UsedScale *= ScaleMutiplier;
                    goto repeat_again;
                }

                ResultString += '\n';
                ResultString += SharedPool.strObject;
                x_pos = xSize;
                max_line_len = std::max(max_line_len, x_pos);
            }
        }
        else { // 0 == x_pos
            x_pos += xSize;
            ResultString += SharedPool.strObject;
            max_line_len = std::max(max_line_len, x_pos);
        }

        // пропускаем ненужные пробелы
        while (' ' == *textPtr || '\r' == *textPtr) {
            ++textPtr;
        }

        // если конец строки, выходим
        if ('\0' == *textPtr) {
            break;
        }

        while ('\n' == *textPtr) {
            // переход к след строке
            ++lines_cnt;
            // проверить, не вышли ли за диапазон по высоте
            if (UsedScale * font_line_height * (lines_cnt + 1) > rectH) {
                UsedScale *= ScaleMutiplier;
                goto repeat_again;
            }
            ++textPtr;
            x_pos = 0.0F;
            ResultString += '\n';
        }
    };

    Size = AkkordPoint(static_cast<int>(max_line_len), static_cast<int>(UsedScale * font_line_height * (lines_cnt + 1)));
};

bool VideoDriver::Init(const VideoDriver::Feature Features) {
    //videoAdapter = VideoAdapter::CreateVideoAdapter();
    videoAdapter = VideoAdapter::GetInstance();
    if (videoAdapter) {
        videoAdapter->PreInit();

        if (!!(Features & VideoDriver::Feature::SDF)) {
            videoAdapter->InitSDFPlain();
        }

        if (!!(Features & VideoDriver::Feature::SDF_Outline)) {
            videoAdapter->InitSDFOutline();
        }

        if (!!(Features & VideoDriver::Feature::Gradient)) {
            videoAdapter->InitGradient();
        }

        videoAdapter->PostInit();
    }
    else {
        logError("CreateVideoAdapter error!");
        return false;
    }

    return true;
};

void VideoDriver::ShutDown() {
    //if (videoAdapter) {
    //    videoAdapter.reset();
    //}
};

bool VideoDriver::DrawLinearGradientRect(const AkkordRect& Rect, const AkkordColor& X0Y0, const AkkordColor& X1Y0, const AkkordColor& X1Y1, const AkkordColor& X0Y1) {
    BWrapper::FlushRenderer();
    videoAdapter->DrawLinearGradientRect(Rect, X0Y0, X1Y0, X1Y1, X0Y1);
    return true;
};

SDFTexture::~SDFTexture() { Clear(); Texture.Clear(); };
SDFTexture::SDFTexture() = default;
SDFTexture::SDFTexture(SDFTexture && rhs) = default; // Перемещающий: конструктор

SDFFontBuffer::SDFFontBuffer() : sdfFont{ nullptr } {};
SDFFontBuffer::SDFFontBuffer(SDFFont * Font, unsigned int DigitsCount, const AkkordColor & Color) : sdfFont{ Font }, color{ Color }{ Reserve(DigitsCount); };
SDFFontBuffer::~SDFFontBuffer() = default;
SDFFontBuffer::SDFFontBuffer(SDFFontBuffer && rhs) = default; // Перемещающий: конструктор