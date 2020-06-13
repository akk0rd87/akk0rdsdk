#pragma once
#ifndef __AKK0RD_ADRANDOMIZER_H__
#define __AKK0RD_ADRANDOMIZER_H__

#include "basewrapper.h"
class AdRandomizer
{
public:
    enum struct Apps : unsigned { CFCross, FCross, JCross, JDraw, WordsRu1, WordsRu2, WordsRu8, WordsUs, Sudoku, DotLines, PirateBomb /*, CJCross*/ };
    static void    OpenURL(Apps AppId);
    static void    OpenURL(BWrapper::OS OSCode, Apps AppId);
    static void    OpenPublisherAppstorePage();

    //Запрещаем создавать экземпляр класса AdRandomizer
    AdRandomizer() = delete;
    ~AdRandomizer() = delete;
    AdRandomizer(const AdRandomizer& rhs)  = delete; // Копирующий: конструктор
    AdRandomizer(AdRandomizer&& rhs) = delete; // Перемещающий: конструктор
    AdRandomizer& operator= (const AdRandomizer& rhs) = delete; // Оператор копирующего присваивания
    AdRandomizer& operator= (AdRandomizer&& rhs) = delete; // Оператор перемещающего присваивания
};

#endif // __AKK0RD_ADRANDOMIZER_H__
