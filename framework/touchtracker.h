#ifndef __AKK0RD_SDK_TOUCHTRACKER_H__
#define __AKK0RD_SDK_TOUCHTRACKER_H__

// Прижатые пальцы и кнопки мыши — по событиям SDL. Вместо счетчика ++/-- — набор SDL_FingerID: повторное или
// «чужое» событие число пальцев не меняет и в минус его не уводит. Кнопка мыши (Windows) — отдельный «палец».
// Игра вызывает FingerDown/FingerUp/MouseDown/MouseUp там же, где обрабатывает эти события, и получает число пальцев
// до и после события (prev — для «движение одним пальцем начинается только с Fingers == 1 && prevFingers == 0»).
// Флаги жестов игры трекер не знает: о сбросе (ResetOn, Sync) он сообщает, а свое состояние игра сбрасывает сама.
//
// SDL_GetTouchFingers для подсчета на каждом событии не годится: SDL обновляет пальцы, когда кладет событие в очередь,
// а игра разбирает очередь позже — при быстром тапе на SDL_EVENT_FINGER_DOWN SDL уже не видит этого пальца.

#include <algorithm>
#include <vector>
#include <SDL3/SDL.h>

class TouchTracker {
public:
    struct Fingers {
        int prev = 0;  // до события
        int count = 0; // после события
    };

    Fingers FingerDown(SDL_FingerID ID) {
        const int prev = Count();
        if (std::find(fingers.cbegin(), fingers.cend(), ID) == fingers.cend()) {
            fingers.push_back(ID);
        }
        return { prev, Count() };
    }

    // и для SDL_EVENT_FINGER_CANCELED
    Fingers FingerUp(SDL_FingerID ID) {
        const int prev = Count();
        fingers.erase(std::remove(fingers.begin(), fingers.end(), ID), fingers.end());
        return { prev, Count() };
    }

    Fingers MouseDown(Uint8 Button) {
        const int prev = Count();
        mouseButtons |= SDL_BUTTON_MASK(Button);
        return { prev, Count() };
    }

    Fingers MouseUp(Uint8 Button) {
        const int prev = Count();
        mouseButtons &= ~SDL_BUTTON_MASK(Button);
        return { prev, Count() };
    }

    int Count() const {
        int buttons = 0;
        for (auto mask = mouseButtons; mask; mask &= mask - 1) {
            ++buttons;
        }
        return static_cast<int>(fingers.size()) + buttons;
    }

    // Снимает все пальцы и кнопки; true — что-то было прижато
    bool Reset() {
        const bool wasPressed = Count() > 0;
        fingers.clear();
        mouseButtons = 0;
        return wasPressed;
    }

    // Уход в фон и потеря фокуса окна: отпускание после них может не прийти. true — состояние сброшено
    bool ResetOn(const SDL_Event& Event) {
        switch (Event.type) {
        case SDL_EVENT_WILL_ENTER_BACKGROUND:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            return Reset();
        default:
            return false;
        }
    }

    // Консервативная сверка с SDL — вызывать между пачками событий: если SDL не видит ни одного пальца, а у нас
    // они остались (отпускание потеряно), пальцы снимаются. Порядок проверок важен: SDL кладет SDL_EVENT_FINGER_UP
    // в очередь раньше, чем удаляет палец, поэтому если SDL пальцев не видит и в очереди нет событий касания,
    // все отпускания уже обработаны и живые пальцы не задеты. Кнопки мыши не трогает. true — пальцы сняты
    bool Sync() {
        if (fingers.empty() || SDLFingersCount() > 0 || SDL_HasEvents(SDL_EVENT_FINGER_DOWN, SDL_EVENT_FINGER_CANCELED)) {
            return false;
        }
        fingers.clear();
        return true;
    }

private:
    std::vector<SDL_FingerID> fingers; // пальцев единицы — вектор проще множества
    SDL_MouseButtonFlags mouseButtons = 0;

    static int SDLFingersCount() {
        int result = 0;
        int devices = 0;
        if (SDL_TouchID* touchIDs = SDL_GetTouchDevices(&devices)) {
            for (int i = 0; i < devices; ++i) {
                int count = 0;
                SDL_free(SDL_GetTouchFingers(touchIDs[i], &count));
                result += count;
            }
            SDL_free(touchIDs);
        }
        return result;
    }
};

#endif // __AKK0RD_SDK_TOUCHTRACKER_H__
