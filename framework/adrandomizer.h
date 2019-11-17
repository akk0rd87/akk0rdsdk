#pragma once
#ifndef __AKK0RD_ADRANDOMIZER_H__
#define __AKK0RD_ADRANDOMIZER_H__

#include "basewrapper.h"
class AdRandomizer
{
public:
    enum struct Apps : unsigned { CFCross, FCross, JCross, JDraw, WordsRu1, WordsRu2, WordsRu8, WordsUs, Sudoku, DotLines, PirateBomb /*, CJCross*/};
    static void    OpenURL(Apps AppId);
    static void    OpenPublisherAppstorePage();
private:
    static void OpenURL_private(BWrapper::OS OSCode, Apps AppId);
};

#endif // __AKK0RD_ADRANDOMIZER_H__
