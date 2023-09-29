#ifndef __AKK0RD_SDK_CustomEvents_H__
#define __AKK0RD_SDK_CustomEvents_H__

#include "core/core_defines.h"
#include "basewrapper.h"

class CustomEvents
{
public:
    static void MessageBoxCallback(int Code, int Result)
    {
        //logDebug("MessageBoxCallback %d %d", Code, Result);
        SDL_Event Event;
        Event.user.type = msgBox::GetEventCode();
        Event.user.code = (Sint32)Code;
        Event.user.data1 = (void*)(uintptr_t)Result;
        SDL_PushEvent(&Event);
    };
};

#endif // __AKK0RD_SDK_CustomEvents_H__