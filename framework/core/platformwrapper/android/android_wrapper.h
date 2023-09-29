#ifndef __AKK0RD_SDK_ANDROID_BASEWRAPPER_H__
#define __AKK0RD_SDK_ANDROID_BASEWRAPPER_H__

#include "basewrapper.h"

class AndroidWrapper
{
private:
public:
    static bool              AndroidShowToast(const char* Message, BWrapper::AndroidToastDuration Duration, int Gravity, int xOffset, int yOffset);
    static int               AndroidGetApiLevel();

    //Запрещаем создавать экземпляр класса AndroidWrapper
    AndroidWrapper() = delete;
    ~AndroidWrapper() = delete;
    AndroidWrapper(AndroidWrapper& rhs) = delete; // Копирующий: конструктор
    AndroidWrapper(AndroidWrapper&& rhs) = delete; // Перемещающий: конструктор
    AndroidWrapper& operator= (AndroidWrapper&& rhs) = delete; // Оператор перемещающего присваивания
};

#endif // __AKK0RD_SDK_ANDROID_BASEWRAPPER_H__