#pragma once
#ifndef __AKK0RD_CustomEvents_H__
#define __AKK0RD_CustomEvents_H__

#include "core/core_defines.h"
#include "basewrapper.h"

/*
class BaseCustomEvent // абстрактный базовый класс пользовательского события
{

};

class TestEvent : BaseCustomEvent
{
public:
    std::string Name;
    ~TestEvent()
    { 
        Name.clear(); 
        logDebug("Destructor");
    };
};


class CustomEvents
{
public:
    enum struct SDKEventType : Sint32 { Test, AdCallback };
    enum struct EventLevel : unsigned { User, SDK, Unknown };

    static void GenerateSDKEvent(CustomEvents::SDKEventType Type, BaseCustomEvent* Data)
    {
        SDL_Event Event;
        Event.user.type  = SDL_USEREVENT;
        Event.user.code  = (Sint32)Type;
        Event.user.data1 = Data;
        Event.user.data2 = (void*)1; // ставим константу 1
        
        SDL_PushEvent(&Event);
    };

    static void GenerateUserEvent(CustomEvents::SDKEventType Type, BaseCustomEvent* Data)
    {
        SDL_Event Event;
        Event.user.type = SDL_USEREVENT;
        Event.user.code = (Sint32)Type;
        Event.user.data1 = Data;
        Event.user.data2 = (void*)2; // ставим константу 2

        SDL_PushEvent(&Event);
    };

    static CustomEvents::EventLevel GetEventLevel(SDL_Event& Event)
    {        
        if (Event.user.type != SDL_USEREVENT)
        {
            logError("Event type is not SDL_USEREVENT [%d]", Event.user.type);
            return EventLevel::Unknown;
        }
        
        if ((void*)1 == Event.user.data2)
            return EventLevel::SDK;

        if ((void*)2 == Event.user.data2)
            return EventLevel::User;

        logError("Unknown event type!");
        return EventLevel::Unknown;
    };
};
*/

class CustomEvents
{
public:
	static void MessageBoxCallback(int Code, int Result)
	{
		//logDebug("MessageBoxCallback %d %d", Code, Result);
		SDL_Event Event;
		Event.user.type = msgBox::GetEventCode();
		Event.user.code =  (Sint32)Code;
		Event.user.data1 = (void*)(uintptr_t)Result;
		SDL_PushEvent(&Event);
	};
};

#endif // __AKK0RD_CustomEvents_H__