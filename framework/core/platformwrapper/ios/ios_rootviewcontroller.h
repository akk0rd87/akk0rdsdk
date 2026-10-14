#pragma once
#ifndef __AKK0RD_SDK_IOS_ROOT_VIEW_CONTROLLER_H__
#define __AKK0RD_SDK_IOS_ROOT_VIEW_CONTROLLER_H__

#import <UIKit/UIKit.h>

// Контроллер, с которого показывать системные окна (реклама, GDPR, шаринг). Не зависит от SDL, поэтому
// годится и для SDL-приложений, и для SwiftUI. UIApplication.keyWindow устарел и при нескольких сценах на
// iPad может вернуть окно не той сцены: берётся ключевое окно активной сцены, а если активной нет (приложение
// уходит в фон) — первой подключённой. Окно ищется через isKeyWindow, а не UIWindowScene.keyWindow (iOS 15),
// чтобы работать с iOS 13.
static inline UIViewController *akkordRootViewController(void) {
    UIWindowScene *windowScene = nil;
    for (UIScene *scene in UIApplication.sharedApplication.connectedScenes) {
        if (![scene isKindOfClass:UIWindowScene.class]) {
            continue;
        }
        if (scene.activationState == UISceneActivationStateForegroundActive) {
            windowScene = (UIWindowScene *)scene;
            break;
        }
        if (windowScene == nil) {
            windowScene = (UIWindowScene *)scene;
        }
    }
    UIWindow *window = nil;
    for (UIWindow *candidate in windowScene.windows) {
        if (candidate.isKeyWindow) {
            window = candidate;
            break;
        }
    }
    return (window ?: windowScene.windows.firstObject).rootViewController;
}

#endif // __AKK0RD_SDK_IOS_ROOT_VIEW_CONTROLLER_H__
