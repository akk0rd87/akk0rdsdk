#include "basewrapper.h"
#include "ios_wrapper.h"
#include "../platforms.h"
#include "../../core_defines.h"
#include "customevents.h"

#import <CoreFoundation/CFBundle.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <StoreKit/StoreKit.h>

class iOSPlatformWrapper : public PlatformWrapper {

    bool vInit() override {
        return true;
    }

    /*
    bool vOpenURL(const char* url) override {
        // https://useyourloaf.com/blog/openurl-deprecated-in-ios10/
        @autoreleasepool
        {
            //NSLog(@"URL=%s", url);
            __block bool Result = false;

            UIApplication *app = [UIApplication sharedApplication];
            NSURL *nsurl = [NSURL URLWithString:@(url)];

            //if ([app canOpenURL:nsurl]) NSLog(@"Can open");
            //else NSLog(@"Cant't open");


            if([app respondsToSelector:@selector(openURL:options:completionHandler:)])
            {
                //NSLog(@"1");
                [app openURL:nsurl options:@{} completionHandler:^(BOOL success) {
                    //NSLog(@"OpenURL %d", success);
                    Result = true;
                }];
            }
            else
            {
                //NSLog(@"2");
                if ([app canOpenURL:nsurl])
                {
                    [app openURL:nsurl];
                    Result = true;
                }
            }
            return Result;
        }
    }
    */

    std::string vGetInternalWriteDir() override {
        NSArray *docdirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documents = docdirs[0];

        auto dir = std::string([documents UTF8String]);
        if(dir.back() != '/' && dir.back() != '\\') {
            dir += '/';
        }

        return dir;
    }

    bool vDirExists(const char* Dir) override {
        NSString *path = [[NSString alloc] initWithUTF8String:Dir];

        BOOL isDir;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir];

        [path release];

        if(exists)
            if(isDir)
                return true;

        return false;
    }

    bool FileExists(const char* FileName) { // пока не использется
        NSString *path = [[NSString alloc] initWithUTF8String:FileName];

        BOOL isDir;
        BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir];

        [path release];

        if(exists)
        {
            if(isDir)
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        return false;
    }

    bool vDirCreate(const char* Dir) override {
        if (vDirExists(Dir))
            return true;

        bool res = false;
        NSError *error;
        NSString *dataPath = [[NSString alloc] initWithUTF8String:Dir];
        if (![[NSFileManager defaultManager] fileExistsAtPath:dataPath])

        if([[NSFileManager defaultManager] createDirectoryAtPath:dataPath withIntermediateDirectories:YES attributes:nil error:&error] == YES)
        {
            res = true;
        }

        [dataPath release];

        return res;
    }

    bool vGetDirContent(const char* Dir, DirContentElementArray& ArrayList) override {
        NSURL *DirURL = [NSURL URLWithString:@(Dir)];

        NSError *error;
        NSURL* URL;
        NSArray* dirContent = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:DirURL includingPropertiesForKeys:[NSArray arrayWithObjects:NSURLNameKey, NSURLIsDirectoryKey, nil] options:NSDirectoryEnumerationSkipsHiddenFiles error:&error];


        NSUInteger count;
        for(count = 0; count < [dirContent count]; ++count)
        {
            URL = [dirContent objectAtIndex:count];
            ArrayList.emplace_back(std::make_unique<DirContentElement>());
            ArrayList.back()->isDir = false;
            {
                NSNumber *isDirectory;
                BOOL success = [URL getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil];
                if (success && [isDirectory boolValue])
                    ArrayList.back()->isDir = true;
            }
            ArrayList.back()->Name = std::string([URL.pathComponents[URL.pathComponents.count - 1] UTF8String]);
        }

        [dirContent release];

        return true;
    }

    bool vDirRemoveRecursive(const char* Dir)  override {
        if(vDirExists(Dir))
        {
            NSError *error;
            NSString *path = [[NSString alloc] initWithUTF8String:Dir];

            BOOL res = [[NSFileManager defaultManager] removeItemAtPath:path error:&error];

            [path release];

            if(res) return true;
            else    return false;
        }

        return true;
    }

    bool vDirRemove(const char* Dir)  override {
        return vDirRemoveRecursive(Dir);
    }

    Locale::Lang vGetDeviceLanguage() override  {
        NSString * language = [[NSLocale preferredLanguages] firstObject];
        NSDictionary *languageDic = [NSLocale componentsFromLocaleIdentifier:language];
        NSString *languageCode = [languageDic objectForKey:@"kCFLocaleLanguageCodeKey"];
        //MessageBoxShow(10000, "Language", std::string([languageCode UTF8String]).c_str(), "Ok", nullptr, nullptr);
        return Locale::DecodeLang_ISO639_Code(std::string([languageCode UTF8String]).c_str());
    }

    void vShareText(const char* Title, const char* Message) override {
    //NSString *textToShare = @"Look at this awesome website for aspiring iOS Developers!";
    //    NSString *sTitle = [[NSString alloc] initWithUTF8String:Title];
        NSString *sMessage = [[NSString alloc] initWithUTF8String:Message];

        NSArray *objectsToShare = @[sMessage];

        UIActivityViewController *activityVC = [[UIActivityViewController alloc] initWithActivityItems:objectsToShare applicationActivities:nil];


        NSArray *excludeActivities = @[UIActivityTypeAirDrop,
                                    UIActivityTypePrint,
                                    UIActivityTypeAssignToContact,
                                    UIActivityTypeSaveToCameraRoll,
                                    UIActivityTypeAddToReadingList,
                                    UIActivityTypePostToFlickr,
                                    UIActivityTypePostToVimeo];
        activityVC.excludedActivityTypes = excludeActivities;

        auto appDelegate = [[UIApplication sharedApplication] delegate];
        activityVC.popoverPresentationController.sourceView = appDelegate.window.rootViewController.view;
        [appDelegate.window.rootViewController presentViewController:activityVC animated:YES completion:nil];

    //    [sTitle release];
        [sMessage release];
    };

    void vMessageBoxShow (int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3, Uint32 TimeOutMS) override {
        NSString *sTitle = [[NSString alloc] initWithUTF8String:Title];
        NSString *sMessage = [[NSString alloc] initWithUTF8String:Message];


        UIAlertController* alert = [UIAlertController alertControllerWithTitle:sTitle
                                                                    message:sMessage
                                                                preferredStyle:UIAlertControllerStyleAlert];

        [sTitle release];
        [sMessage release];

        bool button2, button3;
        button2 = button3 = false;

        { // Button 1
            NSString *sButton1 = [[NSString alloc] initWithUTF8String:Button1];
            UIAlertAction *defaultAction = [UIAlertAction actionWithTitle:sButton1 style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
                logDebug("Action 1");
                CustomEvents::MessageBoxCallback(Code, 1);
            }];
            [alert addAction:defaultAction];
            [sButton1 release];
        }

        if(Button2 != nullptr && Button2[0] != '\0')
            button2 = true;

        if(Button3 != nullptr && Button3[0] != '\0')
            button3 = true;

        if(button2)
        {
            auto button2Style = UIAlertActionStyleCancel;

            if(button3)
                button2Style = UIAlertActionStyleDefault;

            {
                // Button 2
                NSString *sButton2 = [[NSString alloc] initWithUTF8String:Button2];
                UIAlertAction *button2Action = [UIAlertAction actionWithTitle:sButton2 style:button2Style handler:^(UIAlertAction * _Nonnull action) {
                    logDebug("Action 2");
                    CustomEvents::MessageBoxCallback(Code, 2);
                }];
                [alert addAction:button2Action];
                [sButton2 release];
            }

            if(button3)
            {
                // Button 3
                NSString *sButton3 = [[NSString alloc] initWithUTF8String:Button3];
                UIAlertAction *button3Action = [UIAlertAction actionWithTitle:sButton3 style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                    logDebug("Action 3");
                    CustomEvents::MessageBoxCallback(Code, 3);
                }];
                [alert addAction:button3Action];
                [sButton3 release];
            }
        }

        //UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
        //[controller presentViewController:alert animated:YES completion:nil];

        auto appDelegate = [[UIApplication sharedApplication] delegate];
        [appDelegate.window.rootViewController dismissViewControllerAnimated:YES completion:nil];
        [appDelegate.window.rootViewController presentViewController:alert animated:YES completion:nil];
    };

    void vSharePNG(const char* Title, const char* File) override {
        NSString *path = [[NSString alloc] initWithUTF8String:File];
        NSURL* URL = [NSURL fileURLWithPath:path];
        NSData * imageData = [[NSData alloc] initWithContentsOfURL:URL];
        UIImage *imgShare = [[UIImage alloc] initWithData:imageData];
        NSArray *objectsToShare = @[imgShare];

        UIActivityViewController *activityVC = [[UIActivityViewController alloc] initWithActivityItems:objectsToShare applicationActivities:nil];

        //this array should add the activities that I don’t want
        NSArray *excludeActivities = @[UIActivityTypeAirDrop,
                                    UIActivityTypePrint,
                                    UIActivityTypeAssignToContact,
                                    UIActivityTypeSaveToCameraRoll,
                                    UIActivityTypeAddToReadingList,
                                    UIActivityTypePostToFlickr,
                                    UIActivityTypePostToVimeo];
        activityVC.excludedActivityTypes = excludeActivities;

        auto appDelegate = [[UIApplication sharedApplication] delegate];
        activityVC.popoverPresentationController.sourceView = appDelegate.window.rootViewController.view;
        [appDelegate.window.rootViewController presentViewController:activityVC animated:YES completion:nil];

        [path release];
        [imgShare release];
        [imageData release];
    };

    bool vLaunchAppReviewIfAvailable() override {
        [SKStoreReviewController requestReview];
        return true;
    }
};

static iOSPlatformWrapper iosPlatformWrapper;
PlatformWrapper& PlatformWrapper::vGetInstance() {
    return iosPlatformWrapper;
};
