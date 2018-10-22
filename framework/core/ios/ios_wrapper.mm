#include "SDL.h"
#include "ios_wrapper.h"

#include "customevents.h"

#import <CoreFoundation/CFBundle.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

bool iOSWrapper::OpenURL(const char* url)
{
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

NSString *getDocumentsDirectory()
{
    NSArray *docdirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    return docdirs[0];
}

void iOSWrapper::GetInternalWriteDir(std::string& Dir)
{
    NSArray *docdirs = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documents = docdirs[0];
    
    Dir = std::string([documents UTF8String]);
}

bool iOSWrapper::DirectoryExists(const char* Dir)
{
    NSString *path = [[NSString alloc] initWithUTF8String:Dir];
    
    BOOL isDir;       
    BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir];
    
    [path release];
    
    if(exists)
        if(isDir)
            return true;
    
    return false;
}

bool iOSWrapper::FileExists(const char* FileName)
{
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

bool iOSWrapper::DirCreate (const char* Dir)
{
    if (DirectoryExists(Dir))
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

bool iOSWrapper::GetDirContent      (const char* Dir, DirContentElementArray& ArrayList)
{
    NSURL *DirURL = [NSURL URLWithString:@(Dir)];
    
    NSError *error;
    NSURL* URL;
    NSArray* dirContent = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:DirURL includingPropertiesForKeys:[NSArray arrayWithObjects:NSURLNameKey, NSURLIsDirectoryKey, nil] options:NSDirectoryEnumerationSkipsHiddenFiles error:&error];
    
    
    NSUInteger count;
    for(count = 0; count < [dirContent count]; ++count)
    {        
        std::unique_ptr<DirContentElement> dc (new DirContentElement());
        URL = [dirContent objectAtIndex:count];
        dc->isDir = false;
        {
            NSNumber *isDirectory;
            BOOL success = [URL getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil];
            if (success && [isDirectory boolValue])
                dc->isDir = true;
        }
        dc->Name  = std::string([URL.pathComponents[URL.pathComponents.count - 1] UTF8String]);        
        ArrayList.push_back(std::move(dc));
    }
    
    [dirContent release];
    
    return true;
}

bool iOSWrapper::DirRemoveRecursive (const char* Dir)
{
    if(DirectoryExists(Dir))
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

std::string iOSWrapper::GetLanguage()
{
    NSString * language = [[NSLocale preferredLanguages] firstObject];
    return std::string([language UTF8String]);
}

void iOSWrapper::MessageBoxShow (int Code, const char* Title, const char* Message, const char* Button1, const char* Button2, const char* Button3)
{
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
    
    UIViewController *controller = [UIApplication sharedApplication].keyWindow.rootViewController;
    [controller presentViewController:alert animated:YES completion:nil];
};
