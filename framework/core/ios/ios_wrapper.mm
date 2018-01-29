#include "SDL.h"
#include "ios_wrapper.h"

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
    for(count = 0; count < [dirContent count]; count++)
    {
        ArrayList.push_back(new DirContentElement());
        auto back = ArrayList.back();     
        
        URL = [dirContent objectAtIndex:count];       
        back->isDir = URL.hasDirectoryPath;
        back->Name  = std::string([URL.pathComponents[URL.pathComponents.count - 1] UTF8String]);
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
