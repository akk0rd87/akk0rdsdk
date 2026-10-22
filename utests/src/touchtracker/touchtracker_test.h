#ifndef __AKK0RD_SDK_TOUCHTRACKER_TEST_H__
#define __AKK0RD_SDK_TOUCHTRACKER_TEST_H__

// TouchTracker без SDL во время выполнения: Sync (сверка с состоянием касаний SDL) здесь не вызывается,
// остальное — только заголовки SDL

#include "gtest/gtest.h"
#include "touchtracker.h"

static void ExpectFingers(const TouchTracker::Fingers& Fingers, int Prev, int Count) {
    EXPECT_EQ(Fingers.prev, Prev);
    EXPECT_EQ(Fingers.count, Count);
}

TEST(TestSDK, TouchTracker_DownUp) {
    TouchTracker tracker;
    ExpectFingers(tracker.FingerDown(10), 0, 1);
    ExpectFingers(tracker.FingerDown(20), 1, 2);
    ExpectFingers(tracker.FingerUp(10), 2, 1);
    ExpectFingers(tracker.FingerUp(20), 1, 0);
    EXPECT_EQ(tracker.Count(), 0);
}

// повторное нажатие тем же пальцем и отпускание неизвестного пальца число не меняют и в минус не уводят
TEST(TestSDK, TouchTracker_RepeatedAndUnknownEvents) {
    TouchTracker tracker;
    ExpectFingers(tracker.FingerDown(10), 0, 1);
    ExpectFingers(tracker.FingerDown(10), 1, 1);
    ExpectFingers(tracker.FingerUp(30), 1, 1);
    ExpectFingers(tracker.FingerUp(10), 1, 0);
    ExpectFingers(tracker.FingerUp(10), 0, 0);
    EXPECT_EQ(tracker.Count(), 0);
}

// отмена касания (SDL_EVENT_FINGER_CANCELED) — тот же FingerUp: после системного жеста пятью пальцами
// следующий палец снова первый
TEST(TestSDK, TouchTracker_CanceledGesture) {
    TouchTracker tracker;
    for (SDL_FingerID id = 1; id <= 5; ++id) {
        tracker.FingerDown(id);
    }
    EXPECT_EQ(tracker.Count(), 5);
    for (SDL_FingerID id = 5; id >= 1; --id) {
        tracker.FingerUp(id);
    }
    ExpectFingers(tracker.FingerDown(100), 0, 1);
}

// кнопки мыши (Windows) — отдельные «пальцы», вместе с касаниями
TEST(TestSDK, TouchTracker_MouseButtons) {
    TouchTracker tracker;
    ExpectFingers(tracker.MouseDown(SDL_BUTTON_LEFT), 0, 1);
    ExpectFingers(tracker.MouseDown(SDL_BUTTON_LEFT), 1, 1);
    ExpectFingers(tracker.MouseDown(SDL_BUTTON_RIGHT), 1, 2);
    ExpectFingers(tracker.FingerDown(10), 2, 3);
    ExpectFingers(tracker.MouseUp(SDL_BUTTON_LEFT), 3, 2);
    ExpectFingers(tracker.MouseUp(SDL_BUTTON_MIDDLE), 2, 2);
    ExpectFingers(tracker.MouseUp(SDL_BUTTON_RIGHT), 2, 1);
    ExpectFingers(tracker.FingerUp(10), 1, 0);
}

TEST(TestSDK, TouchTracker_Reset) {
    TouchTracker tracker;
    EXPECT_FALSE(tracker.Reset());
    tracker.FingerDown(10);
    tracker.MouseDown(SDL_BUTTON_LEFT);
    EXPECT_TRUE(tracker.Reset());
    EXPECT_EQ(tracker.Count(), 0);
    ExpectFingers(tracker.FingerUp(10), 0, 0); // отпускание после сброса — ничего не меняет
}

// сброс по уходу в фон и потере фокуса окна; другие события состояние не трогают
TEST(TestSDK, TouchTracker_ResetOn) {
    TouchTracker tracker;
    SDL_Event event;
    SDL_zero(event);

    tracker.FingerDown(10);
    event.type = SDL_EVENT_FINGER_MOTION;
    EXPECT_FALSE(tracker.ResetOn(event));
    EXPECT_EQ(tracker.Count(), 1);

    event.type = SDL_EVENT_WILL_ENTER_BACKGROUND;
    EXPECT_TRUE(tracker.ResetOn(event));
    EXPECT_EQ(tracker.Count(), 0);
    EXPECT_FALSE(tracker.ResetOn(event)); // нечего сбрасывать

    tracker.FingerDown(10);
    event.type = SDL_EVENT_WINDOW_FOCUS_LOST;
    EXPECT_TRUE(tracker.ResetOn(event));
    EXPECT_EQ(tracker.Count(), 0);
}

#endif // __AKK0RD_SDK_TOUCHTRACKER_TEST_H__
